#pragma once

#include <spdlog/spdlog.h>

#define TRACE(...) spdlog::get("console")->trace(__VA_ARGS__)
#define DEBUGMSG(...) spdlog::get("console")->debug(__VA_ARGS__)
#define INFO(...) (spdlog::get("console"))->info(__VA_ARGS__)
#define WARN(...) spdlog::get("console")->warn(__VA_ARGS__)
#define ERROR(...) spdlog::get("console")->error(__VA_ARGS__)
#define CRITICAL(...) spdlog::get("console")->critical(__VA_ARGS__)
