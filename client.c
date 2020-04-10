/*
 * Client-side of ping-pong project
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "pingpong.h"


#define PROGRAM_NAME "pp-client"
#define OPTIONS "hv"


static char *filename;

static void print_version (void);
static void print_usage (void);
static void parse_options (int argc, char **argv);
int main (int argc, char **argv);


static void
print_version (void)
{
    printf("%s: version %d.%d \n", PROGRAM_NAME, VERSION_MAJOR, VERSION_MINOR);
    return;
}

static void
print_usage (void)
{
    printf("%s: usage \n\t./%s [-%s] filename\n",
           PROGRAM_NAME, OPTIONS, PROGRAM_NAME
           );
    return;
}

static void
parse_options (int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, OPTIONS)) != -1) {
        switch (c) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            default:
                goto exit_failure;
        }
    }

    if (optind == argc-1) {
        filename = argv[optind];
        printf("filename: %s\n", filename);
    } else if (optind == argc) {
        printf("%s: missing file operand\n", PROGRAM_NAME);
        goto exit_failure;
    } else {
        printf("%s: two many arguments\n", PROGRAM_NAME);
        goto exit_failure;
    }

    return;

exit_failure:
    printf("Try './%s -h' for more information.\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
    int fd;

    parse_options(argc, argv);

    if ((fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
        printf("%s: Failed to open %s\n", PROGRAM_NAME, filename);
        exit(errno);
    }

    close(fd);

    exit(EXIT_SUCCESS);
}
