#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pwd.h>
#include <dirent.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>

#ifdef linux
#include <sys/syscall.h>
#include <bits/syscall.h>
#include <sys/sendfile.h>
#include <linux/capability.h>
#include <shadow.h>
#include "crypt.h"
#endif

#define error_msg "\033[31m%s\033[0m\n"
#define success_msg "\033[32m%s\033[0m"
#define CYAN_MSG "\033[36m%s\033[0m"
#define CYAN "\033[36m"
#define RED  "\033[31m"
#define RST  "\033[0m"
#define ERR_EXIT(msg) do{printf(error_msg, msg);exit(1);}while(0)
#define MAX_COMMAND 1024
