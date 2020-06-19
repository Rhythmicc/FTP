#pragma once

#include "../utils/session.h"

void privop_pasv_get_data_sock(session_t s);
void privop_pasv_active(session_t s);
void privop_pasv_listen(session_t s);
void privop_pasv_accept(session_t sess);

