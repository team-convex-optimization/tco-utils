#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "tco_libd.h"

#include "utils.h"

/* Globals */
struct tco_actuation_data_state *shmem_actuation_data;
sem_t *shmem_actuation_sem;
uint8_t shmem_actuation_open = 0;

int shmem_cleanup()
{
    /* To avoid deadlocks, ensure the shmem is closed on exit. */
    if (shmem_actuation_open)
    {
        if (sem_post(shmem_actuation_sem) == -1)
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
    if (shmem_map(TCO_SHMEM_NAME_STATE, TCO_SHMEM_SIZE_STATE, TCO_SHMEM_NAME_SEM_STATE, access_mode, (void **)&shmem_actuation_data, &shmem_actuation_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
