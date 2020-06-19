#include "FtpServer.h"
static size_t size_of_packet = sizeof(packet);

cinfo_t new_client_info(int sfd, int cid) {
    cinfo_t res = (cinfo_t) malloc(sizeof(client_info));
    res->sfd = sfd;
    res->cid = cid;
    return res;
}

operation_status ftp_s_pwd(packet_t shp, int sfd_client, char*lpwd) {
    shp->type = DATA;
    strcpy(shp->buffer, lpwd);
    if (send(sfd_client, shp, size_of_packet, 0) != size_of_packet)return (operation_status) {FAIL, "send error"};
    return (operation_status) {SUCCESS, "success"};
}

operation_status ftp_s_cd(packet_t shp, int sfd_client, char*lpwd) {
    shp->type = INFO;
    strcpy(shp->buffer, lpwd);
    if (send(sfd_client, shp, size_of_packet, 0) != size_of_packet)return (operation_status) {FAIL, "send error"};
    return (operation_status) {SUCCESS, "success"};
}

operation_status ftp_s_ls(packet_t shp, int sfd_client, char*lpwd) {
    shp->type = DATA;
    DIR *dir = opendir(lpwd);
    struct dirent *e;
    int error_flag = 0;
    while (e = readdir(dir), e) {
        if(e->d_type == 4) sprintf(shp->buffer, CYAN_MSG, e->d_name);
        else sprintf(shp->buffer, "%s", e->d_name);
        if (send(sfd_client, shp, size_of_packet, 0) != size_of_packet) error_flag = 1;
    }
    operation_status res = send_EOT(shp, sfd_client);
    if (!res.status)return res;
    return error_flag ? (operation_status) {FAIL, "send error"} : (operation_status) {SUCCESS, "success"};
}

operation_status ftp_s_get(packet_t shp, int sfd_client) {
    int file_fd = open(shp->buffer, O_RDONLY);
    if (file_fd < 0) {
        send_EOT(shp, sfd_client);
        return (operation_status) {FAIL, "no file error"};
    }
    shp->type = DATA;
    shp->command = GET;
    return send_file(shp, sfd_client, file_fd);
}

operation_status ftp_s_put(packet_t shp, int sfd_client) {
    return recv_file(shp, sfd_client);
}

operation_status ftp_s_mkdir(packet_t shp, int sfd_client) {
    DIR *dir = opendir(shp->buffer);
    int success_flag = 0;
    if (dir) {
        strcpy(shp->buffer, "already exists!");
        closedir(dir);
    } else if (mkdir(shp->buffer, 777) == -1) strcpy(shp->buffer, "wrong path!");
    else {
        shp->type = INFO;
        strcpy(shp->buffer, "success");
        success_flag = 1;
    }
    if (send(sfd_client, shp, size_of_packet, 0) != size_of_packet)
        return (operation_status) {FAIL, "Return info failed"};
    return success_flag ? (operation_status) {SUCCESS, "success"} :
           (operation_status) {FAIL, dir ? "already exists" : "wrong path"};
}
