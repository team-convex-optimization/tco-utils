#ifndef _UTILS_H_
#define _UTILS_H_

#include <semaphore.h>
#include "tco_shmem.h"

extern struct tco_shmem_sem_control *shmem_actuation_data;
extern sem_t *shmem_control_sem;
extern uint8_t shmem_control_open;

/**
 * @brief Performs any necessary cleanup related to shared memory. This needs to be called before
 * terminating after shmem has been initialized.
 * @return 0 on success and 1 on failure.
 */
int shmem_cleanup();

/**
 * @brief Initialize the shared memory required in this whole program.
 * @param access_mode Can be O_RDONLY, O_WRONLY, or O_RDWR (these are from fcntl.h)
 * @return 0 on success and 1 on failure.
 */
int shmem_init(int access_mode);

/**
 * @brief Read the data from TCO_CONTROL_SHMEM
 * @return A copy of the current struct tco_shmem_sem_control 
 */
struct tco_shmem_sem_control read_data(void);

#endif /* _UTILS_H_ */