#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <curses.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "tco_libd.h"
#include "tco_shmem.h"

int log_level = LOG_INFO | LOG_DEBUG | LOG_ERROR;

static int term_width, term_height;

void print_usage(void)
{
    printf("Usage: tco_shmem_viewer <[-i shmem_id | --index shmem_id]>\n"
           "Note that the 'shmem_id' is the index inside 'TCO_SHMEM_ARR_NAME'\n");
}

int shmem_parse(uint8_t *data, uint16_t size, char *target, uint16_t target_size, uint16_t col_size)
{
    uint32_t output_bytes = 0;
    for (uint32_t src_i = 0; src_i < size; src_i++)
    {
        if (src_i > 0 && src_i % col_size == 0)
        {
            target[output_bytes++] = '\n';
        }
        output_bytes += sprintf(&target[output_bytes], "%02X ", data[src_i]);
    }
    return 0;
}

int dump_file(uint8_t *const data, uint32_t const data_len)
{
    FILE *file = fopen("dump.txt", "wb");
    fwrite(data, sizeof(uint8_t), data_len, file);
    fclose(file);
    return 0;
}

int main(int argc, char const *argv[argc])
{
    if (log_init("shmem_viewer", "./log.txt") != 0)
    {
        printf("Failed to initialize the logger\n");
        return EXIT_FAILURE;
    }

    char *shmem_names[] = TCO_SHMEM_ARR_NAME;
    char *shmem_sem_names[] = TCO_SHMEM_ARR_SEM_NAME;
    uint32_t shmem_sizes[] = TCO_SHMEM_ARR_SIZE;
    uint8_t shmem_count = sizeof(shmem_sizes) / sizeof(uint32_t);
    uint8_t dump_file_only = 0;

    uint8_t shmem_id = 0;
    if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--index") == 0)
    {
        shmem_id = strtoul(argv[2], 0L, 10);
        if (shmem_id >= shmem_count)
        {
            printf("The requested shmem ID does not exist, there are %u shared memory regions\n", shmem_count);
        }

        if (argc == 4 && strcmp(argv[3], "--dump") == 0)
        {
            dump_file_only = 1;
        }
    }
    else
    {
        print_usage();
        return EXIT_FAILURE;
    }
    log_info("Selected shmem with ID %u and name '%s'", shmem_id, shmem_names[shmem_id]);

    uint8_t *shmem_data;
    sem_t *shmem_sem;
    if (shmem_map(shmem_names[shmem_id], shmem_sizes[shmem_id], shmem_sem_names[shmem_id], O_RDONLY, (void **)&shmem_data, &shmem_sem) != 0)
    {
        log_error("Failed to map shared memory and associated semaphore");
        return EXIT_FAILURE;
    }

    /* Init ncurses */
    initscr();
    use_default_colors(); /* Use the default colors of the terminal e.g. background, text,... */
    noecho();             /* Don't echo keyboard input */
    getmaxyx(stdscr, term_height, term_width);
    refresh();
    start_color();      /* Enable color support */
    nodelay(stdscr, 1); /* Makes 'getch' a non-blocking call */

    /* Write name of shmem that will be displayed */
    addstr(shmem_names[shmem_id]);
    addch('\n');

    uint32_t const shmem_data_len = shmem_sizes[shmem_id];
    uint8_t shmem_data_cpy[shmem_data_len];

    /* Explanation for the string length: 2 hex digits per byte, n-1 spaces between the hex digits, at most n new lines. */
    uint16_t const shmem_data_string_len = (shmem_sizes[shmem_id] * 2) + shmem_sizes[shmem_id] - 1 + shmem_sizes[shmem_id];
    char shmem_data_string[shmem_data_string_len];

    int key = 0;
    do
    {
        int old_term_width = term_width, old_term_height = term_height;
        getmaxyx(stdscr, term_height, term_width);
        if (old_term_width != term_width || old_term_height != term_height)
        {
            /* Clear old memory text after terminal resize */
            clrtobot();
        }

        if (sem_wait(shmem_sem) == -1)
        {
            log_error("sem_wait: %s", strerror(errno));
            return EXIT_FAILURE;
        }
        /* START: Critical section */
        memcpy(&shmem_data_cpy, shmem_data, shmem_data_len);
        /* END: Critical section */
        if (sem_post(shmem_sem) == -1)
        {
            log_error("sem_post: %s", strerror(errno));
            return EXIT_FAILURE;
        }

        if (dump_file_only)
        {
            dump_file(shmem_data_cpy, shmem_sizes[shmem_id]);
            break;
        }
        if (shmem_parse(shmem_data_cpy, shmem_data_len, shmem_data_string, shmem_data_string_len, 32) != 0)
        {
            log_error("Failed to parse contents of shmem");
            return EXIT_FAILURE;
        }

        addstr(shmem_data_string);
        move(1, 0);
        refresh();

        usleep(100000);
    } while ((key = getch()) != 'q');

    /* Deinit ncurses resources and exit */
    endwin();
    return EXIT_SUCCESS;
}
