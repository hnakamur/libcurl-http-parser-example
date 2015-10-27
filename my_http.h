#ifndef _MY_HTTP_H_
#define _MY_HTTP_H_

#include <curl/curl.h>
#include <http_parser.h>

typedef struct {
  char *field;
  size_t field_len;
  char *value;
  size_t value_len;
} my_http_header_line;

typedef struct {
  /* NOTE: parser must be the first field since we cast (http_parser *) to (my_http_client *)
   * in our code.
   */
  http_parser parser;
  http_parser_settings settings;
  int parser_initialized;
  CURL *curl;
  int last_was_value;
  size_t nlines;
  my_http_header_line *lines;
} my_http_client;

my_http_client *my_http_client_init();
void my_http_client_cleanup(my_http_client *client);

#endif
