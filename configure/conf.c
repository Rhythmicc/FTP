#include "conf.h"

int ftp_pasv_enable = 1;
int ftp_port_enable = 1;
unsigned ftp_listen_port = 21;
unsigned ftp_min_data_port=1024;
unsigned ftp_max_data_port=5000;
unsigned ftp_max_clients = 100;
unsigned ftp_max_per_ip  = 20;
unsigned ftp_accept_timeout  = 60;
unsigned ftp_connect_timeout = 60;
unsigned ftp_session_timeout = 300;
unsigned ftp_data_connection_timeout = 300;
unsigned ftp_local_umask = 077;
unsigned ftp_upload_max_rate = 0;
unsigned ftp_download_max_rate = 0;
const char*ftp_listen_addr;
char*ftp_server_addr;
