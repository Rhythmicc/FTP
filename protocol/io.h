#pragma once

#include "../utils/session.h"

void download_file(session_t sess);
void upload_file(session_t sess, int is_appe);
void trans_list(session_t sess, int list);

