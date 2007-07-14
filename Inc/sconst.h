#define REG_GS      0
#define REG_FS      REG_GS  + 4
#define REG_ES      REG_FS  + 4
#define REG_DS      REG_ES  + 4
#define REG_EDI     REG_DS  + 4
#define REG_ESI     REG_EDI + 4
#define REG_EBP     REG_ESI + 4
#define REG_XESP    REG_EBP + 4
#define REG_EBX     REG_XESP + 4
#define REG_EDX     REG_EBX + 4
#define REG_ECX     REG_EDX + 4
#define REG_EAX     REG_ECX + 4
#define DIR_RET     REG_EAX + 4
#define REG_EIP     DIR_RET + 4
#define REG_CS      REG_EIP + 4
#define REG_FLAGS   REG_CS  + 4
#define REG_ESP     REG_FLAGS + 4
#define REG_SS      REG_ESP + 4
#define STACKTOP    REG_SS  + 4
#define DIR_SEL_LDT STACKTOP
#define LDT         DIR_SEL_LDT + 4
