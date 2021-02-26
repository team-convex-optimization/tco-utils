#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>

#include "ws.h"
#include "tco_shmem.h"
#include "tco_libd.h"

int log_level = LOG_INFO | LOG_ERROR; /* | LOG_DEBUG */

static struct tco_shmem_data_control *control_data;
static sem_t *control_data_sem;

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
    if (control_data->valid == 0)
    {
        memset(control_data, 0, TCO_SHMEM_SIZE_CONTROL);
        control_data->valid = 1;
    }
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
int main(void)
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

    struct ws_events evs;
    evs.onopen = &onopen;
    evs.onclose = &onclose;
    evs.onmessage = &onmessage;
    ws_socket(&evs, 8080); /* Never returns. */

    return (0);
}
