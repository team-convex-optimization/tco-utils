#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "tco_libd.h"
#include "utils.h"

/* Globals */
struct tco_shmem_data_control *shmem_control_data;
sem_t *shmem_control_sem;
uint8_t shmem_control_open = 0;

int shmem_cleanup()
{
    /* To avoid deadlocks, ensure the shmem is closed on exit. */
    if (shmem_control_open)
    {
        if (sem_post(shmem_control_sem) == -1)
        {
            log_error("sem_post: %s", strerror(errno));
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int shmem_init(int access_mode)
{
    if (access_mode != O_RDONLY && access_mode != O_WRONLY && access_mode != O_RDWR)
    {
        return EXIT_FAILURE;
    }
    if (shmem_map(TCO_SHMEM_NAME_STATE, TCO_SHMEM_SIZE_STATE, TCO_SHMEM_NAME_SEM_STATE, access_mode, (void **)&shmem_control_data, &shmem_control_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Read or Write the data from TCO_CONTROL_SHMEM
 * @param data is a ptr to write the data to
 * @param dir is the direction to R/W. 0 is write @p data to shmem. else is read shmem into @p data
 * @return void
 */
void get_or_set_data(struct tco_shmem_data_control *data, uint8_t dir) {
    if (sem_wait(shmem_control_sem) == -1)
    {
        log_error("sem_wait: %s", strerror(errno));
        return;
    }
    /* START: Critical section */
    switch (dir)
    {
        case 0:
            memcpy(shmem_control_data, data, TCO_SHMEM_SIZE_CONTROL);
            break;
        default:
            memcpy(data, shmem_control_data, TCO_SHMEM_SIZE_CONTROL);
            break;
    }
    /* END: Critical section */
    if (sem_post(shmem_control_sem) == -1)
    {
        log_error("sem_post: %s", strerror(errno));
        return;
    }
}