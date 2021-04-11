#ifndef _RECORDER_H_
#define _RECORDER_H_

/**
 * @brief Initialize the recorder module.
 * @return 0 on success, 1 on failure.
 */
int recorder_init();

/**
 * @brief Deinitializes the recorder.
 * @return 0 on success, 1 on failure.
 */
int recorder_deinit();

/**
 * @brief Begin recording frames.
 */
void recorder_start();

#endif /* _RECORDER_H_ */
