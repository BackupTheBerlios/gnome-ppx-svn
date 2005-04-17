#ifndef __PPPD_UTIL_H__
#define __PPPD_UTIL_H__

#include <time.h>
#include <net/if.h>

enum {
	PPPD_ERROR_OK,
	PPPD_ERROR_NO_TMP_PASS,
	PPPD_ERROR_NO_PASS,
	PPPD_ERROR_NO_REN,
};

typedef enum {
	PPPD_OPT_VOID,
	PPPD_OPT_BOOL,
	PPPD_OPT_INT,
	PPPD_OPT_STR
} pppd_option_type;

typedef struct _pppd_option pppd_option;
	
struct _pppd_option {
	char *name;
	pppd_option_type type;
	union {
		int number;
		char *string;
	};
};

#define PPPD_OPT_SET_STR(opt,_name,s)  {(opt).name = (_name); (opt).type = PPPD_OPT_STR; (opt).string = (s);}
#define PPPD_OPT_SET_INT(opt,_name,n)  {(opt).name = (_name); (opt).type = PPPD_OPT_INT; (opt).number = (n);}
#define PPPD_OPT_SET_BOOL(opt,_name,b) {(opt).name = (_name); (opt).type = PPPD_OPT_BOOL; (opt).number = (b);}
/* if 'password' is NULL the old password will be removed from
 * password file (if existing).
 */
int pppd_update_pap (const char *user, const char *passwd);
int pppd_update_chap (const char *user, const char *passwd);

/*
 * from a link name returns the pppd PID, interface name and
 * connection starting time.
 */
int pppd_get_link (const char *name, int *pid, char iface[IFNAMSIZ], time_t *start_time);

/*
 * Converts an option to string value, this can result in a max of two strings
 * It will be null terminated so if there is one entry then
 * ret[1] will have NULL.
 * All values are alloced and must be released with free().
 * Returns 0 when the type of option is unknown.
 *
 * Boolean:
 * adds a 'no' prefix in case of FALSE, except ipdefault, which does not
 * have a TRUE representation (it is when it's omited).
 *
 * Void:
 * simply translates the element.
 */
int pppd_option_to_str (pppd_option *option, char* ret[2]);
/*
 * pppd_option's created this way must be released using
 * pppd_option_free. It is initialized with VOID type.
 */
pppd_option *pppd_option_new(const char *name);
/* string must be previously created with malloc, or strdup,
 * because it will be released with free()
 */
void pppd_option_set_name (pppd_option *self, const char *val);
void pppd_option_set_string (pppd_option *self, char *val);
void pppd_option_set_const_string (pppd_option *self, const char *val);
void pppd_option_set_int (pppd_option *self, int val);
void pppd_option_set_bool (pppd_option *self, int val);
void pppd_option_set_void (pppd_option *self, const char *value);
/*
 * Releases a ppp_option created with pppd_option_new()
 */
void pppd_option_free (pppd_option *self);
#endif /* __PPPD_UTIL_H__ */
