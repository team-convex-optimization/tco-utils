#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tco_libd.h"
#include "utils.h"
#include "host.h"
#include "client.h"

const int log_level = LOG_INFO | LOG_ERROR | LOG_DEBUG;
int host = 0;

/**
 * @brief Instruct how to use the program.
 */
void show_usage()
{
	printf("Allows actuation shmem to be shared over a network.\n"
		   "Usage: tco_actuation_forwarder.bin < -c | -h >\n"
		   "'-h <port>': host the actuation data (forwarder) \n"
		   "'-c <ip> <port>': client of actuation data (receiver) \n");
}

static void handle_stop(int sig)
{
    log_debug("Cleaning the process");

    if (shmem_cleanup() != 0)
    {
        log_error("Failed to cleanup shmem");
        exit(EXIT_FAILURE);
    }

	if (host) {
		host_stop(); /* Does nothing */
	} else {
		client_stop();
	}

    log_info("Succesfully cleaned. Exiting.");
    exit(EXIT_SUCCESS);
}

/**
 * @brief Register the handler that should be called when the recorder is stopped.
 */
static void register_stop_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_stop;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char *argv[])
{
	register_stop_handler();
	if (log_init("actuation_forwarder", "./log.txt") != 0)
	{
		printf("Failed to initialize the logger\n");
		return EXIT_FAILURE;
	}

	if (argc == 3 && strcmp("-h", argv[1]) == 0)
	{
		log_info("Started Host");
		host = 1;
		const int port = atoi(argv[2]);
		host_start(port); /* never returns */
	}
	else if (argc == 4 && strcmp("-c", argv[1]) == 0)
	{
		client_start(argv[2], atoi(argv[3])); /* never returns */
	}
	else
	{
		show_usage();
	}

	return EXIT_SUCCESS;
}
