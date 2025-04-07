#ifndef UI_H_
#define UI_H_
#include "types.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void cpp_init(void);
EXTERNC void cpp_imgui_render(void);
EXTERNC void cpp_end(void);
EXTERNC const char* opcode_to_string(uint8_t opcode);

#endif
