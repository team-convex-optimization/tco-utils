#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ws.h>

/**
 * @brief Called when a client connects to the server.
 * @param fd File Descriptor belonging to the client. The @p fd parameter is used in order to send
 * messages and retrieve informations about the client.
 */
void onopen(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    printf("Connection opened, client: %d | addr: %s\n", fd, cli);
    free(cli);
}

/**
 * @brief Called when a client disconnects to the server.
 * @param fd File Descriptor belonging to the client. The @p fd parameter is used in order to send
 * messages and retrieve informations about the client.
 */
void onclose(int fd)
{
    char *cli;
    cli = ws_getaddress(fd);
    printf("Connection closed, client: %d | addr: %s\n", fd, cli);
    free(cli);
}

/**
 * @brief Called when a client connects to the server.
 * @param fd File Descriptor belonging to the client. The
 * @p fd parameter is used in order to send messages and retrieve informations about the client.
 * @param msg Received message, this message can be a text or binary message.
 * @param size Message size (in bytes).
 * @param type Message type.
 */
void onmessage(int fd, const unsigned char *msg, size_t size, int type)
{
    char *cli;
    cli = ws_getaddress(fd);
    printf("Received a message: %s (size: %zu, type: %d), from: %s/%d\n", msg, size,
           type, cli, fd);
    free(cli);
    /* Echo received frame. */
    ws_sendframe(fd, (char *)msg, size, true, type);
}

/**
 * @note After invoking @ref ws_socket, this routine never returns, unless if invoked from a
 * different thread.
 */
int main(void)
{
    struct ws_events evs;
    evs.onopen = &onopen;
    evs.onclose = &onclose;
    evs.onmessage = &onmessage;
    ws_socket(&evs, 8080); /* Never returns. */

    return (0);
}
