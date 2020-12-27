#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

#include "tco_shmem.h"

typedef enum
{
    CMD_INVALID,
    CMD_UNK,
    CMD_CREATE,
    CMD_HELP,
    CMD_DELETE
} command_t;

void static usage_msg(void)
{
    printf("Usage: tco_shmem_util <[-c | --create] | [-d | --delete] | [-h | --help]>\n");
}

command_t static parse_arguments(int argc, char const *argv[])
{
    if (argc != 2)
    {
        return CMD_INVALID;
    }

    if (strcmp(argv[1], "--create") == 0 || strcmp(argv[1], "-c") == 0)
    {
        return CMD_CREATE;
    }
    else if (strcmp(argv[1], "--delete") == 0 || strcmp(argv[1], "-d") == 0)
    {
        return CMD_DELETE;
    }
    else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)
    {
        return CMD_HELP;
    }
    else
    {
        return CMD_UNK;
    }
}

void static shmem_create(void)
{
    char *const shmem_names[] = TCO_SHMEM_ARR_NAME;
    uint32_t const shmem_sizes[] = TCO_SHMEM_ARR_SIZE;
    uint16_t const shmem_count = sizeof(shmem_names) / sizeof(char *);

    for (uint16_t shmem_i = 0; shmem_i < shmem_count; shmem_i++)
    {
        printf("\n");
        printf("Creating shared memory object '%s' of size %u\n", shmem_names[shmem_i], shmem_sizes[shmem_i]);
        int fd = shm_open(shmem_names[shmem_i], O_CREAT | O_EXCL | O_RDWR, 0666);
        if (fd == -1)
        {
            perror("shm_open");
            struct stat stat_buf = {0};
            switch (errno)
            {
            case EEXIST:
                /* Need to try opening again since shm_open failed with EEXIST and we need to read object size. */
                fd = shm_open(shmem_names[shmem_i], O_RDONLY, 0666);
                if (fd != -1)
                {
                    if (fstat(fd, &stat_buf) != 0)
                    {
                        perror("fstat");
                        printf("Tried checking if the existing shared memory object is of the requested size but failed.\n");
                        break;
                    }

                    if (stat_buf.st_size == shmem_sizes[shmem_i])
                    {
                        printf("The existing shared memory object size is equal to the requested size.\n");
                    }
                    else
                    {
                        printf("The existing shared memory object size is NOT equal to the requested size. Remove it first then recreate it.\n");
                    }
                }
                /* When opening for readonly failed, just leave it at that i.e. error message for the first shm_open. */
                break;
            default:
                break;
            }
        }
        else
        {
            if (ftruncate(fd, shmem_sizes[shmem_i]) != 0)
            {
                perror("ftruncate");
            }
            else
            {
                printf("Successfully created the shared memory object\n");
            }
        }
    }
}

void static shmem_delete(void)
{
    char *const shmem_names[] = TCO_SHMEM_ARR_NAME;
    uint16_t const shmem_count = sizeof(shmem_names) / sizeof(char *);

    for (uint16_t shmem_i = 0; shmem_i < shmem_count; shmem_i++)
    {
        printf("\n");
        printf("Removing object '%s'\n", shmem_names[shmem_i]);
        if (shm_unlink(shmem_names[shmem_i]) == 0)
        {
            printf("Successfully removed the shared memory object\n");
        }
        else
        {
            perror("shm_unlink");
            printf("Failed to remove the shared memory object\n");
        }
    }
}

void static shmem_sem_create(void)
{
    char *const sem_names[] = TCO_SHMEM_ARR_SEM_NAME;
    uint16_t const sem_count = sizeof(sem_names) / sizeof(char *);

    for (uint16_t sem_i = 0; sem_i < sem_count; sem_i++)
    {
        printf("\n");
        printf("Creating semaphore '%s'\n", sem_names[sem_i]);
        sem_t *sem = sem_open(sem_names[sem_i], O_CREAT | O_EXCL, 0666, 0);
        if (sem == SEM_FAILED)
        {
            perror("sem_open");
            switch (errno)
            {
            case EEXIST:
                printf("Semaphore already exists.\n");
                break;
            default:
                printf("Failed to create the semaphore.\n");
                break;
            }
        }
        else
        {
            printf("Successfully created the semaphore\n");
        }
    }
}

void static shmem_sem_delete(void)
{
    char *const sem_names[] = TCO_SHMEM_ARR_SEM_NAME;
    uint16_t const sem_count = sizeof(sem_names) / sizeof(char *);

    for (uint16_t sem_i = 0; sem_i < sem_count; sem_i++)
    {
        printf("\n");
        printf("Removing semaphore '%s'\n", sem_names[sem_i]);
        if (sem_unlink(sem_names[sem_i]) == 0)
        {
            printf("Successfully removed the semaphore.\n");
        }
        else
        {
            perror("sem_unlink");
            printf("Failed to remove the semaphore.\n");
        }
    }
}

int main(int argc, char const *argv[])
{
    switch (parse_arguments(argc, argv))
    {
    case CMD_UNK:
        printf("Unknown command argument. Take a look at the usage message again.\n");
        exit(EXIT_FAILURE);
        break;
    case CMD_CREATE:
        printf("Creating shared memory regions and their semaphores.\n");
        shmem_create();
        shmem_sem_create();
        break;
    case CMD_DELETE:
        printf("Deleting shared memory regions and their semaphores.\n");
        shmem_delete();
        shmem_sem_delete();
        break;
    case CMD_HELP:
        usage_msg();
        exit(EXIT_SUCCESS);
        break;
    case CMD_INVALID:
        printf("Invalid command format. Take a look at the usage message again.\n");
        usage_msg();
        exit(EXIT_FAILURE);
        break;
    }
    return 0;
}
