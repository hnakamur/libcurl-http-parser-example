CFLAGS=-g -O0
LDFLAGS=-lcurl -lhttp_parser

hello: hello.c
	cc -o hello hello.c $(CFLAGS) $(LDFLAGS)

https_get: https_get.c
	cc -o https_get https_get.c $(CFLAGS) $(LDFLAGS)

clean:
	-rm hello https_get
