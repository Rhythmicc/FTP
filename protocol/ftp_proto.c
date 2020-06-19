#include "ftp_proto.h"
#include "../utils/sysutil.h"
#include "../utils/strutil.h"
#include "ftp_codes.h"
#include "command_map.h"
#include "ctrl.h"

/// 子进程不断的从FTP客户端接收FTP指令，并给与回应
void handle_proto(session_t s) {
    /// 往客户端写
    ftp_reply(s, FTP_GREET, "(RhythmLian's FtpServer)");
    while (1) {
        session_reset_command(s); /// 清空session中字符串内容

        /// 开始计时
        start_signal_alarm_ctrl_fd();

        /// 接受命令
        int ret = readline(s->peer_fd, s->command, MAX_COMMAND);
        if (ret == -1) {
            if (errno == EINTR) continue;
            ERR_EXIT("readline");
        } else if (ret == 0) exit(EXIT_SUCCESS);
        str_strip(s->command);
        str_split(s->command, s->com, s->args, ' ');
        str_upper(s->com);
        printf("CMD=[%s], ARGS=[%s]\n", s->com, strcmp(s->com, "PASS")?s->args: "NOT SHOW");

        do_command_map(s); //执行命令映射
    }
}
