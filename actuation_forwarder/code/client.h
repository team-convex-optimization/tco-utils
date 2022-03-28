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

void client_start(const char *ip, const int port);

#endif /*_CLIENT_H_ */