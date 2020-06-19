#include "io.h"
#include "../utils/sysutil.h"

static size_t size_of_packet = sizeof(packet);

operation_status send_EOT(packet_t hp, int sfd) {
    hp->type = EOT;
    if (send(sfd, hp, size_of_packet, 0) != size_of_packet)return (operation_status) {FAIL, "send error"};
    return (operation_status) {SUCCESS, "success"};
}

operation_status send_TERM(packet_t hp, int sfd) {
    hp->type = TERM;
    if (send(sfd, hp, size_of_packet, 0) != size_of_packet)return (operation_status) {FAIL, "send error"};
    return (operation_status) {SUCCESS, "success"};
}

operation_status send_file(packet_t hp, int sfd, int file_fd) {
    int success_flag = 1;
    lock_file_read(file_fd);
    while ((hp->datalen = read(file_fd, hp->buffer, LENBUFFER)) > 0)
        if (send(sfd, hp, size_of_packet, 0) != size_of_packet) success_flag = 0;
    unlock_file(file_fd);
    close(file_fd);
    operation_status res = send_EOT(hp, sfd);
    if (!res.status) return res;
    return success_flag ? (operation_status) {SUCCESS, "success"} :
           (operation_status) {FAIL, "exist failed send action"};
}

operation_status recv_file(packet_t hp, int sfd) {
    int recv_flag = 1, init_flag = 1;
    int file_fd;
    char filename[LENBUFFER];
    strcpy(filename, hp->buffer);
    if(send(sfd, hp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    do {
        if(recv(sfd, hp, size_of_packet, 0) <= 0) {
            recv_flag = 0;
            break;
        } else if (hp->type == DATA) {
            if (init_flag) {
                file_fd = open(filename, O_WRONLY | O_CREAT);
                lock_file_write(file_fd);
                init_flag = 0;
            }
            write(file_fd, hp->buffer, hp->datalen);
        }
    } while (hp->type == DATA);
    if(init_flag) return (operation_status) {FAIL, "No file error"};
    unlock_file(file_fd);
    close(file_fd);
    if(recv_flag == 0) return (operation_status) {FAIL, "exists broken file block"};
    return hp->type == EOT ? (operation_status) {SUCCESS, "success"}:
           (operation_status) {FAIL ,"Error occured while downloading remote file."};
}
