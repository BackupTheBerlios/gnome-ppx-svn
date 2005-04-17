#include "pppd_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
enum {
	PARSE_BLANK,
	PARSE_COMMENT,
	PARSE_WORD,
	PARSE_ESC,
	PARSE_DONE,
};

static char *inttostr(int x)
{
	char buff[20];
	snprintf(buff, sizeof(buff), "%d", x);
	return strdup(buff);
}

struct _PString {
	char *str;
	int len;
	int allocated_len;
};

typedef struct _PString PString;

static PString *p_string_new(const char *str)
{
	PString *ret = malloc(sizeof(PString));
	ret->str = (char *) strdup(str);
	ret->len = strlen(str);
	ret->allocated_len = ret->len;
	return ret;
}

static void p_string_inc_size (PString *str, int size)
{
	str->str = (char *)realloc(str->str, str->allocated_len + size);
	str->allocated_len += size;
}

static void p_string_append_c (PString *str, int c)
{
	/* we must hold the NULL byte */
	if (str->len + 1 == str->allocated_len) {
		p_string_inc_size(str, 100);
	}
	str->str[str->len] = c;
	str->len++;
	/* NULL terminated string */
	str->str[str->len] = '\0';
}

static void p_string_free (PString *str)
{
	assert(str->str);
	free(str->str);
	str->str = NULL;
	free(str);
	str = NULL;
}

static void p_string_append (PString *self, const char *str)
{
	int str_len = strlen(str);
	if (self->len + str_len >= self->allocated_len) {
		p_string_inc_size(self, str_len + 100);
	}
	memcpy (self->str + self->len, str, str_len);
	self->len += str_len;
	/* terminate it with a NULL char */
	self->str[self->len] = '\0';
}

struct _pppd_parser {
	/* input buffer */
	PString *input;
	/* index which points to input buffer position */
	int index;
	/* output buffer */
	PString *output;
	/* if the parser is valid */
	char done;
};

#define PPPD_IS_BLANK(c) (c == ' ' || c == '\t' || c == '\n')

/* get the next token from buffer */
static int pppd_next_token (pppd_parser *self)
{
	if (self->index == self->input->len) {
		return EOF;
	} else {
		int ret = self->input->str[self->index];
		/* increase the index */
		self->index++;
		return ret;
	}
}

static void pppd_parser_push (pppd_parser *self)
{
	self->index--;
}

static int pppd_append_octal (pppd_parser *self)
{
	PString *str = p_string_new("");
	char *tmp;
	int token;
	for (token = pppd_next_token(self); token != EOF && token >= '0' && token <= '7'; token = pppd_next_token(self)) {
		p_string_append_c(str, token);
	}
	if (str->len) {
		tmp = inttostr((int) strtol(str->str, NULL, 16));
		p_string_append(self->output, tmp);
		free(tmp);
		pppd_parser_push(self);
	}
	p_string_free(str);
	return str->len > 0;
}

static int pppd_append_hex(pppd_parser *self)
{
	PString *str = p_string_new("");
	char *tmp;
	int token;
	for (token = pppd_next_token(self); token != EOF && isxdigit(token); token = pppd_next_token(self)) {
		p_string_append_c(str, token);
	}
	if (str->len) {
		tmp = inttostr((int) strtol(str->str, NULL, 16));
		p_string_append(self->output, tmp);
		free(tmp);
		pppd_parser_push(self);
	}
	p_string_free(str);
	return str->len > 0;
}

char * pppd_parser_parse (pppd_parser *self)
{
	int state;
	int token;
	if (self->done)
		return NULL;
	/* initial state */
	state = PARSE_BLANK;
	/* reset the buffer */
	self->output->len = 0;
	while (state != PARSE_DONE) {
		if (state == PARSE_BLANK) {
			for (token = pppd_next_token(self); token != EOF && PPPD_IS_BLANK(token); token = pppd_next_token(self))
				/* feed on this tokens */;
			if (token == EOF) {
				self->done = 1;
				return NULL;
			}
			else if (token == '"')
				state = PARSE_ESC;
			else if (token == '#')
				state = PARSE_COMMENT;
			else {
				/* we found a word, let's paste append the first char in the
				 * output buffer
				 */
				p_string_append_c(self->output, token);
				state = PARSE_WORD;
			}

		} else if (state == PARSE_COMMENT) {
			for (token = pppd_next_token(self); token != EOF && token != '\n'; token = pppd_next_token(self))
				/* feed on comments */;
			state = PARSE_BLANK;
		} else if (state == PARSE_WORD) {
			for (token = pppd_next_token(self); token != EOF && !PPPD_IS_BLANK(token) && token != '"'; token = pppd_next_token(self)) {
				p_string_append_c(self->output, token);
			}
			if (token == '"') {
				self->done = 1;
				return NULL;
			}
			state = PARSE_DONE;
		} else if (state == PARSE_ESC) {
			for (token = pppd_next_token(self); token != EOF && token != '"'; token = pppd_next_token(self)) {
				/* now we have a new automata */
				if (token == '\\') {
					token = pppd_next_token(self);
					if (token == EOF) {
						self->done = 1;
						return NULL;
					} 
					else if (token == 'a')
						p_string_append_c(self->output, '\a');
					else if (token == 'b')
						p_string_append_c(self->output, '\b');
					else if (token == 'f')
						p_string_append_c(self->output, '\f');
					else if (token == 'n')
						p_string_append_c(self->output, '\n');
					else if (token == 'r')
						p_string_append_c(self->output, '\r');
					else if (token == 's')
						p_string_append_c(self->output, ' ');
					else if (token == 't')
						p_string_append_c(self->output, '\t');
					else if (token == '0') {
						if (!pppd_append_octal(self)) {
							self->done = 1;
							return NULL;
						}
						
					} else if (token == 'x') {
						if (!pppd_append_hex(self)) {
							self->done = 1;
							return NULL;
						}
					}
				} else {
					p_string_append_c(self->output, token);
				}
			}
			if (token == EOF) {
				/* error */
				self->done = 1;
				return NULL;
			}
			state = PARSE_DONE;
		}
	}
	/* returns a copy of the output buffer */
	return strdup(self->output->str);
}

void pppd_parser_free (pppd_parser *self)
{
	p_string_free(self->input);
	self->input = NULL;
	p_string_free(self->output);
	self->output = NULL;
	free(self);
	self = NULL;
}

pppd_parser *pppd_parser_new (const char *buff)
{
	pppd_parser *self;
	self = malloc (sizeof(pppd_parser));
	assert(self);
	self->input = p_string_new(buff);
	self->output = p_string_new("");
	self->index = 0;
	self->done = 0;
	return self;
}

void pppd_parser_set_buffer_str (pppd_parser *self, const char *buffer)
{
	self->index = 0;
	self->input->len = 0;
	self->done = 0;
	p_string_append(self->input, buffer);
}
