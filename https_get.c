#include <stdio.h>
#include <curl/curl.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  uint r;
  r = size * nmemb;
  if (size > 0 && nmemb > 0) {
    fwrite(ptr, size, nmemb, stdout);
  }
  return r;
}

int main(void) {
  CURL *curl;
  CURLcode res;
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://secure.sakura.ad.jp/cloud/");
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
 
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
 
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
