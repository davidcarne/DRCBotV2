

enum debug_level_t {

	DEBUG_NONE = 0,
	DEBUG_ERROR = 1,
	DEBUG_WARN = 2,
	DEBUG_MSG = 3,
	DEBUG_VERBOSE = 4

	};

extern "C" {

extern enum debug_level_t debug_level;
};

#define _DBG_PF(lvl, ...) if (debug_level >= lvl) { printf(__VA_ARGS__); printf("\n"); }
#define DBG_ERR_PF(...) _DBG_PF(DEBUG_ERROR, __VA_ARGS__)
#define DBG_WARN_PF(...) _DBG_PF(DEBUG_WARN, __VA_ARGS__)
#define DBG_MSG_PF(...) _DBG_PF(DEBUG_MSG, __VA_ARGS__)
#define DBG_VERBOSE_PF(...) _DBG_PF(DEBUG_VERBOSE, __VA_ARGS__)

#define strndup(str,len) FIX_strndup(str,len)

char * FIX_strndup(char * str, size_t len);
