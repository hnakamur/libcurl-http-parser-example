https_get: https_get.c
	cc -o https_get https_get.c -lcurl

hello: hello.c
	cc -o hello hello.c -lcurl
