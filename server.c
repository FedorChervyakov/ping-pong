/*
 * Server-side of ping-pong project
 */

#include <stdio.h>
#include <unistd.h>

#include "pingpong.h"


static char *filename;

static void
version (void)
{
    printf("pp-server: version %d.%d \n", VERSION_MAJOR, VERSION_MINOR);
    return;
}

static void
usage  (void)
{
    printf("pp-server: [-hv]\n");
    return;
}

static void
parse_options (int argc, char **argv)
{
    int c;

    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
            case 'h':
                usage();
                break;
            case 'v': 
                version();
                break;
            default:
                printf("pp-server: Unknown option %s\n", optarg);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    return;
}

int
main (int argc, char **argv)
{
    parse_options(argc, argv);

    return 0;
}
