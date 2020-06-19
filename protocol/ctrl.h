#pragma once

/*
 *限速模块
 */
#include "../utils/session.h"

void limit_cur_rate(session *sess, int nbytes, int is_upload);

//控制连接
void setup_signal_alarm_ctrl_fd();
void start_signal_alarm_ctrl_fd();

//数据连接
void setup_signal_alarm_data_fd();
void start_signal_alarm_data_fd();

void cancel_signal_alarm();

void setup_signal_sigurg();

void do_site_chmod(session *sess, char *args);
void do_site_umask(session *sess, char *args);
void do_site_help(session *sess);

