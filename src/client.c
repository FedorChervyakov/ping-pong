/*
 * Client-side of ping-pong project
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "pingpong.h"


#define PROGRAM_NAME "client"


static char *short_options = "hvu::";
static struct option long_options[] = {
    {"unix",    optional_argument, 0, 'u'},
    {"help",    no_argument,       0, 'h'},
    {"version", no_argument,       0, 'v'},
    {0,         0,                 0, 0}
};

enum protocol
{
    NONE,
    UNIX,
};

static enum protocol proto = NONE;

static char unix_sock_path[108] = UNIX_SOCKET_PATH;

static void print_version (void);
static void print_usage (void);
static void parse_options (int argc, char **argv);
static void parse_unix_socket_param (const char *optarg);
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
    printf("%s: usage \n./%s [-hvu] [-u sock_path]\n",
           PROGRAM_NAME, PROGRAM_NAME);
    puts(("\
\t-h, --help              show this message\n\
\t-u, --unix [pathname]   use Unix socket, bind it to pathname\n\
\t-v, --version           show version"));
    return;
}


static void
parse_unix_socket_param (const char *optarg)
{
    char *env_var;

    if (! optarg) {
        if ((env_var = getenv("UNIX_SOCKET")) != NULL) {
            /* Use env var if set */
            strcpy(unix_sock_path, env_var);
        }
        return;
    }

    strcpy(unix_sock_path, optarg);
    return;

}

static void
parse_options (int argc, char **argv)
{
    int c;

    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        char *temp_arg = NULL;

        switch (c) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case 'u':
                if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    /* This construct is required to pass optarg
                     * as -u path or --unix path, instead of
                     * default getopt_long behavior that is
                     * -upath or --unix=path.
                     *
                     * Test explaination
                     * No argument is specified, and there is
                     * another string in argv after this option
                     * that does not look like an option
                     */
                    temp_arg = argv[optind++];
                }
                parse_unix_socket_param(temp_arg);
                proto = UNIX;
                break;
            default:
                goto exit_failure;
        }
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

    ssize_t read_cnt, write_cnt, write_cum;
    char recv_buf[BUF_SIZE];
    const char *send_buf = PING;

    parse_options(argc, argv);

    if (proto == NONE) {
        printf("No protocol specified!\n");
        exit(EXIT_FAILURE);
    }
    /* Create Unix socket */
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error("socket");

    /* Initialize server_addr structure with pathname set to unix_sock_path */
    memset(&server_addr, 0, sizeof(struct sockaddr_un)); /* clear structure */
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, unix_sock_path, sizeof(server_addr.sun_path)-1);

    /* Attempt connection to a socket binded to path_name */
    if (connect(sfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1)
        handle_error("connect");

    write_cnt = 0;
    printf("Sending %s\n", send_buf);

    while ((write_cnt=write(sfd, send_buf+write_cum, strlen(send_buf)-write_cum)) < strlen(send_buf)-write_cum) {
        if (write_cnt == -1)
            handle_error("write");
        write_cum += write_cnt;
    }

    do {
        /* Read the message to recv_buf */
        read_cnt=read(sfd, recv_buf, BUF_SIZE);
        if (read_cnt == -1)
            handle_error("read");

        /* Check if there is an unread data in the buffer */
        if (ioctl(sfd, SIOCINQ, &read_cnt) == -1)
            handle_error("ioctl");

    } while (read_cnt > 0);

    printf("Received %s\n", recv_buf);

    if (close(sfd) == -1)
        handle_error("close");

    exit(EXIT_SUCCESS);
}
