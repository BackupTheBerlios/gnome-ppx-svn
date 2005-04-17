#include "pppd_util.h"
#include "pppd_parser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

/* helper function for skipping null values */
#define STRDUP(string) ((string) != NULL ? strdup (string) : NULL)
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

static int pppd_update_pwd (
			const char *username, 
			const char *password,
			const char *password_db,
			const char *tmp_password_db)
{
	/* reset umask so that no one can read password file */
	mode_t old_umask;
	FILE *db = NULL, *new_db = NULL;
	char *buff, *data;
	int line_len, buff_len;
	int wrote_username;
	pppd_parser *parser;
	
	old_umask = umask(0077);
	new_db = fopen (tmp_password_db, "w");
	umask(old_umask);
	if (!new_db) {
		return PPPD_ERROR_NO_TMP_PASS;
	}
	
	db = fopen (password_db, "r");
	if (!db) {
		fclose (new_db);
		/* delete temp file */
		unlink(tmp_password_db);
		return PPPD_ERROR_NO_PASS;
	}
	
	parser = pppd_parser_new("");
	buff = NULL;
	buff_len = 0;
	line_len = 0;
	wrote_username = password ? 1 : 0;
	while ((line_len = getline(&buff, &buff_len, db)) != -1) {
		/* let's parse a line */
		pppd_parser_set_buffer_str(parser, buff);
		
		data = pppd_parser_parse(parser);
		
		/* compare given username with provided */
		if (data && !strcmp(data, username)) {
			if (!wrote_username) {
				fprintf(new_db, "\"%s\" * \"%s\"\n", username, password);
				wrote_username = 1;
			}
		} else {
			fwrite(buff, line_len, 1, new_db);
		}
		
		if (data)
			free(data);
	}
	if (buff)
		free(buff);
	pppd_parser_free(parser);
	fclose(db);
	/* we are not updating but creating a fresh one */
	if (!wrote_username) {
		fprintf(new_db, "\"%s\" * \"%s\"\n", username, password);
	}
	fclose(new_db);
	
	if(unlink(password_db) || rename(tmp_password_db, password_db)) {
		/* bah, we couldn't create it... */
		unlink(tmp_password_db);
		return PPPD_ERROR_NO_REN;
	}
	return 0;
}

int pppd_update_pap (const char *username, const char *password)
{
	return pppd_update_pwd(username, password, "pap-secrets", ".pap-secrets.tmp");
}

int pppd_update_chap (const char *username, const char *password)
{
	return pppd_update_pwd (username, password, "chap-secrets", ".chap-secrets.tmp");
}

int pppd_get_link (const char *name, int *pid, char iface[IFNAMSIZ], time_t *start_time)
{
	char *filename, line[1024];
	int len;
	FILE *f;
	struct stat s;
	assert(name && iface && pid && start_time);
	
	#define LINK_BASE_LEN 17
	len = LINK_BASE_LEN + strlen(name) + 1;
	#undef LINK_BASE_LEN
	
	/* get the pid */
	filename = (char *) malloc (len);
	assert (filename);
	sprintf(filename, "/var/run/ppp-%s.pid", name);
	f = fopen(filename, "r");
	
	stat(filename, &s);
	*start_time = s.st_mtime;
	
	free(filename);
	
	if (!f) {
		return 0;
	}
	
	if (!fscanf(f, "%d\n", pid)) {
		fclose(f);
		return 0;
	}
	
	if (!fgets(line, sizeof(line), f)) {
		fclose(f);
		return 0;
	}
	fclose(f);
	
	len = strlen(line);
	if (line[len - 1] == '\n')
		line[len - 1] = '\0';
	
	/* make sure we don't buffer overflow */
	line[IFNAMSIZ - 1] = '\0';
	strcpy(iface, line);
	
	return 1;
}

int pppd_option_to_str (pppd_option *option, char* opts[3])
{
	
	char buff[10];
	int tmp;
	opts[0] = NULL;
	opts[1] = NULL;
	/* we'll strdup the name, so we must make sure it's not null */
	assert (option);
	assert (option->name);
	/* special boolean */
	if (option->type == PPPD_OPT_BOOL && !strcmp(option->name, "ipdefault")) {
		/* boolean value */
		if (!option->number) {
			opts[0] = STRDUP("noipdefault");
		}
	} else if (option->type == PPPD_OPT_BOOL) {
		if (!option->number) {
			tmp = strlen(option->name) + 2 + 1;
			opts[0] = malloc(tmp * sizeof(char));
			strcpy(opts[0], "no");
			strcat(opts[0], option->name);
		} else {
			opts[0] = STRDUP(option->name);
		}
		
	/* void value */
	} else if (option->type == PPPD_OPT_VOID) {
		/* void arguments */
		opts[0] = STRDUP(option->name);
	/* integer value */
	} else if (option->type == PPPD_OPT_INT) {
		/* get the next token and convert it to a string */
		snprintf(buff, sizeof(buff), "%d", option->number);
		opts[0] = STRDUP(option->name);
		opts[1] = STRDUP(buff);
	/* string value */
	} else if (option->type == PPPD_OPT_STR) {
		/* we are going to get the next token and
		 * embend it into two quotes
		 */
		/* since we're doing a strdup, we must assert value is null */
		assert(option->string);
		opts[0] = STRDUP(option->name);
		opts[1] = STRDUP(option->string);
	} else {
		/* unknown option */
		printf ("Unknown option %s, type %d\n", option->name, option->type);
		return 0;
	}
	return 1;
}

pppd_option *pppd_option_new (const char *name)
{
	pppd_option *ret;
	assert(name);
	ret = (pppd_option *) malloc(sizeof(pppd_option));
	/* just to make sure, but be carefull that
	 * asserts are removed without -g
	 */
	assert(ret);
	if (!ret) {
		return NULL;
	}
	ret->name = STRDUP(name);
	/* reset dont start it with an int value */
	ret->type = PPPD_OPT_VOID;
	return ret;
}

static void pppd_option_free_string (pppd_option *self)
{
	assert (self);
	if (self->type == PPPD_OPT_STR && self->string)
		free(self->string);
}

void pppd_option_set_string (pppd_option *self, char *val)
{
	assert (self);
	pppd_option_free_string (self);
	self->type = PPPD_OPT_STR;
	self->string = val;
}

void pppd_option_set_const_string (pppd_option *self, const char *val)
{
	/* don't strdup NULL values */
	char *v = STRDUP(val);
	assert (self);
	pppd_option_set_string(self, v);
}

void pppd_option_set_int (pppd_option *self, int val)
{
	assert (self);
	pppd_option_free_string (self);
	self->type = PPPD_OPT_INT;
	self->number = val;
}

void pppd_option_set_bool (pppd_option *self, int val)
{
	assert (self);
	pppd_option_free_string (self);
	self->type = PPPD_OPT_BOOL;
	self->number = val;
}

void pppd_option_free (pppd_option *self)
{
	assert (self);
	pppd_option_free_string(self);
	free(self);
}

void pppd_option_set_name (pppd_option *self, const char *name)
{
	assert(name);
	free (self->name);
	self->name = STRDUP(name);
}

void pppd_option_set_void (pppd_option *self, const char *value)
{
	assert (self);
	pppd_option_set_name(self, value);
}
