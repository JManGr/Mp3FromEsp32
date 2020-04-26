#ifdef _TOUTCH_KEY
#define_TOUTCH_KEY
#endif
typedef void (*RunCommand_t)(char c);

void setupTouch(RunCommand_t runCmd);