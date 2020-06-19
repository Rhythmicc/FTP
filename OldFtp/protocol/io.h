#pragma once

#include "../common.h"

typedef enum {
    REQU, DONE,
    INFO, TERM,
    DATA, EOT
} ftp_status;

typedef enum {
    GET, PUT,
    CD, LS,
    MKDIR, PWD,
    HELP, QUIT
} ftp_command;

typedef struct packet {
    int conid, datalen;
    ftp_status  type;
    ftp_command command;
    char buffer[LENBUFFER];
}packet, *packet_t;

operation_status send_EOT(packet_t, int);
operation_status send_TERM(packet_t, int);
operation_status send_file(packet_t, int, int);
operation_status recv_file(packet_t hp, int sfd);

typedef struct {
    struct sockaddr_in addr;
    int ctrl_fd;
    int link_fd;
    int is_linking;
    int is_sending;
    int is_recving;
} DataCore;

