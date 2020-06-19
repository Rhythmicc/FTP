#include "FtpClient.h"
#include "../configure/conf.h"
#include "../configure/parse_conf.h"

void deal_ctrl_c(int signum) {
    if(ctrl_c_flag) {
        printf(error_msg, "建议通过'quit'命令退出客户端，如遇卡死请再次执行Ctrl C");
        return;
    }
    cli_quit(registered_cli);
    ERR_EXIT("Force Exit");
}

int main(int argc, char **argv) {
    signal(SIGINT, deal_ctrl_c);                            /// 处理Ctrl C

    load_configure("../configure/ftp.conf");          /// 载入配置表
    if(argc > 1) strcpy(ftp_server_addr, argv[1]);          /// 命令行设置目的ip
    if(argc > 2) ftp_listen_port = atoi(argv[2]);           /// 命令行设置目的端口
    registered_cli = (cli_t) malloc(sizeof(ClientSession)); /// 注册客户端
    if(cli_start(registered_cli) < 0) {                     /// 初始化客户端（测试是否连接成功）
        cli_quit(registered_cli);
        return 1;
    }
    while (1) {                                             /// 客户端主循环
        printf("ftp> ");
        reset_cli(registered_cli);
        fgets(registered_cli->command, MAX_COMMAND, stdin);
        int res = cli_do_command(registered_cli);           /// 任务交给cli_do_command执行
        if(res<=0) break;
    }
    cli_quit(registered_cli);
    return 0;
}
