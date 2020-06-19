#include "../common.h"
#include "../protocol/io.h"

typedef struct {
    unsigned id;
    int cur, mem;
    char**paths;
}user_command, *ucmd_t;

ucmd_t user_input_to_command(char buffer[LENBUFFER]);
void del_active_command(ucmd_t cmd);
operation_status ftp_c_pwd(packet_t chp, int sfd);
operation_status ftp_c_ls(packet_t chp, int sfd);
operation_status ftp_c_cd(packet_t chp, int sfd, char*path);
operation_status ftp_c_get(packet_t chp, int sfd, char*path);
operation_status ftp_c_put(packet_t chp, int sfd, char*path);
operation_status ftp_c_mkdir(packet_t chp, int sfd, char*path);
