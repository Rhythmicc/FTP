#include "FtpClient.h"

size_t size_of_sockaddr = sizeof(struct sockaddr);
size_t size_of_packet = sizeof(packet);

int main(int argc, char **argv) {
    struct sockaddr_in sin_server;
    int sfd_client, client_alive = 1;
    packet_t chp = (packet_t) malloc(size_of_packet);
    memset(chp, 0, sizeof(struct sockaddr_in));

    if((sfd_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        printf(error_msg, "create socket failed");
    memset((char*) &sin_server, 0, sizeof(struct sockaddr_in));
    sin_server.sin_family = AF_INET;
    sin_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin_server.sin_port = htons(PORT);

    if(connect(sfd_client, (struct sockaddr*) &sin_server, size_of_sockaddr) < 0)
        printf(error_msg, "connect failed");
    printf("[ INFO ] FTP Client started up, server: 127.0.0.1:%d.\n\n", PORT);

    ucmd_t cmd;
    char uinput[LENBUFFER];
    operation_status status;
    while (client_alive) {
        printf("> ");
        fgets(uinput, LENBUFFER, stdin);
        cmd = user_input_to_command(uinput);
        if (!cmd) continue;
        switch (cmd->id) {
            case GET:
                if (cmd->paths)status = ftp_c_get(chp, sfd_client, *cmd->paths);
                else printf(error_msg, "No file path input");
                break;
            case PUT:
                if (cmd->paths)status = ftp_c_put(chp, sfd_client, *cmd->paths);
                else printf(error_msg, "No file path input");
                break;
            case CD:
                if (cmd->paths)status = ftp_c_cd(chp, sfd_client, *cmd->paths);
                else printf(error_msg, "No file path input");
                break;
            case PWD:
                status = ftp_c_pwd(chp, sfd_client);
                break;
            case LS:
                status = ftp_c_ls(chp, sfd_client);
                break;
            case MKDIR:
                if (cmd->paths)status = ftp_c_mkdir(chp, sfd_client, *cmd->paths);
                else printf(error_msg, "No file path input");
                break;
            case HELP:
                puts("get   <path>\n"
                     "put   <path>\n"
                     "cd    <path>\n"
                     "mkdir <path>\n"
                     "pwd | ls");
                status.status = SUCCESS;
                break;
            case QUIT:
                client_alive = 0;
                status.status = SUCCESS;
                break;
            default:
                break;
        }
        if (!status.status) printf(error_msg, status.msg);
        del_active_command(cmd);
    }
    return 0;
}
