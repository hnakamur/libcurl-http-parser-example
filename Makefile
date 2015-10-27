CFLAGS=-g -O0
LDFLAGS=-lcurl -lhttp_parser

http_get_example: http_get_example.c
	cc -o http_get_example http_get_example.c my_http.c $(CFLAGS) $(LDFLAGS)

clean:
	-rm http_get_example https_get
