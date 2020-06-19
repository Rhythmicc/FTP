#include "../common.h"
#include "../utils/sysutil.h"
#include "../utils/session.h"
#include "../configure/conf.h"
#include "../configure/parse_conf.h"
#include "../protocol/ftp_assist.h"


void deal_ctrl_c(int signum) {
    remove_hash();
    free(p_sess);
    exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[]) {
    check_permission();                                          /// 使用root权限运行
    setup_signal_chld();                                         /// 处理子进程退出
    load_configure("../configure/ftp.conf");               /// 解析配置文件
    print_conf();                                                /// 输出配置表

    signal(SIGINT, deal_ctrl_c);                                 /// 设置处理Ctrl C
    init_hash();                                                 /// 初始化哈希表

    int listenfd = tcp_server(ftp_listen_addr, ftp_listen_port); /// 创建一个监听fd

    pid_t pid;
    p_sess = (session_t) malloc(sizeof(session));
    session_init(p_sess);

    while (1) {
        struct sockaddr_in addr;
        int ctrl_fd = accept_timeout(listenfd, &addr, ftp_accept_timeout);
        if (ctrl_fd == -1 && errno == ETIMEDOUT) continue;
        else if (ctrl_fd == -1) ERR_EXIT("accept_timeout");

        uint32_t ip = addr.sin_addr.s_addr;
        p_sess->ip = ip;
        add_clients_to_hash(p_sess, ip);                        /// 获取ip地址，并在hash中添加一条记录

        if ((pid = fork()) == -1) ERR_EXIT("fork");
        else if (pid == 0) {                                    /// 每当用户连接上，就fork一个子进程
            close(listenfd);
            p_sess->peer_fd = ctrl_fd;
            limit_num_clients(p_sess);
            session_begin(p_sess);
            exit(EXIT_SUCCESS);                                 /// 每次成功执行后退出循环
        } else {
            add_pid_ip_to_hash(pid, ip);
            close(ctrl_fd);
        }
    }
    return 0;
}
