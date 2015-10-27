#include <string.h>
#include "my_http.h"

static void write_strn(const char *s, size_t len) {
  fwrite(s, len, 1, stdout);
}

static void write_str(const char *s) {
  write_strn(s, strlen(s));
}

static void print_header_lines(my_http_header_line *lines, size_t nlines) {
  int i;
  my_http_header_line *line;

  for (i = 0; i < nlines; i++) {
    line = &lines[i];
    write_str("field:");
    write_strn(line->field, line->field_len);
    write_str("\tvalue:");
    write_strn(line->value, line->value_len);
    write_str("\n");
  }
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
  CURLcode res;
  my_http_client *client;
  char *url;
 
  if (argc != 2) {
    fprintf(stderr, "Usage: %s url\n", argv[0]);
    return 1;
  }

  url = argv[1];

  curl_global_init(CURL_GLOBAL_ALL);
 
  client = my_http_client_init();
  if (client) {
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
 
    res = curl_easy_perform(client->curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
 
    write_str("\n");
    print_header_lines(client->lines, client->nlines);

    my_http_client_cleanup(client);
  }
  curl_global_cleanup();
  return 0;
}
