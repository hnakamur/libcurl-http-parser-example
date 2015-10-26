#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

static void trim(char **start, char **end) {
    while (*start < *end && isspace(**start)) {
      *start = *start + 1;
    }
    while (*start < *end && isspace(**end)) {
      *end = *end - 1;
    }
}

static void split_header_name_and_value(char *buffer, size_t buffer_len,
        char **name, size_t *name_len, char **value, size_t *value_len) {
  char *name_end, *value_end, *save_ptr;
  *name = strtok_r(buffer, ":", &save_ptr);
  if (*name == NULL) {
    return;
  }
  name_end = save_ptr;
  *value = strtok_r(NULL, "\r", &save_ptr);
  if (*value == NULL) {
    return;
  }
  value_end = save_ptr;

  trim(name, &name_end);
  trim(value, &value_end);
  *name_len = name_end - *name;
  *value_len = value_end - *value;
}

static void write_strn(char *s, size_t len) {
  fwrite(s, len, 1, stdout);
}

static void write_str(char *s) {
  write_strn(s, strlen(s));
}

size_t header_callback(char *ptr, size_t size, size_t nitems, void *userdata) {
  char *name, *value;
  size_t name_len, value_len;

  uint r;
  r = size * nitems;
  if (size > 0 && nitems > 0) {
    split_header_name_and_value(ptr, r, &name, &name_len, &value, &value_len);
    if (name && value) {
      write_str("header name=");
      write_strn(name, name_len);
      write_str(", value=");
      write_strn(value, value_len);
      write_str(".\n");
    }
  }
  return r;
}

size_t write_callback(char *ptr, size_t size, size_t nitems, void *userdata) {
  uint r;
  r = size * nitems;
/*
  if (size > 0 && nitems > 0) {
    fwrite(ptr, size, nitems, stdout);
  }
*/
  return r;
}

int main(void) {
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "http://www.sakura.ad.jp");
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
 
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
 
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
