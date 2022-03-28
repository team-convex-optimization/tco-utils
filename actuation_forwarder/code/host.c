#include "host.h"
#include "utils.h"
#include <stdio.h>

struct ws_events evs;
enum ws_state net_state = INIT;
static pthread_t aThread; /* Thread for sending actuation frames */

/*************************************************
 * START forward declaration for WS protocol *****
 *************************************************/
void net_init();
void net_onopen(int fd);
void net_onclose(int fd);
void net_onmessage(int fd, const unsigned char *msg, size_t size, int type);

/*************************************************
 *********** END forward declarations ************
 *************************************************/

void host_start(const int port) {
    if (shmem_init(O_RDONLY)) {
        log_error("Failed to init shmem");
        exit(-1);
    }
    net_init();
    log_info("Server correctly instantiated");
    ws_socket(&evs, port, 0); /* Never returns. */
}

/*****************************************
 ***** START WS protocol definitions *****
 *****************************************/

void net_init() {
    evs.onopen = &net_onopen;
    evs.onclose = &net_onclose;
    evs.onmessage = &net_onmessage;
}

void net_onopen(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    log_info("Connection opened, client: %d | addr: %s", fd, cli);
    free(cli);
}

void net_onclose(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    log_info("Connection closed, client: %d | addr: %s", fd, cli);
    free(cli);
}

/**
 * @brief a function meant to be run by a separate thread to send images to a socket
 * @p args is a void * with the fd to send frames to
 * @return never returns
 */
void *send_frames(void *args) {
    int fd = (int) args;
    _Alignas(4) struct tco_shmem_data_control reply; /* Buffer for Image Reply */
    while (1) {
        get_or_set_data(&reply, 1);
        if (ws_sendframe_bin(fd, (const char *)(&reply), TCO_SHMEM_SIZE_CONTROL, 0) == -1) {
            log_error("failed to send frame to socket");
            exit(-1);
        }
        usleep(25000); /* Sleep for 1/40 seconds for 40 frames a second */
    }
}

/**
 * @brief Called when a client connects to the server. Will send 40 actuation frames a second.
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and retrieve informations about the client.
 * @param msg Received message, this message can be a text or binary message.
 * @param size Message size (in bytes).
 * @param type Message type.
 */
void net_onmessage(int fd, const unsigned char *msg, size_t size, int type)
{
    char *cli;
    cli = ws_getaddress(fd);
    printf("Received a message: '%s' (size: %zu, type: %d), from: %s/%d\n", msg, size,
              type, cli, fd);
    log_debug("Received a message: '%.*s' (size: %zu, type: %d), from: %s/%d and fd is %d", size - 1, msg, size,
              type, cli, fd, fd);
    free(cli);

    if (net_state == INIT) { /* Send the image frame to the client */
        net_state = RUNNING;
        if (pthread_create(&aThread, NULL, send_frames, (void *)fd) != 0) { /* Never returns */
            log_error("failed to create frame_server thread : %s", strerror(errno));
            net_state = ERR;
        };
    }

}
