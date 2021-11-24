#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "ws.h"
#include "tco_shmem.h"
#include "tco_libd.h"

typedef enum { /* Simple enumeration to keep track of video state */
    OFF = 0,
    INIT = 1,
    RUNNING = 2
} video_status_t;
static video_status_t video = OFF; 

int log_level = LOG_INFO | LOG_ERROR; /* | LOG_DEBUG */

static struct tco_shmem_data_state *control_state;
static struct tco_shmem_data_control *control_data;
static sem_t *control_data_sem;
static sem_t *control_state_sem;
static pthread_t vThread; /* Thread for sending video frames */


void usage()
{
  printf("Usage: ./tco_remote_control.bin <[--video | -v | [--help | -h]>\n"
         "'-v': Send shmem image frames over the websocket to the connected client. \n");
}

/**
 * @brief reads image from shmem
 * @param msg is a buffer of (already allocated) TCO_FRAME_HEIGHT * TCO_FRAME_WIDTH bytes
 * @return is void. Result is in msg. 
 */
void prepare_img_frame(uint8_t (*msg)[TCO_FRAME_HEIGHT * TCO_FRAME_WIDTH]) {
    if (sem_wait(control_state_sem) == -1)
    {
        log_error("sem_wait: %s", strerror(errno));
        return;
    }
    /* START: Critical section */
    memcpy(msg, control_state->frame, TCO_FRAME_HEIGHT*TCO_FRAME_WIDTH);
    /* END: Critical section */
    if (sem_post(control_state_sem) == -1)
    {
        log_error("sem_post: %s", strerror(errno));
        return;
    }
}

/**
 * @brief a function meant to be run by a separate thread to send images to a socket
 * @p args is a void * with the fd to send frames to
 * @return never returns
 */
void *send_frames(void *args) {
    int fd = ((int *)args)[0];
    while (1) {
        _Alignas(4) uint8_t reply[TCO_FRAME_HEIGHT * TCO_FRAME_WIDTH]; /* Buffer for Image Reply */
        prepare_img_frame(&reply);
        if (ws_sendframe_bin(fd, (char *)reply, TCO_FRAME_HEIGHT*TCO_FRAME_WIDTH, 1) <= 0) {
            log_error("failed to send frame to socket");
            exit(-1);
        }
        usleep(50000); /* Sleep for 1/20 seconds for 20 frames a second */
    }
}

/**
 * @brief Called when a client connects to the server.
 * @param fd File Descriptor belonging to the client. The @p fd parameter is used in order to send
 * messages and retrieve informations about the client.
 */
void onopen(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    log_info("Connection opened, client: %d | addr: %s", fd, cli);
    free(cli);
}

/**
 * @brief Called when a client disconnects to the server.
 * @param fd File Descriptor belonging to the client. The @p fd parameter is used in order to send
 * messages and retrieve informations about the client.
 */
void onclose(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    log_info("Connection closed, client: %d | addr: %s", fd, cli);
    free(cli);
}

/**
 * @brief Called when a client connects to the server.
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and retrieve informations about the client.
 * @param msg Received message, this message can be a text or binary message.
 * @param size Message size (in bytes).
 * @param type Message type.
 */
void onmessage(int fd, const unsigned char *msg, size_t size, int type)
{
    char *cli;
    cli = ws_getaddress(fd);
    log_debug("Received a message: '%.*s' (size: %zu, type: %d), from: %s/%d", size - 1, msg, size,
              type, cli, fd);
    free(cli);

    if (video == INIT) { /* Send the image frame to the client */
        video = RUNNING;
        if (pthread_create(&vThread, NULL, send_frames, (void *)&fd) != 0) { /* Never returns */
            log_error("failed to create frame_server thread : %s", strerror(errno));
            video = OFF;
        };
    }

    /* Parse message */
    uint8_t channel;
    int32_t pulse_frac_raw;
    float pulse_frac;
    sscanf((const char *)msg, "%" SCNu8 " "
                              "%" SCNi32,
           &channel, &pulse_frac_raw);
    pulse_frac = ((pulse_frac_raw / 1000000.0f) + 1.0f) / 2.0f;
    log_debug("Parsed message: Ch=%" PRIu8 "\tFRAC=%f", channel, pulse_frac);

    if (sem_wait(control_data_sem) == -1)
    {
        log_error("sem_wait: %s", strerror(errno));
        return;
    }
    /* START: Critical section */
    control_data->ch[channel].active = 1;
    control_data->ch[channel].pulse_frac = pulse_frac;
    /* END: Critical section */
    if (sem_post(control_data_sem) == -1)
    {
        log_error("sem_post: %s", strerror(errno));
        return;
    }

}

/**
 * @note After invoking @ref ws_socket, this routine never returns, unless if invoked from a
 * different thread.
 */
int main(int argc, char *argv[])
{
    if (log_init("remote_control_server", "./log.txt") != 0)
    {
        printf("Failed to initialize the logger.\n");
        return EXIT_FAILURE;
    }

    if (shmem_map(TCO_SHMEM_NAME_CONTROL, TCO_SHMEM_SIZE_CONTROL, TCO_SHMEM_NAME_SEM_CONTROL, O_RDWR, (void **)&control_data, &control_data_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }

    if (shmem_map(TCO_SHMEM_NAME_STATE, TCO_SHMEM_SIZE_STATE, TCO_SHMEM_NAME_SEM_STATE, O_RDONLY, (void **)&control_state, &control_state_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }

    if (argc == 2 && (strcmp(argv[1], "--video") == 0 || strcmp(argv[1], "-v")) == 0) {
        video = INIT;
    } else if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h")) == 0) {
        usage();
        return 0;
    }

    struct ws_events evs;
    evs.onopen = &onopen;
    evs.onclose = &onclose;
    evs.onmessage = &onmessage;
    ws_socket(&evs, 8080); /* Never returns. */

    return (0);
}
