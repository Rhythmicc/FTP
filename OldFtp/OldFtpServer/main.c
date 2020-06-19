#include "FtpServer.h"
#include "sys/signal.h"
#include "../configure/conf.h"
#include "../configure/parse_conf.h"

size_t size_of_sockaddr = sizeof(struct sockaddr), size_of_packet = sizeof(packet);
int sfd_server;

void service_for_client(cinfo_t ci) {
    int sfd_client, connection_id, x;
    packet_t shp = (packet_t) malloc(size_of_packet);
    char lpwd[LENBUFFER];
    sfd_client = ci->sfd;
    connection_id = ci->cid;
    if(fork() == 0) {
        while (1) {
            if (recv(sfd_client, shp, size_of_packet, 0) == 0) {
                printf("\033[32m[ACTION]\033[0m client closed/terminated. closing connection.\n");
                break;
            }
            if (shp->type == TERM) break;
            if (shp->conid == -1) shp->conid = connection_id;
            if (shp->type == REQU) {
                operation_status status;
                switch (shp->command) {
                    case PWD:
                        if (!getcwd(lpwd, sizeof lpwd)) printf(error_msg, "[ERROR ] getcwd failed");
                        status = ftp_s_pwd(shp, sfd_client, lpwd);
                        break;
                    case CD:
                        if ((x = chdir(shp->buffer)) == -1)
                            printf(error_msg, "[ERROR ] Wrong path.");
                        status = ftp_s_cd(shp, sfd_client, x == -1 ? "fail" : "success");
                        break;
                    case MKDIR:
                        status = ftp_s_mkdir(shp, sfd_client);
                        break;
                    case LS:
                        if (!getcwd(lpwd, sizeof lpwd))printf(error_msg, "[ERROR ] getcwd failed");
                        status = ftp_s_ls(shp, sfd_client, lpwd);
                        break;
                    case GET:
                        status = ftp_s_get(shp, sfd_client);
                        break;
                    case PUT:
                        status = ftp_s_put(shp, sfd_client);
                        break;
                    default:
                        break;
                }
                if (!status.status) printf(error_msg, status.msg);
            } else {
                printf(error_msg, "[ERROR ] accept unsafe operation");
                send_TERM(shp, sfd_client);
                break;
            }
        }
        close(sfd_client);
        free(ci);
        free(shp);
        fflush(stdout);
        exit(0);
    }
}

void deal_sigint(int sig) {
    while (wait(NULL) > 0);
    close(sfd_server);
    exit(0);
}

int main(int argc, char **argv) {
    struct sockaddr_in sin_server, sin_client;
    int sfd_client;
    short int connection_id = 0;

    if((sfd_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf(error_msg, "[ERROR ] create socket failed");
        return 1;
    }

    load_configure("../configure/ftp.conf");

    memset((char*) &sin_server, 0, sizeof(struct sockaddr_in));
    sin_server.sin_family = AF_INET;
    sin_server.sin_port = htons(ftp_listen_port);
    sin_server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sfd_server, (struct sockaddr*) &sin_server, size_of_sockaddr) < 0) {
        printf(error_msg, "[ERROR ] bind port failed");
        return 1;
    }
    if(listen(sfd_server, 1) < 0) {
        printf(error_msg, "[ERROR ] listen failed");
        return 1;
    }

    signal(SIGINT, deal_sigint);
    printf("[ INFO ] FTP Server started up @ %s:%d. Waiting for client(s)...\n\n", ftp_listen_addr, ftp_listen_port);

    while(1) {
        if((sfd_client = accept(sfd_server, (struct sockaddr*) &sin_client, (socklen_t*)&size_of_sockaddr)) < 0) printf(error_msg, "[ERROR ]accpet failed");
        printf("\033[32m[ACTION]\033[0m linking with %s:%d\n", inet_ntoa(sin_client.sin_addr), ntohs(sin_client.sin_port));
        fflush(stdout);
        cinfo_t ci = new_client_info(sfd_client, connection_id++);
        service_for_client(ci);
    }
}
