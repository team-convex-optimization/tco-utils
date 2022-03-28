#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "tco_shmem.h"
#include "tco_libd.h"
#include "utils.h"
#include "toyws.h"

/**
 * @brief Start the client communication protocol with @p ip ip and @p port
 * 
 * @param ip IP of host
 * @param port port of host
 * @return This task never returns.
 */
void client_start(const char *ip, const int port);

/**
 * @brief Cleanup the client state
 * 
 */
void client_stop();


#endif /*_CLIENT_H_ */