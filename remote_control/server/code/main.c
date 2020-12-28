#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <inttypes.h>
#include <string.h>

#include "ws.h"
#include "tco_shmem.h"

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
    printf("Connection opened, client: %d | addr: %s\n", fd, cli);
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
    printf("Connection closed, client: %d | addr: %s\n", fd, cli);
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
    printf("Received a message: '%.*s' (size: %zu, type: %d), from: %s/%d\n", size - 1, msg, size,
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
    printf("Parsed message: Ch=%" PRIu8 "\tFRAC=%f\n", channel, pulse_frac);

    if (sem_wait(control_data_sem) == -1)
    {
        perror("sem_wait");
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
        perror("sem_post");
        return;
    }
    printf("\n");
}

/**
 * @note After invoking @ref ws_socket, this routine never returns, unless if invoked from a
 * different thread.
 */
int main(void)
{
    /* Map shared memory object for control data into memory and open its associated semaphore. */
    int shmem_fd = shm_open(TCO_SHMEM_NAME_CONTROL, O_RDWR, 0666);
    if (shmem_fd == -1)
    {
        perror("shm_open");
        printf("Failed to open the shared memory object.\n");
        return EXIT_FAILURE;
    }
    control_data = (struct tco_shmem_data_control *)mmap(0, TCO_SHMEM_SIZE_CONTROL, PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd, 0);
    if (control_data == MAP_FAILED)
    {
        perror("mmap");
        printf("Failed to map the shared memory object into memory.\n");
        return EXIT_FAILURE;
    }
    if (close(shmem_fd) == -1) /* No longer needed. */
    {
        perror("close"); /* Not a critical error. */
    }
    control_data_sem = sem_open(TCO_SHMEM_NAME_SEM_CONTROL, 0);
    if (control_data_sem == SEM_FAILED)
    {
        perror("sem_open");
        printf("Failed to open the semaphore associated with the shared memory object '%s'.\n", TCO_SHMEM_NAME_CONTROL);
        return EXIT_FAILURE;
    }
    /* === */

    struct ws_events evs;
    evs.onopen = &onopen;
    evs.onclose = &onclose;
    evs.onmessage = &onmessage;
    ws_socket(&evs, 8080); /* Never returns. */

    return (0);
}
