#pragma once
#include "../common.h"
#include "../protocol/io.h"
#include <time.h>
#include <sys/stat.h>


typedef struct {
    int sfd, cid;
} client_info, *cinfo_t;

cinfo_t new_client_info(int, int);

/**
 * API
 */

operation_status ftp_s_pwd(packet_t shp, int sfd_client, char*lpwd);
operation_status ftp_s_cd(packet_t shp, int sfd_client, char*lpwd);
operation_status ftp_s_ls(packet_t shp, int sfd_client, char*lpwd);
operation_status ftp_s_get(packet_t shp, int sfd_client);
operation_status ftp_s_put(packet_t shp, int sfd_client);
operation_status ftp_s_mkdir(packet_t shp, int sfd_client);
