#include "client.h"

struct tws_ctx ctx;

void run() {
    size_t buff_size = TCO_SHMEM_SIZE_CONTROL;
    int frm_type = 0;
    while (1) {
        if (tws_receiveframe(&ctx, (char **) &shmem_control_data, &buff_size, &frm_type) < 0) {
            log_error("Failed to receive message!");
            exit(-1);
        }
        get_or_set_data(shmem_control_data, 0);
    }
}

void client_start(const char *ip, const int port) {
    if (shmem_init(O_WRONLY)) {
        log_error("Failed to init client shmem");
        exit(-1);
    }
    tws_connect(&ctx, ip, port);
    char *hellotxt = "TCO_START";
    if (tws_sendframe(&ctx, (uint8_t *)hellotxt, strlen(hellotxt), 1)) {
        log_error("Failed to send initial message");
        exit(-1);
    }
    run();
}

