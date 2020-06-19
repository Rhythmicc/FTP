#include "FtpClient.h"

static const char*cmd_ls[] = {
        "get", "put",
        "cd", "ls",
        "mkdir", "pwd",
        "?", "quit"
};

static size_t size_of_packet = sizeof(packet);

ucmd_t user_input_to_command(char buffer[LENBUFFER]) {
    ucmd_t cmd = (ucmd_t)malloc(sizeof(user_command));
    cmd->id = -1;
    cmd->cur = 0;
    cmd->mem = 10;
    cmd->paths = (char **)malloc(sizeof(char *) * cmd->mem);
    char*rest_buffer, *token;
    token = strtok_r(buffer, " \t\n", &rest_buffer);
    while (token) {
        if(cmd->id == -1){ for(int i=0;i<8;++i)if(!strcmp(token, cmd_ls[i])){
            cmd->id = i;
            break;
        }} else {
            if(cmd->cur == cmd->mem) {
                cmd->mem += 10;
                cmd->paths = (char **)realloc(cmd->paths, sizeof(char*) * cmd->mem);
            }
            cmd->paths[cmd->cur] = (char*)malloc(sizeof(char) * (strlen(token) + 1));
            strcpy(cmd->paths[cmd->cur++], token);
        }
        token = strtok_r(NULL, " \t\n", &rest_buffer);
    }
    if(cmd->id >= 0) return cmd;
    else {
        free(cmd->paths);
        free(cmd);
        return NULL;
    }
}

void del_active_command(ucmd_t cmd) {
    for(int i=0;i<cmd->cur;++i)free(cmd->paths[i]);
    free(cmd->paths);
    free(cmd);
}

operation_status ftp_c_pwd(packet_t chp, int sfd) {
    chp->type = REQU;
    chp->conid = -1;
    chp->command = PWD;
    if(send(sfd, chp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    if(recv(sfd, chp, size_of_packet, 0) <= 0) return (operation_status) {FAIL, "recv msg failed"};
    if(chp->type == DATA && chp->command == PWD && chp->buffer[0]){
        printf("%s\n", chp->buffer);
        return (operation_status) {SUCCESS, "success"};
    } else return (operation_status) {FAIL, "error in recv msg"};
}

operation_status ftp_c_ls(packet_t chp, int sfd) {
    chp->type = REQU;
    chp->conid = -1;
    chp->command = LS;
    int success_flag = 1;
    if(send(sfd, chp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    while (chp->type != EOT) {
        if(recv(sfd, chp, size_of_packet, 0) <= 0) success_flag = 0;
        if(chp->type == DATA && chp->command == LS && chp->buffer[0]) printf("%s\t", chp->buffer);
    }
    puts("");
    return success_flag? (operation_status) {SUCCESS, "success"}:
    (operation_status) {FAIL, "exists broken block while recving"};
}

operation_status ftp_c_cd(packet_t chp, int sfd, char*path) {
    chp->type = REQU;
    chp->conid = -1;
    chp->command = CD;
    strcpy(chp->buffer, path);
    if(send(sfd, chp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    if(recv(sfd, chp, size_of_packet, 0) <= 0) return (operation_status) {FAIL, "recv msg failed"};
    if(chp->type == INFO && chp->command == CD && !strcmp(chp->buffer, "success"))
        return (operation_status) {SUCCESS, "success"};
    return (operation_status) {FAIL, "error excuting command on server."};
}

operation_status ftp_c_get(packet_t chp, int sfd, char*path) {
    chp->type = REQU;
    chp->conid = -1;
    chp->command = GET;
    strcpy(chp->buffer, path);
    return recv_file(chp, sfd);
}

operation_status ftp_c_put(packet_t chp, int sfd, char*path) {
    int file_fd = open(path, O_RDONLY);
    if(file_fd < 0) return (operation_status) {FAIL, "No file error"};
    chp->type = REQU;
    chp->conid = -1;
    chp->command = PUT;
    strcpy(chp->buffer, path);
    if(send(sfd, chp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    if(recv(sfd, chp, size_of_packet, 0) <= 0) return (operation_status) {FAIL, "recv msg failed"};
    chp->type = DATA;
    return send_file(chp, sfd, file_fd);
}

operation_status ftp_c_mkdir(packet_t chp, int sfd, char*path) {
    chp->type = REQU;
    chp->conid = -1;
    chp->command = MKDIR;
    strcpy(chp->buffer, path);
    if(send(sfd, chp, size_of_packet, 0) != size_of_packet) return (operation_status) {FAIL, "send msg failed"};
    if(recv(sfd, chp, size_of_packet, 0) <= 0) return (operation_status) {FAIL, "recv msg failed"};
    if(chp->type == INFO && chp->command == MKDIR) return (operation_status){strcmp(chp->buffer, "success")?FAIL:SUCCESS, chp->buffer};
    else{
        printf("%d %s\n",chp->type, chp->buffer);
        return (operation_status) {FAIL, "error excuting on server."};
    }
}