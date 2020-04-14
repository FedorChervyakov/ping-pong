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

#include "pingpong.h"


#define PROGRAM_NAME "pp-server"
#define OPTIONS "hv"

#define LISTEN_BACKLOG 5


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
    ssize_t sfd, cfd;
    struct sockaddr_un my_addr;

    int read_cnt, write_cnt;
    char recv_buf[BUF_SIZE];
    const char *send_buf = PONG;

    parse_options(argc, argv);

    /* Create Unix socket */
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error("socket");

    /* Initialize my_addr structure with pathname set to sock_path */
    memset(&my_addr, 0, sizeof(struct sockaddr_un)); /* clear structure */
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, sock_path, sizeof(my_addr.sun_path)-1);

    /* Bind socket to the address */
    unlink(my_addr.sun_path);
    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_un)) == -1)
        handle_error("bind");

    /* Mark socket as passive socket */
    if (listen(sfd, LISTEN_BACKLOG) == -1)
        handle_error("listen");

    write_cnt = 0;

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

    /* Check if message is PING, and reply with PONG */
    if (strncmp(PING, recv_buf, strlen(PING)) == 0) {
        printf("Sending %s\n", send_buf);
        while ((write_cnt=write(cfd, send_buf+write_cnt, strlen(send_buf)-write_cnt)) < strlen(send_buf)) {
            if (write_cnt == -1)
                handle_error("write");
        }
    }

    if (close(cfd) == -1)
        handle_error("close cfd");

    /* Grace exit */
    if (unlink(sock_path) == -1)
        handle_error("unlink");

    if (close(sfd) == -1)
        handle_error("close sfd");

    exit(EXIT_SUCCESS);
}
