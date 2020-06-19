#include "FtpClient.h"
#include "../configure/conf.h"
#include "../protocol/ftp_codes.h"
#include "../utils/strutil.h"
#include "../utils/sysutil.h"

int ctrl_c_flag = 1;

/**
 * 连接host:port
 * @param host:  ip地址
 * @param port:  端口
 * @return 成功返回sock_fd，失败则退出
 */
int socket_connect(const char*host, unsigned port) {
    int sock_fd;
    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR_EXIT("socket");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if(connect_timeout(sock_fd, &addr, ftp_connect_timeout) < 0) ERR_EXIT("Connect Error or Timeout");
    return sock_fd;
}

/**
 * 获取客户端ip
 * @param ip: 16位字符串
 * @return sock_fd
 */
void cli_local_ip(char*ip) {
    strcpy(ip, "124.234.95.139");
}

typedef struct {
    const char *cmd; //FTP指令
    int (*cmd_handler)(cli_t sess);//该指令所对应的执行函数
} ClientCommmand;

cli_t registered_cli = NULL;

ClientCommmand cmd_map[] = {
        {"USER", cli_user},
        {"PASS", cli_pass},
        {"CWD", cli_cwd},
        {"CD", cli_cwd},
        {"DIR", cli_list},
        {"LS", cli_list},
        {"LIST", cli_list},
        {"MKDIR", cli_mkd},
        {"MKD", cli_mkd},
        {"RETR", cli_retr},
        {"GET", cli_retr},
        {"STOR", cli_stor},
        {"PUT", cli_stor},
        {"HELP", cli_help},
        {"?", cli_help}
};

int cli_user(cli_t cli) {
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) return 0;
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0){
        printf(error_msg, "Read Failed");
        return 0;
    }
    printf(strtol(cli->reply, NULL, 10) == FTP_GIVEPWORD? success_msg: error_msg, cli->reply);
    return 1;
}

int cli_pass(cli_t cli) {
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) return 0;
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0){
        printf(error_msg, "Read Failed");
        return 0;
    }
    printf(strtol(cli->reply, NULL, 10) == FTP_LOGINOK? success_msg: error_msg, cli->reply);
    return 1;
}

int cli_cwd(cli_t cli) {
    sprintf(cli->command, "CWD %s\r\n", cli->arg);
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) return 0;
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0){
        printf(error_msg, "Read Failed");
        return 0;
    }
    printf(strtol(cli->reply, NULL, 10) == FTP_CWDOK? success_msg: error_msg, cli->reply);
    return 1;
}

int cli_mkd(cli_t cli) {
    sprintf(cli->command, "MKD %s\r\n", cli->arg);
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) return 0;
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0){
        printf(error_msg, "Read Failed");
        return 0;
    }
    printf(strtol(cli->reply, NULL, 10) == FTP_MKDIROK? success_msg: error_msg, cli->reply);
    return 1;
}

/**
 * port 模式 (远程访问时，返回的sock_fd无法生效，暂时搁置)
 * @param cli
 * @param pid
 * @return port成功后的sock_fd
 */
int cli_port(cli_t cli) {
    char ip[16], info[MAX_COMMAND];
    cli_local_ip(ip);
    int sock_fd = cli->ctrl_socket_fd;
    unsigned u_ip[4];
    sscanf(ip, "%u.%u.%u.%u", u_ip, u_ip+1, u_ip+2, u_ip+3);
    sprintf(info, "PORT %u,%u,%u,%u,%u,%u\r\n", u_ip[0], u_ip[1], u_ip[2], u_ip[3], sock_fd / 256, sock_fd % 256);
    if(send(cli->ctrl_socket_fd, info, strlen(info), 0) < 0) ERR_EXIT("cli_port: send");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_port: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_PORTOK) printf(success_msg , cli->reply);
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    return sock_fd;
}

/**
 * pasv 模式
 * @param cli
 * @return 成功后返回可用的sock_fd，否则返回负值
 */
int cli_pasv(cli_t cli) {
    if(send(cli->ctrl_socket_fd, "PASV\r\n", 6, 0) < 0) ERR_EXIT("cli_pasv: send");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_pasv: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_PASVOK) printf(success_msg, cli->reply);
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    str_strip(cli->reply);
    unsigned u[6];
    sscanf(cli->reply, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u).", u, u+1, u+2, u+3, u+4, u+5);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    unsigned port = u[4] * 256 + u[5];
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ftp_server_addr);
    if(connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0) ERR_EXIT("cli_pasv: connect");
    return sock;
}

/**
 * 发送TYPE I指令设定传输方式
 * @param cli
 * @return
 */
int cli_type(cli_t cli) {
    if(send(cli->ctrl_socket_fd, "TYPE I\r\n", 8, 0) < 0) ERR_EXIT("cli_type: send");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_type: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_TYPEOK) printf(success_msg, cli->reply);
    else {
        printf(error_msg, cli->reply);
        return 0;
    }
    return 1;
}

/**
 * list命令族
 * @param cli
 * @return 执行结果状态号
 */
int cli_list(cli_t cli) {
    int sock_fd = cli_pasv(cli), res;                  /// 设定pasv工作模式
    if(sock_fd < 0) {
        printf(error_msg, "cli_list: pasv failed");
        return 0;
    }
    memset(cli->reply, 0, MAX_COMMAND);
    if(cli_type(cli) == 0) return 0;                  /// 设定type i
    memset(cli->reply, 0, MAX_COMMAND);
    sprintf(cli->command, "LIST %s\r\n", cli->arg);
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) ERR_EXIT("cli_list: send list");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_port: recv list");
    if(strtol(cli->reply, NULL, 10) == FTP_DATACONN) printf(success_msg, cli->reply);
    else {
        printf(error_msg, cli->reply);
        return 0;
    }
    memset(cli->reply, 0, MAX_COMMAND);
    do {                                              /// 开始接受数据
        res = recv(sock_fd, cli->reply, MAX_COMMAND, 0);
        if(res > 0) {
            printf("%s", cli->reply);
            memset(cli->reply, 0, MAX_COMMAND);
        }
    } while (res > 0);
    memset(cli->reply, 0, MAX_COMMAND);
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_list: recv list"); /// 接受226
    if(strtol(cli->reply, NULL, 10) == FTP_TRANSFEROK) printf(success_msg, cli->reply);
    else {
        printf(error_msg, cli->reply);
        return 0;
    }
    return 1;
}

/**
 * retr命令族
 * @param cli
 * @return
 */
int cli_retr(cli_t cli) {
    int file_fd = open(cli->arg, O_WRONLY | O_CREAT); /// 创建file fd，失败则退出
    if(file_fd < 0) {
        printf(error_msg, "Open or create file failed, try later");
        return 1;
    }

    int sock_fd = cli_pasv(cli), res;                 /// pasv工作模式
    if(sock_fd < 0) {
        printf(error_msg, "cli_retr: pasv failed");
        return 0;
    }

    memset(cli->reply, 0, MAX_COMMAND);
    if(cli_type(cli) == 0) return 0;                  /// type i
    memset(cli->reply, 0, MAX_COMMAND);

    sprintf(cli->command, "RETR %s\r\n", cli->arg);   /// 开始执行retr命令
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) ERR_EXIT("cli_retr: send");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_retr: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_DATACONN) printf(success_msg , cli->reply);
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    char data[65537] = {0};                           /// 开始接收文件数据
    lock_file_write(file_fd);                         /// 对文件加写锁
    do {
        res = recv(sock_fd, data, 65536, 0);
        if(res > 0) write(file_fd, data, res);
    } while (res > 0);
    unlock_file(file_fd);                             /// 解锁
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_retr: recv");  /// 接收 226
    if(strtol(cli->reply, NULL, 10) == FTP_TRANSFEROK) printf(success_msg , cli->reply);
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    return 1;
}

/**
 * stor命令族
 * @param cli
 * @return
 */
int cli_stor(cli_t cli) {
    int file_fd = open(cli->arg, O_RDONLY);             /// 创建读file fd, 失败退出执行
    if(file_fd < 0) {
        printf(error_msg, "No such file!");
        return 1;
    }

    int sock_fd = cli_pasv(cli), res;                   /// pasv
    if(sock_fd < 0) {
        printf(error_msg, "cli_stor: pasv failed");
        return 0;
    }

    memset(cli->reply, 0, MAX_COMMAND);
    if(cli_type(cli) == 0) return 0;                    /// type i
    memset(cli->reply, 0, MAX_COMMAND);

    sprintf(cli->command, "STOR %s\r\n", cli->arg);
    if(send(cli->ctrl_socket_fd, cli->command, strlen(cli->command), 0) < 0) ERR_EXIT("cli_retr: send");
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_retr: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_DATACONN) printf(success_msg , cli->reply);
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    char data[65537] = {0};
    lock_file_read(file_fd);                            /// 加读锁
    do {
        res = read(file_fd, data, 65536);
        if(res > 0) write(sock_fd, data, res);
    } while (res > 0);
    close(sock_fd);
    unlock_file(file_fd);                               /// 解锁
    if(recv(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND, 0) < 0) ERR_EXIT("cli_retr: recv");
    if(strtol(cli->reply, NULL, 10) == FTP_TRANSFEROK) printf(success_msg , cli->reply); /// 接收 226
    else {
        printf(error_msg, cli->reply);
        return -1;
    }
    return 1;
}

/**
 * 输出在命令映射表中的命令，5个一行
 * @return
 */
int cli_help(cli_t cli) {
    int size = sizeof(cmd_map) / sizeof(ClientCommmand);
    printf(CYAN);
    for(int i=0;i<size;++i) printf("%s%c", cmd_map[i].cmd, (i+1)%5?'\t':'\n');
    printf(RST);
    return 1;
}

/**
 * 初始化客户端
 * @param cli: 客户端指针
 * @return 连接失败退出程序，连接成功但返回状态码错误返回-1，否则返回0
 */
int cli_start(cli_t cli) {
    cli->ctrl_socket_fd = socket_connect(ftp_server_addr, ftp_listen_port);
    if(read_timeout(cli->ctrl_socket_fd, ftp_connect_timeout) < 0) ERR_EXIT("Receive msg Timeout: preview");
    if(readline(cli->ctrl_socket_fd, cli->reply, MAX_COMMAND) < 0) ERR_EXIT("Receive msg Timeout: readline");
    str_strip(cli->reply);
    str_split(cli->reply, cli->com, cli->arg, ' ');
    if(atoi(cli->com) != FTP_GREET){
        printf(error_msg, cli->reply);
        return -1;
    } else printf(success_msg "\n", cli->reply);
    return 0;
}

/**
 * 执行cli->command命令
 * @param cli: 客户端指针
 * @return 执行结果，需退出程序时返回0，执行失败时返回-1，否则返回1
 */
int cli_do_command(cli_t cli) {
    str_strip(cli->command);
    str_split(cli->command, cli->com, cli->arg, ' ');
    str_upper(cli->com);
    if(strcmp(cli->com, "QUIT") == 0) return 0;

    memset(cli->reply, 0, sizeof(cli->reply));
    int i, size = sizeof(cmd_map) / sizeof(ClientCommmand); //数组大小
    for (i = 0; i < size; ++i) {
        if (strcmp(cmd_map[i].cmd, cli->com) == 0) {
            if (cmd_map[i].cmd_handler != NULL) {
                strcat(cli->command, "\r\n");
                return cmd_map[i].cmd_handler(cli);
            }
            else {
                printf(error_msg, "Unimplement command."); //该命令没有实现
                return -1;
            }
        }
    }

    if (i == size) {
        printf(error_msg, "Unknown command.");
        return -1;
    }

    return 1;
}

/**
 * 关闭客户端
 * @param cli: 客户端指针
 */
void cli_quit(cli_t cli) {
    if(send(cli->ctrl_socket_fd, "quit\r\n", 6, 0) < 0) printf(error_msg, "send quit error!");
    if(cli->ctrl_socket_fd > 0) close(cli->ctrl_socket_fd);
    if(cli->data_socket_fd > 0) close(cli->data_socket_fd);
    free(cli);
}


/**
 * 清空cli中的字符数组
 * @param cli
 */
void reset_cli(cli_t cli) {
    memset(cli->command, 0, MAX_COMMAND);
    memset(cli->com, 0, MAX_COMMAND);
    memset(cli->arg, 0, MAX_COMMAND);
    memset(cli->reply, 0, MAX_COMMAND);
}