#pragma once
#include <xtl.h>

const float log_red[3]   = { 1.0f, 0.0f, 0.0f };
const float log_green[3] = { 0.0f, 1.0f, 0.0f };
const float log_blue[3]  = { 0.0f, 0.0f, 1.0f };
const float log_white[3] = { 1.0f, 1.0f, 1.0f };

void Log(const char* string, const float* color = log_white, bool display_extra_as_int = false, void* extra_to_display = 0);
void Log_Render();


