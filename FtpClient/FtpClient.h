#pragma once
#include "../common.h"

extern int ctrl_c_flag;

typedef struct {
    int ctrl_socket_fd;
    int data_socket_fd;
    char command[MAX_COMMAND];
    char com[MAX_COMMAND];
    char arg[MAX_COMMAND];
    char reply[MAX_COMMAND];
}ClientSession, *cli_t;

extern cli_t registered_cli;

void reset_cli(cli_t cli);

int cli_start(cli_t cli);
int cli_do_command(cli_t cli);
void cli_quit(cli_t cli);

int cli_user(cli_t cli);
int cli_pass(cli_t cli);
int cli_cwd(cli_t cli);
int cli_list(cli_t cli);
int cli_mkd(cli_t cli);
int cli_retr(cli_t cli);
int cli_stor(cli_t cli);
int cli_help(cli_t cli);
