/*
 * Server-side of ping-pong project
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
#include <fcntl.h>
#include <getopt.h>

#include "pingpong.h"


#define PROGRAM_NAME "pp-server"

#define LISTEN_BACKLOG 5

static char *short_options = "hvl:u::";
static struct option long_options[] = {
    {"log",     required_argument, 0, 'l'},
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
static char *log_path = "log.txt";

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
    printf("%s: usage \n./%s [-hvu] [-l log_path] [-u sock_path]\n",
           PROGRAM_NAME, PROGRAM_NAME);
    puts(("\
\t-h, --help              show this message\n\
\t-u, --unix [pathname]   use Unix socket, bind it to pathname\n\
\t-l, --log [pathname]    write log to pathname\n\
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
            case 'l':
                log_path = optarg;
                break;
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
    int sfd, cfd, log_fd;
    struct sockaddr_un my_addr;

    ssize_t read_cnt, write_cnt, write_cum;
    char recv_buf[BUF_SIZE];
    const char *send_buf = PONG;

    parse_options(argc, argv);

    if (proto == NONE) {
        printf("No protocol specified!\n");
        exit(EXIT_FAILURE);
    }

    /* Open log file */
    log_fd = open(log_path, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
    if (log_fd == -1)
        handle_error("open log");

    /* Create Unix socket */
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error("socket");

    /* Initialize my_addr structure with pathname set to unix_sock_path */
    memset(&my_addr, 0, sizeof(struct sockaddr_un)); /* clear structure */
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, unix_sock_path, sizeof(my_addr.sun_path)-1);

    /* Bind socket to the address */
    unlink(my_addr.sun_path);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1)
        handle_error("bind");

    /* Mark socket as passive socket */
    if (listen(sfd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

    /* Accept an incoming connection */
    cfd = accept(sfd, NULL, NULL);
    if (cfd == -1)
        handle_error("accept");

    do {
        /* Read the message to recv_buf */
        read_cnt=read(cfd, recv_buf, BUF_SIZE);
        if (read_cnt == -1)
            handle_error("read");

        /* Check if there is unread data in the buffer */
        if (ioctl(cfd, SIOCINQ, &read_cnt) == -1)
            handle_error("ioctl");

    } while (read_cnt > 0);

    printf("Received %s\n", recv_buf);

    write_cnt = write_cum = 0;

    while ((write_cnt=write(log_fd, recv_buf+write_cum, strlen(recv_buf)-write_cum)) < strlen(recv_buf)-write_cum) {
        if (write_cnt == -1)
            handle_error("log write");
        write_cum += write_cnt;
    }

    write_cnt = write_cum = 0;

    /* Check if message is PING, and reply with PONG */
    if (strncmp(PING, recv_buf, strlen(PING)) == 0) {
        printf("Sending %s\n", send_buf);
        while ((write_cnt=write(cfd, send_buf+write_cum, strlen(send_buf)-write_cum)) < strlen(send_buf)-write_cum) {
            if (write_cnt == -1)
                handle_error("write");
            write_cum += write_cnt;
        }
    }

    if (close(log_fd) == -1)
        handle_error("log close");

    if (close(cfd) == -1)
        handle_error("close cfd");

    if (unlink(unix_sock_path) == -1)
        handle_error("unlink");

    if (close(sfd) == -1)
        handle_error("close sfd");

    exit(EXIT_SUCCESS);
}
