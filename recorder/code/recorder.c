#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "tco_libd.h"
#include "tco_shmem.h"

#include "recorder.h"
#include "utils.h"

static uint8_t *vid_buf;                               /* Pointer to the array of recorded frames. */
static uint8_t *vid_buf_frame_last;                    /* Pointer to start of last frame. */
static uint32_t vid_buf_size = 9000 * sizeof(frame_t); /* Current size of buffer (in frames). */
static uint32_t frame_recorded_count = 0;              /* Number of recorded vid_buf. */
static uint32_t frame_id_last = 0;
static char const *filename_dest = "video.txt";

/**
 * @brief Handle situation where the recorder should terminate. This performs video compression and
 * dumps data to a file.
 * @param sig Signal number that has been received. This is ignored since this relies on the fact
 * that the only signals that get handled are those that must lead to stopping of the recording.
 */
static void handle_stop(int sig)
{
    log_debug("Saving video to '%s'", filename_dest);

    if (shmem_cleanup() != 0)
    {
        log_error("Failed to cleanup shmem");
        recorder_deinit();
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(filename_dest, "wb");
    if (file == NULL)
    {
        log_error("Could not open DUMP file");
        recorder_deinit();
        exit(EXIT_FAILURE);
    }

    /* Write magic number to file (8 chars) */
    fprintf(file, MAGIC_NUMBER);
    fprintf(file, "00000000"); /* Reserve space to store file size, and number of saved frames, and compressed size. */

    uint8_t *buf_ptr = vid_buf;
    uint32_t buf_bytes_parsed = 0;
    uint32_t frames_parsed = 0;

    /* Apply "run length encoding" */
    while (buf_bytes_parsed < vid_buf_size && frames_parsed < frame_recorded_count)
    {
        fwrite(buf_ptr, sizeof(uint8_t), sizeof(frame_t), file);
        frames_parsed++;
        uintptr_t const buf_ptr_old = (uintptr_t)buf_ptr;
        buf_ptr = (uint8_t *)ptr_next_aligned((uintptr_t)buf_ptr, sizeof(frame_t));
        uintptr_t const buf_ptr_new = (uintptr_t)buf_ptr;
        buf_bytes_parsed += buf_ptr_new - buf_ptr_old;
    }

    /* Save size of video. */
    fseek(file, 8, SEEK_SET);
    fwrite(&buf_bytes_parsed, sizeof(uint32_t), 1, file);
    fwrite(&frames_parsed, sizeof(uint32_t), 1, file);

    log_info("Recording complete and video (%lu frames)", frame_recorded_count);
    fclose(file);

    recorder_deinit();
    exit(EXIT_SUCCESS);
}

/**
 * @brief Register the handler that should be called when the recorder is stopped.
 */
static void register_stop_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_stop;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

/**
 * @brief Copy the next frame (higher id) from status shmem into @p image .
 * @param frame_dest Where to copy frame to.
 * @return 0 on success, 1 on failure
 */
static int read_frame(frame_t *const frame_dest)
{
    uint8_t frame_next_found = 0;
    while (!frame_next_found)
    {
        if (sem_wait(shmem_state_sem) == -1)
        {
            log_error("sem_wait: %s", strerror(errno));
            return EXIT_FAILURE;
        }
        /* START: Critical section */
        shmem_state_open = 1;
        if (frame_id_last != shmem_state_data->frame_id)
        {
            memcpy(frame_dest, shmem_state_data->frame, sizeof(frame_t));
            frame_id_last = shmem_state_data->frame_id;
            frame_next_found = 1;
        }
        /* END: Critical section */
        if (sem_post(shmem_state_sem) == -1)
        {
            log_error("sem_post: %s", strerror(errno));
            return EXIT_FAILURE;
        }
        shmem_state_open = 0;

        usleep(20000); /* 20ms backoff to prevent blocking the shmem */
    }
    return EXIT_SUCCESS;
}

int recorder_deinit()
{
    free(vid_buf);
    log_debug("Recorder deinitialized");
    return EXIT_SUCCESS;
}

int recorder_init()
{
    if (shmem_init(O_RDONLY) != 0)
    {
        log_error("Failed to init shmem");
        return EXIT_FAILURE;
    }

    vid_buf = aligned_alloc(32, vid_buf_size);
    if (vid_buf == NULL)
    {
        log_error("Failed to allocate the video buffer in recorder");
        return EXIT_FAILURE;
    }

    log_debug("Recorder initialized");
    return EXIT_SUCCESS;
}

void recorder_start()
{
    register_stop_handler();
    log_info("Recorder started");
    vid_buf_frame_last = vid_buf;
    while (1)
    {
        uint8_t *const vid_buf_frame_next = (uint8_t *)ptr_next_aligned((uintptr_t)vid_buf_frame_last, sizeof(frame_t));
        if ((uintptr_t)(vid_buf_frame_next - vid_buf) + sizeof(frame_t) >= vid_buf_size)
        {
            /* No more space in video buffer so need to dump and quit. */
            handle_stop(0);
        }
        read_frame((frame_t *)vid_buf_frame_next);
        frame_recorded_count++;
        vid_buf_frame_last = vid_buf_frame_next;
    }
}
