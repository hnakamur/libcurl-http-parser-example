#!/bin/bash
sudo yum install -y gcc make gdb libcurl-devel

curl -sL -o http-parser-master.tar.gz https://github.com/nodejs/http-parser/archive/master.tar.gz
tar xf http-parser-master.tar.gz
cd http-parser-master
make
sudo make install
sudo sh -c "echo /usr/local/lib > /etc/ld.so.conf.d/http_parser.x86_64.conf"
sudo ldconfig
