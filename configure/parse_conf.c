#include "parse_conf.h"
#include "../utils/strutil.h"
#include "../common.h"
#include "conf.h"

typedef struct {
    const char*key;
    int*val;
} bool_setting;

/**
 * bool型配置
 */
static bool_setting bool_array[] = {
        {"pasv_enable", &ftp_pasv_enable},
        {"port_enable", &ftp_port_enable},
        {NULL, NULL}
};

typedef struct {
    const char*key;
    unsigned *val;
} uint_setting;

/**
 * 数字型配置
 */
static uint_setting uint_array[] = {
        {"listen_port", &ftp_listen_port},
        {"min_data_port", &ftp_min_data_port},
        {"max_data_port", &ftp_max_data_port},
        {"max_clients", &ftp_max_clients},
        {"max_per_ip", &ftp_max_per_ip},
        {"accept_timeout", &ftp_accept_timeout},
        {"connect_timeout", &ftp_connect_timeout},
        {"session_timeout", &ftp_session_timeout},
        {"data_connection_timeout", &ftp_data_connection_timeout},
        {"local_umask", &ftp_local_umask},
        {"upload_max_rate", &ftp_upload_max_rate},
        {"download_max_rate", &ftp_download_max_rate},
        {NULL, NULL}
};

typedef struct {
    const char* key;
    const char**val;
} str_setting;

/**
 * 字符串型配置
 */
static str_setting str_array[] = {
        {"listen_address", &ftp_listen_addr},
        {"server_address", &ftp_server_addr},
        {NULL, NULL}
};

/**
 * 载入配置
 * @param setting: 一行配置
 */
static void load_setting(const char*setting) {
    while (isspace(*setting))++setting;
    char key[512] = {0}, val[512] = {0};
    str_split(setting, key, val, '=');              /// 切分，遍历匹配
    if(!val[0]) {
        printf(RED "missing value at: %s\n" RST, key);
        exit(1);
    }

    str_setting *s_ptr = str_array;
    while (s_ptr->key) {
        if(!strcmp(key, s_ptr->key)) {
            if(*s_ptr->val)free((char*)*s_ptr->val);
            *s_ptr->val = strdup(val);
            return;
        }
        ++s_ptr;
    }

    bool_setting *b_ptr = bool_array;
    while (b_ptr->key) {
        if(!strcmp(key, b_ptr->key)) {
            str_upper(val);
            if (strcmp(val, "YES") == 0 || strcmp(val, "TRUE") == 0 || strcmp(val, "1") == 0) *(b_ptr->val) = 1;
            else if (strcmp(val, "NO") == 0 || strcmp(val, "FALSE") == 0 || strcmp(val, "0") == 0) *(b_ptr->val) = 0;
            else {
                printf(RED "wrong bool value: %s\n" RST, key);
                exit(1);
            }
            return;
        }
        ++b_ptr;
    }

    uint_setting* u_ptr = uint_array;
    while (u_ptr->key) {
        if(!strcmp(key, u_ptr->key)) {
            if(val[0] == '0') *(u_ptr->val) = str_octal_to_uint(val);
            else *(u_ptr->val) = atoi(val);
            return;
        }
        ++u_ptr;
    }
}

/**
 * 载入配置表
 * @param path: 配置表地址
 */
void load_configure(const char*path) {
    FILE *f = fopen(path, "r");
    if(!f)ERR_EXIT("fopen failed");
    char line[1024] = {0};
    while (fgets(line, 1024, f) != NULL) {
        if (strlen(line) == 0 || line[0] == '#' || str_all_space(line)) continue;
        str_strip(line);
        load_setting(line);
        memset(line, 0, 1024);
    }
    fclose(f);
}
