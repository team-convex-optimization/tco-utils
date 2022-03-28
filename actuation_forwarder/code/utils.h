#ifndef _UTILS_H_
#define _UTILS_H_

#include <semaphore.h>
#include "tco_shmem.h"

extern struct tco_shmem_data_control *shmem_control_data;
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
 * @brief Read or Write the data from TCO_CONTROL_SHMEM
 * @param data is a ptr to write the data to
 * @param dir is the direction to R/W. 0 is write @p data to shmem. else is read shmem into @p data
 * @return void
 */
void get_or_set_data(struct tco_shmem_data_control *data, uint8_t dir);

#endif /* _UTILS_H_ */