#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "tco_libd.h"

#include "recorder.h"
#include "player.h"

#define FILENAME "DUMP.txt"

const int log_level = LOG_INFO | LOG_ERROR | LOG_DEBUG;

/**
 * @brief Instruct how to use the program.
 */
void show_usage()
{
	printf("Allows recording of the frames saved in state shmem and ability to play them back into the shared memory.\n"
		   "Usage: tco_recorder.bin <-r | -p file>\n"
		   "'-r': record\n"
		   "'-p': replay the frames contained in the provided file\n");
}

int main(int argc, char *argv[])
{
	if (log_init("recorder", "./log.txt") != 0)
	{
		printf("Failed to initialize the logger\n");
		return EXIT_FAILURE;
	}

	if (argc == 2 && strcmp("-r", argv[1]) == 0)
	{
		if (recorder_init() == 0)
		{
			recorder_start();
		}
	}
	else if (argc == 3 && strcmp("-p", argv[1]) == 0)
	{
		if (player_init(argv[2]) == 0)
		{
			player_start();
		}
	}
	else
	{
		show_usage();
	}

	return EXIT_SUCCESS;
}
