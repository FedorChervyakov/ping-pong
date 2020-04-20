/*
 * Server-side of ping-pong project
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "pingpong.h"


#define PROGRAM_NAME "server"

#define LISTEN_BACKLOG 5

static char *short_options = "l:u::L::46hv";
static struct option long_options[] = {
    {"log",     required_argument, 0, 'l'},
    {"unix",    optional_argument, 0, 'u'},
    {"listen",  optional_argument, 0, 'L'},
    {"help",    no_argument,       0, 'h'},
    {"version", no_argument,       0, 'v'},
    {0,         0,                 0, 0}
};

static int sock_domain, sock_type;
static char unix_sock_path[108];
static char hostname[254];
static char service[16];
static int ai_family = AF_UNSPEC;

static char *log_path = "log.txt";

static void print_version (void);
static void print_usage (void);
static void parse_options (int argc, char **argv);
static void parse_unix_sock_addr (const char *optarg);
static void parse_inet_sock_addr (const char *optarg);

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
\t-h, --help               show this message\n\
\t-u, --unix [pathname]    use Unix socket, bind it to pathname\n\
\t-L, --listen [addr:port] use TCP socket, bind it to addr:port\n\
\t-4                       force IPv4\n\
\t-6                       force IPv6\n\
\t-l, --log [pathname]     write log to pathname\n\
\t-v, --version            show version"));
    return;
}

static void
parse_unix_sock_addr (const char *optarg)
{
    char *env_var;

    /* Choose a path to bind to */
    if (optarg) {
        strcpy(unix_sock_path, optarg);
    //} else if ((char *env_var = getenv("UNIX_SOCKET")) != NULL) {
    } else if ((env_var = getenv("UNIX_SOCKET")) != NULL) {
        /* Use env var if set */
        strcpy(unix_sock_path, env_var);
    } else {
        strcpy(unix_sock_path, UNIX_SOCKET_PATH);
    }

    return;
}

static void
parse_inet_sock_addr (const char *optarg)
{
    if (optarg) {
        /* Locate last colon ":" in the optarg */
        char *colon_loc = strrchr(optarg, ':');
        
        if (colon_loc != NULL) {
            /* colon is present, assuming arg is in host:port format */
            strncpy(hostname, optarg, colon_loc-optarg);
            strcpy(service, colon_loc+1);
        } else {
            /* colon is not present */
            fputs("parse_inet_sockaddr: use host:port format", stderr);
            exit(EXIT_FAILURE);
        }
    } else {
        strcpy(hostname, INET_SOCKET_HOST);
        strcpy(service, INET_SOCKET_PORT);
    }

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
                if (sock_domain || sock_type)
                    goto conflicting_options;

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

                sock_domain = AF_UNIX;
                sock_type = SOCK_STREAM;

                parse_unix_sock_addr(temp_arg);

                break;
            case 'L':
                if (sock_domain || sock_type)
                    goto conflicting_options;

                if (!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    /* For the explanation, see the comment in the 'u' case */
                    temp_arg = argv[optind++];
                }

                sock_domain = AF_INET;
                sock_type = SOCK_STREAM;

                parse_inet_sock_addr(temp_arg);

                break;
            case '6':
                if (ai_family != AF_UNSPEC)
                    goto conflicting_options;

                ai_family = AF_INET6;
                break;
            case '4':
                if (ai_family != AF_UNSPEC)
                    goto conflicting_options;

                ai_family = AF_INET;
                break;
            default:
                goto exit_failure;
        }
    }

    if (!sock_domain) {
        fprintf(stderr, "%s: no socket domain specified\n", argv[0]);
        goto exit_failure;
    }

    return;

conflicting_options:
    fprintf(stderr, "%s: conficting options specified\n", argv[0]);

exit_failure:
    fprintf(stderr, "Try '%s -h' for more information.\n", argv[0]);
    exit(EXIT_FAILURE);
}

int
main (int argc, char **argv)
{
    int sfd, cfd, log_fd;
    struct addrinfo *rp;

    ssize_t read_cnt, write_cnt, write_cum;
    char recv_buf[BUF_SIZE];
    const char *send_buf = PONG;

    parse_options(argc, argv);

    /* Open log file */
    log_fd = open(log_path, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
    if (log_fd == -1)
        handle_error("open log");

    struct sockaddr_un unix_addr;

    int err;
    struct addrinfo *inet_result;
    struct addrinfo hints;

    /* Create and bind socket to the address */
    switch (sock_domain) {
        case AF_UNIX:
            /* Initialize unix_addr structure */
            memset(&unix_addr, 0, sizeof(struct sockaddr_un));
            unix_addr.sun_family = AF_UNIX;
            strncpy(unix_addr.sun_path, unix_sock_path, sizeof(unix_addr.sun_path)-1);

            unlink(unix_addr.sun_path);

            sfd = socket(sock_domain, sock_type, 0);
            if (sfd == -1)
                handle_error("socket");

            if (bind(sfd, (struct sockaddr *) &unix_addr, sizeof(struct sockaddr_un)) == -1)
                handle_error("bind");

            break;
        case AF_INET:
            /* Initialize hints structure */
            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_family = ai_family;
            hints.ai_socktype = sock_type;
            hints.ai_protocol = 0;
            hints.ai_canonname = NULL;
            hints.ai_addr = NULL;
            hints.ai_next = NULL;

            if ((err=getaddrinfo(hostname, service, &hints, &inet_result)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
            }

            for (rp = inet_result; rp != NULL; rp = rp->ai_next) {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (sfd == -1)
                    continue;

                if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                    break;

                close(sfd);
            }

            if (rp == NULL)
                handle_error("bind");
            break;
        default:
            exit(EXIT_FAILURE);
    }

    freeaddrinfo(inet_result);

    /* Mark socket as passive socket */
    if (listen(sfd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

    /* Accept an incoming connection */
    cfd = accept(sfd, NULL, NULL);
    if (cfd == -1)
        handle_error("accept");

    do {
        /* Read the message to recv_buf */
        read_cnt=recv(cfd, recv_buf, BUF_SIZE, 0);
        if (read_cnt == -1)
            handle_error("recv");

        /* Check if there is unread data in the buffer */
        if (ioctl(cfd, FIONREAD, &read_cnt) == -1)
            handle_error("ioctl");

    } while (read_cnt > 0);

    printf("Received %s\n", recv_buf);

    write_cnt = write_cum = 0;
    while ((write_cnt=write(log_fd, recv_buf+write_cum, strlen(recv_buf)-write_cum)) < strlen(recv_buf)-write_cum) {
        if (write_cnt == -1)
            handle_error("log write");
        write_cum += write_cnt;
    }


    /* Check if message is PING, and reply with PONG */
    write_cnt = write_cum = 0;
    if (strncmp(PING, recv_buf, strlen(PING)) == 0) {
        printf("Sending %s\n", send_buf);
        while ((write_cnt=send(cfd, send_buf+write_cum, strlen(send_buf)-write_cum, 0)) < strlen(send_buf)-write_cum) {
            if (write_cnt == -1)
                handle_error("write");
            write_cum += write_cnt;
        }
    }

    if (close(log_fd) == -1)
        handle_error("log close");

    if (close(cfd) == -1)
        handle_error("close cfd");

    if (strlen(unix_sock_path) > 0) {
        if (unlink(unix_sock_path) == -1)
            handle_error("unlink");
    }

    if (close(sfd) == -1)
        handle_error("close sfd");

    exit(EXIT_SUCCESS);
}
