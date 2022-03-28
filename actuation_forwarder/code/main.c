#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tco_libd.h"
#include "utils.h"
#include "host.h"
#include "client.h"

const int log_level = LOG_INFO | LOG_ERROR | LOG_DEBUG;

/**
 * @brief Instruct how to use the program.
 */
void show_usage()
{
	printf("Allows actuation shmem to be shared over a network.\n"
		   "Usage: tco_actuation_forwarder.bin < -c | -h > <port>\n"
		   "'-h': host the actuation data (forwarder) \n"
		   "'-c': client of acutation data (receiver) \n");
}

static void handle_stop(int sig)
{
    log_debug("Cleaning the process");

    if (shmem_cleanup() != 0)
    {
        log_error("Failed to cleanup shmem");
        exit(EXIT_FAILURE);
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
	if (log_init("actuation_forwarder", "./log.txt") != 0)
	{
		printf("Failed to initialize the logger\n");
		return EXIT_FAILURE;
	}

	if (argc == 3 && strcmp("-h", argv[1]) == 0)
	{
		if (host_init(atoi(argv[2])) == 0)
		{
			host_start();
		}
	}
	else if (argc == 3 && strcmp("-c", argv[1]) == 0)
	{
		if (client_init(atoi(argv[2])) == 0)
		{
			client_start();
		}
	}
	else
	{
		show_usage();
	}

	return EXIT_SUCCESS;
}
