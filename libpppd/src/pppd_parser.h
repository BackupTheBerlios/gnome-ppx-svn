#ifndef __PPPD_PARSER_H__
#define __PPPD_PARSER_H__
typedef struct _pppd_parser pppd_parser;
pppd_parser *pppd_parser_new (const char *buff);
void pppd_parser_set_buffer_str (pppd_parser *self, const char *buffer);
char * pppd_parser_parse(pppd_parser *self);
void pppd_parser_free (pppd_parser *self);
#endif
