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
#include <string.h>

#include "pingpong.h"


#define PROGRAM_NAME "pp-client"
#define OPTIONS "hv"


#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


static char *sock_path;


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
    printf("%s: usage \n./%s [-%s] sock_path\n",
           PROGRAM_NAME, PROGRAM_NAME, OPTIONS
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
        sock_path = argv[optind];
        printf("sock_path: %s\n", sock_path);
    } else if (optind == argc) {
        printf("%s: missing socket path\n", PROGRAM_NAME);
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
    int sfd;
    struct sockaddr_un server_addr;

    parse_options(argc, argv);

    /* Create Unix socket */
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error("socket");

    /* Initialize server_addr structure with pathname set to sock_path */
    memset(&server_addr, 0, sizeof(struct sockaddr_un)); /* clear structure */
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, sock_path, sizeof(server_addr.sun_path)-1);

    /* Attempt connection to a socket binded to path_name */
    if (connect(sfd, &server_addr, sizeof(struct sockaddr_un)) == -1)
        handle_error("connect");

    printf("Connected to: %s\n", server_addr.sun_path);

    if (close(sfd) == -1)
        handle_error("close");

    exit(EXIT_SUCCESS);
}
