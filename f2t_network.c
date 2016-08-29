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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>

int f2t_net_socket_tcp_nodelay(int sockfd)
{
    int on = 1;
    int ret;

    ret = setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &on, sizeof(on));
    if (ret == -1) {
        perror("setsockopt");
        return -1;
    }

    return 0;
}

int f2t_net_socket_create(int family, int nonblock)
{
    int fd;

    /* create the socket and set the nonblocking flag status */
    fd = socket(family, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    if (nonblock) {
        f2t_net_socket_tcp_nodelay(fd);
    }

    return fd;
}


/* Connect to a TCP socket server and returns the file descriptor */
int f2t_net_tcp_connect(char *host, unsigned long port)
{
    int socket_fd = -1;
    int ret;
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    char _port[6];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(_port, sizeof(_port), "%lu", port);
    ret = getaddrinfo(host, _port, &hints, &res);
    if (ret != 0) {
      fprintf(stderr, "net_tcp_connect: Can't get addr info\n");
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        socket_fd = f2t_net_socket_create(rp->ai_family, 0);
        if (socket_fd == -1) {
            fprintf(stderr, "Error creating client socket, retrying\n");
            continue;
        }

        if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) == -1) {
            fprintf(stderr, "Cannot connect to %s port %s\n", host, _port);
            close(socket_fd);
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (rp == NULL) {
        return -1;
    }

    return socket_fd;
}

/* Connect to a TCP socket server and returns the file descriptor */
int f2t_net_tcp_fd_connect(int fd, char *host, unsigned long port)
{
    int ret;
    struct addrinfo hints;
    struct addrinfo *res;
    char _port[6];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(_port, sizeof(_port), "%lu", port);
    ret = getaddrinfo(host, _port, &hints, &res);
    if (ret != 0) {
        fprintf(stderr, "net_tcp_connect: Can't get addr info\n");
        return -1;
    }

    ret = connect(fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    return ret;
}
