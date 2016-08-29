/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  File 2 TCP
 *  -----------------
 *  Copyright (C) 2016, Eduardo Silva P. <eduardo@monkey.io>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <getopt.h>

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD  "\033[1m"

struct f2t_config {
    /* Source file */
    char *file_path;
    size_t file_size;

    /* Target server */
    int net_conc;
    char *net_host;
    int  net_port;
};

#define f2t_debug(fmt, ...)  fprintf(stderr, "[f2t] " fmt "\n", __VA_ARGS__)

static int print_err(char *msg)
{
    fprintf(stderr, "%s\n\n", msg);
    exit(EXIT_FAILURE);
}

static void f2t_help(int rc)
{
    printf("Usage: file2tcp [OPTIONS]\n\n");
    printf("%sAvailable Options%s\n", ANSI_BOLD, ANSI_RESET);
    printf("  -f  --file=FILE\tinput file to send\n");
    printf("  -h, --host=HOST\tset target host (default: 127.0.0.1)\n");
    printf("  -p, --port=TCP_PORT\tset target TCP port (default: 24224)\n");
    printf("  -c, --conc=N\t\tnumber of parallel workers\n\n");
    exit(rc);
}

static char *f2t_hr_size(char *buf, double size)
{
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units[i]);

    return buf;
}

int f2t_bench_start(struct f2t_config *c)
{
    int file_fd;
    int sock;
    int ret;
    struct stat st;
    char buf[32];
    long bytes, written =0;
    int count = 1024000; /* 1M */
    off_t *file_offset = 0;
    float progress;

    ret = stat(c->file_path, &st);
    if (ret == -1) {
        perror("stat");
        return -1;
    }

    /* Get the file size */
    f2t_hr_size(buf, st.st_size);
    f2t_debug("file size: %lu bytes (%s)", st.st_size, buf);
    f2t_debug("host/port: %s:%i", c->net_host, c->net_port);

    /* Open the input file */
    file_fd = open(c->file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        return -1;
    }

    sock = f2t_net_tcp_connect(c->net_host, c->net_port);
    if (sock == -1) {
        return -1;
    }

    do {
        bytes = sendfile(sock, file_fd, file_offset, count);
        if (bytes > 0) {
            written += bytes;
            progress = ((written * 100.00) / st.st_size);
            printf("\r[f2t] progress: %2.2f%% (%li/%li bytes)",
                   progress, written, st.st_size);
            fflush(stdout);
        }

    } while (bytes > 0);

    close(sock);
    close(file_fd);
}

int main(int argc, char **argv)
{
    int fd;
    int opt;
    int ret;
    struct f2t_config *config;

    /* Setup long-options */
    static const struct option long_opts[] = {
        { "file" , required_argument, NULL, 'f' },
        { "host" , required_argument, NULL, 'h' },
        { "port" , required_argument, NULL, 'p' },
        { "conc" , required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0 }
    };

    if (argc < 3) {
        f2t_help(EXIT_FAILURE);
    }

    config = calloc(1, sizeof(struct f2t_config));
    if (!config) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Parse the command line options */
    while ((opt = getopt_long(argc, argv, "f:h:p:c:",
                              long_opts, NULL)) != -1) {

        switch (opt) {
        case 'f':
            config->file_path = strdup(optarg);
            break;
        case 'h':
            config->net_host = strdup(optarg);
            break;
        case 'p':
            config->net_port = atoi(optarg);
            break;
        case 'c':
            config->net_conc = atoi(optarg);
            break;
        }
    }

    if (!config->file_path) {
        f2t_help(EXIT_FAILURE);
    }
    else {
        ret = access(config->file_path, R_OK);
        if (ret == -1) {
            perror("access");
            return -1;
        }
    }

    if (!config->net_host) {
        config->net_host = strdup("127.0.0.1");
    }

    if (config->net_port <= 0) {
        config->net_port = 24224;
    }

    if (config->net_conc <= 0) {
        config->net_conc = 1;
    }

    f2t_bench_start(config);

    return 0;
}
