#!/bin/bash
####################################################################
# 停止数据中心后台服务程序的脚本。
####################################################################

killall -9 procctl
killall gzipfiles crtsurfdata deletefiles 

sleep 3

killall -9 gzipfiles crtsurfdata deletefiles

# docker run -d -v /var/ftp:/home/vsftpd \
# -p 20:20 -p 21:21 -p  21100-21110:21100-21110 \
# -e FTP_USER=test -e FTP_PASS=123456 \
# -e PASV_ADDRESS=47.120.62.211 \
# -e PASV_MIN_PORT=21100 -e PASV_MAX_PORT=21110 \
# --name vsftpd --restart=always fauria/vsftpd