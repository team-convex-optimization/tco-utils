#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "tco_libd.h"

#include "utils.h"

/* Globals */
struct tco_shmem_data_state *shmem_state_data;
sem_t *shmem_state_sem;
uint8_t shmem_state_open = 0;

uintptr_t ptr_next_aligned(uintptr_t ptr, uint32_t step)
{
    uintptr_t const ptr_next = ptr + step;
    return (ptr_next + 4 - 1) & ~(4 - 1);
}

int shmem_cleanup()
{
    /* To avoid deadlocks, ensure the shmem is closed on exit. */
    if (shmem_state_open)
    {
        if (sem_post(shmem_state_sem) == -1)
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
    if (shmem_map(TCO_SHMEM_NAME_STATE, TCO_SHMEM_SIZE_STATE, TCO_SHMEM_NAME_SEM_STATE, access_mode, (void **)&shmem_state_data, &shmem_state_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
