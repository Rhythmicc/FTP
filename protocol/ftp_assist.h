#pragma once

#include "../utils/session.h"
#include "../utils/hash.h"

extern session_t p_sess;
extern unsigned int num_of_clients;
extern hash *ip_to_clients;
extern hash *pid_to_ip;

void init_hash();
void remove_hash();
void check_permission();
void setup_signal_chld();
void print_conf();


void limit_num_clients(session_t sess);
void add_clients_to_hash(session_t sess, uint32_t ip);
void add_pid_ip_to_hash(pid_t pid, uint32_t ip);

