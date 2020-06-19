#include "session.h"
#include "../protocol/ftp_proto.h"
#include "../protocol/ftp_nobody.h"
#include "../priv/priv_sock.h"
#include "../configure/conf.h"

void session_init(session_t s) {
    memset(s->command, 0, sizeof(s->command));
    memset(s->com, 0, sizeof(s->com));
    memset(s->args, 0, sizeof(s->args));

    s->ip = 0;
    memset(s->username, 0, sizeof(s->username));

    s->peer_fd = -1;
    s->nobody_fd = -1;
    s->proto_fd = -1;

    s->user_uid = 0;
    s->ascii_mode = 0;

    s->p_addr = NULL;
    s->data_fd = -1;
    s->listen_fd = -1;

    s->restart_pos = 0;
    s->rnfr_name = NULL;

    s->limits_max_upload   = ftp_upload_max_rate * 1024;
    s->limits_max_download = ftp_download_max_rate * 1024;
    s->start_time_sec = 0;
    s->start_time_usec = 0;

    s->is_translating_data = 0;
    s->is_receive_abor = 0;

    s->curr_clients = 0;
    s->curr_ip_clients = 0;
}

void session_reset_command(session_t s) {
    memset(s->command, 0, sizeof(s->command));
    memset(s->com, 0, sizeof(s->com));
    memset(s->args, 0, sizeof(s->args));
}

void session_begin(session_t sess) {
    priv_sock_init(sess);

    pid_t pid;
    if ((pid = fork()) == -1)
        ERR_EXIT("fork");
    else if (pid == 0) {
        priv_sock_set_proto_context(sess);
        handle_proto(sess);
    } else {
        priv_sock_set_nobody_context(sess);
        handle_nobody(sess);
    }
}
