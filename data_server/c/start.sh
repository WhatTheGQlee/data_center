#!/bin/bash
####################################################################
# 启动数据中心后台服务程序的脚本。
####################################################################

# 检查服务程序是否超时，配置在/etc/rc.local中由root用户执行。
/home/gqlee/project/bin/procctl 30 /home/gqlee/project/bin/checkproc

# 压缩数据中心后台服务程序的备份日志。
/home/gqlee/project/bin/procctl 300 /home/gqlee/project/bin/gzipfiles /home/gqlee/project/log "*.log.20*" 0.04

# 生成用于测试的全国气象站点观测的分钟数据。
/home/gqlee/project/bin/procctl  60 /home/gqlee/project/bin/crtsurfdata \
/home/gqlee/project/data_server/ini/stcode.ini \
/home/gqlee/project/tmp/surfdata \
/home/gqlee/project/log/data_server.log xml

# 清理原始的全国气象站点观测的分钟数据目录/tmp/surfdata中的历史数据文件。
/home/gqlee/project/bin/procctl 300 /home/gqlee/project/bin/deletefiles /home/gqlee/project/tmp/surfdata "*" 0.04

# # 文件传输的服务端程序。
# /home/gqlee/project/bin/procctl 10 /home/gqlee/project/bin/fileserver 22000 /home/gqlee/project/log/file_server.log

# # 把目录/tmp/ftpputest中的文件上传到/tmp/tcpputest目录中。
# /home/gqlee/project/bin/procctl 10 /home/gqlee/project/bin/tcpputfiles /home/gqlee/project/log/file_client.log /home/gqlee/project/util/tcpputfiles.xml

# # 把目录/tmp/tcpputest中的文件下载到/tmp/tcpgetest目录中。
# /home/gqlee/project/bin/procctl 10 /home/gqlee/project/bin/tcpgetfiles /home/gqlee/project/log/file_client.log /home/gqlee/project/util/tcpgetfiles.xml

# # 清理采集的全国气象站点观测的分钟数据目录/tmp/tcpgetest中的历史数据文件。
# /home/gqlee/project/bin/procctl 300 /home/gqlee/project/bin/deletefiles /home/gqlee/project/tmp/surfdata "*" 0.02

# # 把全国站点参数数据保存到数据库表中，如果站点不存在则插入，站点已存在则更新。
# /home/gqlee/project/bin/procctl 120 /home/gqlee/project/bin/obtcodetodb /home/gqlee/project/idc/ini/stcode.ini "127.0.0.1,root,123456,data_server,3306" utf8 /home/gqlee/project/log/obtcodetodb.log

# # 把全国站点分钟观测数据保存到数据库的T_ZHOBTMIND表中，数据只插入，不更新。
# /home/gqlee/project/bin/procctl 120 /home/gqlee/project/bin/obtcodetodb /home/gqlee/project/idc/ini/stcode.ini "127.0.0.1,root,123456,data_server,3306" utf8 /home/gqlee/project/log/obtcodetodb.log
