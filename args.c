#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "args.h"

#define PROGRAM_NAME "mygrep"

struct opts opt;

static char *prog_doc =

    "\nmulti-threaded grep.\n\n"
    "USAGE\n\n" PROGRAM_NAME
    " [OPTIONS] "
    "<stringtosearch> <filesearchfrom>\n"
    "OPTIONS\n"
    "-v <invert> - skip lines containing the \"invert\" pattern\n"
    "-s          - single-thread (disable multi-threading)\n"
    "-h          - print this help text\n\n"
    "EXAMPLE\n"
    "Print all lines in the file mygrep.c containing the string\n"
    "'include', but skip lines containing the string '<stdio.h>':\n\n"
    "./" PROGRAM_NAME " -v '<stdio.h>' 'include' mygrep.c\n\n";

void print_usage(void)
{
    system("clear");
    printf("\033[0;34m %s \033[0m", prog_doc);
}

int parse_args(int argc, char **argv)
{
    int c;

    /* Default values. */
    opt.vstring = NULL;
    opt.single = 0;

    /* Parse optional arguments */
    opterr = 0;
    while ((c = getopt(argc, argv, "hsdv:")) != -1)
    {

        switch (c)
        {

        case 'v':
            opt.vstring = optarg;
            opt.vlen = strlen(opt.vstring);
            break;

        case 's':
            opt.single = 1;
            break;

        case 'h':

            return 0;

        case '?':
            fprintf(stderr, "Error: unknown option -%c\n\n", optopt);
            return 0;

        default:
            return 0;
        }
    }

    if (argc != optind + 2)
    {
        puts("Error: wrong number of mandatory arguments");
        return 0;
    }

    /* Mandatory arguments */
    opt.needle = argv[optind++];
    opt.needlen = strlen(opt.needle);
    opt.filename = argv[optind];

    return 1;
}