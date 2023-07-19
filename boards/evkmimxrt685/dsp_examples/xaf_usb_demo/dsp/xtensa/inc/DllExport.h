#pragma once
#ifdef _WIN32
#ifdef FLOWENGINELIB_EXPORTS
#define FLOW_API __declspec(dllexport)
#else
#define FLOW_API
#endif
#else
#define FLOW_API
#endif
