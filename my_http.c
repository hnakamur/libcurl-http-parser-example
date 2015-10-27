#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "my_http.h"

static int on_header_field(http_parser *parser, const char *at, size_t len) {
  my_http_client *client = (my_http_client *)parser;
  my_http_header_line *current_line;

  if (client->last_was_value) {
    client->nlines++;
    client->lines = realloc(client->lines, client->nlines * sizeof(my_http_header_line));
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
  my_http_header_line *current_line = &client->lines[client->nlines - 1];
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

static size_t header_callback(char *ptr, size_t size, size_t nitems, void *userdata) {
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

static void my_http_cleanup_header_lines(my_http_header_line *lines, size_t nlines) {
  int i;
  my_http_header_line *line;

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


my_http_client *my_http_client_init() {
  my_http_client *client = calloc(1, sizeof(my_http_client));
  if (!client) {
    return NULL;
  }
  client->curl = curl_easy_init();
  if (!client->curl) {
    free(client);
    return NULL;
  }
  curl_easy_setopt(client->curl, CURLOPT_HEADERDATA, client);
  curl_easy_setopt(client->curl, CURLOPT_HEADERFUNCTION, header_callback);
  return client;
}

void my_http_client_cleanup(my_http_client *client) {
  my_http_cleanup_header_lines(client->lines, client->nlines);
  curl_easy_cleanup(client->curl);
  free(client);
}
