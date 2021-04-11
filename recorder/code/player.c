#include <stdio.h>
#include <stdlib.h>

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include "player.h"
#include "utils.h"

#include "tco_libd.h"

static uint8_t *video_buf;
static uint32_t video_buf_size;
static uint32_t video_buf_frame_count;

static void handle_stop(int sig)
{
    log_debug("Cleaning the player");

    if (shmem_cleanup() != 0)
    {
        log_error("Failed to cleanup shmem");
        exit(EXIT_FAILURE);
    }

    free(video_buf);
    log_info("Succesfully cleaned. Exiting.");
    exit(EXIT_SUCCESS);
}

static int file_video_metadata(FILE *const file_video)
{
    rewind(file_video);
    char file_magic_num[8];
    fread(file_magic_num, sizeof(char), 8, file_video);
    if (strncmp(file_magic_num, MAGIC_NUMBER, 8) != 0)
    {
        log_error("Unrecognized format.");
        return EXIT_FAILURE;
    }
    fread(&video_buf_size, sizeof(uint32_t), 1, file_video);
    fread(&video_buf_frame_count, sizeof(uint32_t), 1, file_video);
    printf("vid_buf_size: %u, frame_count: %u\n", video_buf_size, video_buf_frame_count);
    return EXIT_SUCCESS;
}

/**
 * @brief Open, verify and decode a compressed video dump. The format of these files is 'MAGIC_NUMBER<long><long><long><short><char><short><char>...'
 * @param file_video a FILE ptr to the RLE compressed data
 * @return 0 on success, 1 on failure
 * */
static int file_video_decode(FILE *const file_video)
{
    fseek(file_video, 8 + 4 + 4, SEEK_SET);

    uint8_t *frame_ptr = video_buf;
    for (int frame_idx = 0; frame_idx < video_buf_frame_count; frame_idx++)
    {
        fread(frame_ptr, sizeof(uint8_t), sizeof(frame_t), file_video);
        /* Align next frame pointer */
        frame_ptr = (uint8_t *)ptr_next_aligned((uintptr_t)frame_ptr, sizeof(frame_t));
    }
    return EXIT_SUCCESS;
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
 * @brief Will write the images from image to shmem.
 */
static void playback_start()
{
    uint32_t frame_id = 0;
    uint8_t *vid_buf_frame_ptr = video_buf;

    for (int frame_idx = 0; frame_idx < video_buf_frame_count; frame_idx++)
    {
        if (sem_wait(shmem_state_sem) == -1)
        {
            log_error("sem_wait: %s", strerror(errno));
            return;
        }
        /* START: Critical section */
        memcpy(&shmem_state_data->frame, vid_buf_frame_ptr, sizeof(frame_t));
        shmem_state_data->frame_id = frame_id++;
        /* END: Critical section */
        if (sem_post(shmem_state_sem) == -1)
        {
            log_error("sem_post: %s", strerror(errno));
            return;
        }
        usleep(45000); /* 0.045 seconds ~ 22 fps */
        vid_buf_frame_ptr = (uint8_t *)ptr_next_aligned((uintptr_t)vid_buf_frame_ptr, sizeof(frame_t));
    }
}

int player_init(char *file_name)
{
    register_stop_handler();
    if (shmem_init(O_RDWR) != 0)
    {
        log_error("Failed to init shmem");
        return EXIT_FAILURE;
    }
    /* Create a buffer for the video. */
    FILE *file_video = fopen(file_name, "rb");
    if (file_video == NULL)
    {
        log_error("Could not open file with name '%s'", file_name);
        return EXIT_FAILURE;
    }
    file_video_metadata(file_video);
    video_buf = aligned_alloc(32, video_buf_size);
    memset(video_buf, 150, video_buf_size);
    if (video_buf == NULL)
    {
        log_error("Failed to allocate video buffer");
        return EXIT_FAILURE;
    }

    /* Decode file */
    if (file_video_decode(file_video) != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    fclose(file_video);
    log_debug("Player initialized");
    return EXIT_SUCCESS;
}

void player_start()
{
    playback_start();
    free(video_buf);
}
