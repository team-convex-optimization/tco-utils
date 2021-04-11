#ifndef _UTILS_H_
#define _UTILS_H_

#include <semaphore.h>

#include "tco_shmem.h"

#define MAGIC_NUMBER "TCOVIDEO" /* Magic number to identify video files */

extern struct tco_shmem_data_state *shmem_state_data;
extern sem_t *shmem_state_sem;
extern uint8_t shmem_state_open;

typedef uint8_t frame_t[TCO_FRAME_HEIGHT][TCO_FRAME_WIDTH];

/**
 * @brief Computes a pointer aligned to 4 bytes.
 * @param ptr
 * @param step This is a step that should be added to the pointer before alignment.
 * @return A pointed aligned to a 4 byte boundary and stepped atleast @p step bytes ahead.
 */
uintptr_t ptr_next_aligned(uintptr_t ptr, uint32_t step);

/**
 * @brief Performs any necesary cleanup related to shared memorie. This needs to be called before
 * terminating after shmem has been initialized.
 * @return 0 on success and 1 on failure.
 */
int shmem_cleanup();

/**
 * @brief Initialize the shared memory required in this whole program.
 * @param access_mode Can be O_RDONLY, O_WRONLY, or O_RDWR (these are fron fcntl.h)
 * @return 0 on success and 1 on failure.
 */
int shmem_init(int access_mode);

#endif /* _UTILS_H_ */