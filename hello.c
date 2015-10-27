#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <http_parser.h>

typedef struct {
  char *field;
  size_t field_len;
  char *value;
  size_t value_len;
} header_line;

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
  header_line *lines;
} my_http_client;

static void write_strn(const char *s, size_t len) {
  fwrite(s, len, 1, stdout);
}

static void write_str(const char *s) {
  write_strn(s, strlen(s));
}

static void print_header_lines(header_line *lines, size_t nlines) {
  int i;
  header_line *line;

  for (i = 0; i < nlines; i++) {
    line = &lines[i];
    write_str("field:");
    write_strn(line->field, line->field_len);
    write_str("\tvalue:");
    write_strn(line->value, line->value_len);
    write_str("\n");
  }
}

static void cleanup_header_lines(header_line *lines, size_t nlines) {
  int i;
  header_line *line;

  for (i = 0; i < nlines; i++) {
    line = &lines[i];
    if (line) {
      if (line->field) {
        free(line->field);
      }
      if (line->value) {
        free(line->value);
      }
    }
  }
}

static int on_header_field(http_parser *parser, const char *at, size_t len) {
  my_http_client *client = (my_http_client *)parser;
  header_line *current_line;

  if (client->last_was_value) {
    client->nlines++;
    client->lines = realloc(client->lines, client->nlines * sizeof(header_line));
    assert(client->lines != NULL);
    current_line = &client->lines[client->nlines - 1];
    
    current_line->value = NULL;
    current_line->value_len = 0;

    current_line->field_len = len;
    current_line->field = malloc(len+1);
    assert(current_line->field != NULL);
    strncpy(current_line->field, at, len);

  } else {
    current_line = &client->lines[client->nlines - 1];
    assert(current_line->value == NULL);
    assert(current_line->value_len == 0);

    current_line->field_len += len;
    current_line->field = realloc(current_line->field,
        current_line->field_len+1);
    assert(current_line->field != NULL);
    strncat(current_line->field, at, len);
  }

  current_line->field[current_line->field_len] = '\0';
  client->last_was_value = 0;
  return 0;
}

static int on_header_value(http_parser *parser, const char *at, size_t len) {
  my_http_client *client = (my_http_client *)parser;
  header_line *current_line = &client->lines[client->nlines - 1];
  if (!client->last_was_value) {
    current_line->value_len = len;
    current_line->value = malloc(len+1);
    assert(current_line->value != NULL);
    strncpy(current_line->value, at, len);
  } else {
    current_line->value_len += len;
    current_line->value = realloc(current_line->value,
        current_line->value_len+1);
    assert(current_line->value != NULL);
    strncat(current_line->value, at, len);
  }

  current_line->value[current_line->value_len] = '\0';
  client->last_was_value = 1;
  return 0;
}

size_t header_callback(char *ptr, size_t size, size_t nitems, void *userdata) {
  size_t r, nparsed;
  my_http_client *client;
  CURLcode res;
  curl_socket_t socket;

  client = (my_http_client *)userdata;
  if (!client->parser_initialized) {
    client->settings.on_header_field = on_header_field;
    client->settings.on_header_value = on_header_value;
    http_parser_init(&client->parser, HTTP_RESPONSE);
    client->parser_initialized = 1;
    client->last_was_value = 1;
  }

  r = size * nitems;
  if (r > 0) {
    r = http_parser_execute(&client->parser, &client->settings, ptr, r);
  }
  return r;
}

size_t write_callback(char *ptr, size_t size, size_t nitems, void *userdata) {
  uint r;
  r = size * nitems;
  if (size > 0 && nitems > 0) {
    write_strn(ptr, r);
  }
  return r;
}

int main(int argc, char **argv) {
  CURL *curl;
  CURLcode res;
  my_http_client client;
  char *url;
 
  if (argc != 2) {
    fprintf(stderr, "Usage: %s url\n", argv[0]);
    return 1;
  }

  url = argv[1];

  curl_global_init(CURL_GLOBAL_ALL);
 
  memset(&client, 0, sizeof(my_http_client));
  curl = curl_easy_init();
  if (curl) {
    client.curl = curl;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &client);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
 
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
 
    write_str("\n");
    print_header_lines(client.lines, client.nlines);
    cleanup_header_lines(client.lines, client.nlines);
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
