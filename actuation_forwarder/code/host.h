#ifndef _HOST_H_
#define _HOST_H_

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "tco_libd.h"
#include "tco_shmem.h"
#include "ws.h"

enum ws_state {
    INIT = 0,
    RUNNING = 1,
    ERR = 2
};

void host_start(const int port);

#endif /*_HOST_H_ */