# 开发框架头文件路径 export LD_LIBRARY_PATH=/home/gqlee/project/public
PUBINCL = -I/home/gqlee/project/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
PUBCPP = -L/home/gqlee/project/public

# mysql头文件存放路径
MYSQLINCL = -I/usr/include/mysql  -I/home/gqlee/project/public/db/mysql

# mysql库文件保存路径 export LD_LIBRARY_PATH=/home/gqlee/project/public
MYSQLLIB = -L/usr/lib64/mysql

# mysql链接库
MYSQLLIBS = -lmysqlclient

# 开发框架mysql的cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试。
MYSQLCPP = /home/gqlee/project/public/db/mysql/_mysql.cpp

#可执行文件位置
DIR_BIN = ../bin

# 编译参数
CFLAGS = -g 

# C++标准
CPPSTD = -std=c++11

all: procctl checkproc gzipfiles deletefiles ftpgetfiles ftpputfiles

procctl:procctl.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/procctl procctl.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

checkproc:checkproc.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/checkproc checkproc.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

gzipfiles:gzipfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/gzipfiles gzipfiles.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

deletefiles:gzipfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/deletefiles deletefiles.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

ftpgetfiles:ftpgetfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/ftpgetfiles ftpgetfiles.cpp /home/gqlee/project/public/_ftp.cpp $(PUBINCL) $(PUBCPP) -l_public -lftp -lm -lc

ftpputfiles:ftpputfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/ftpputfiles ftpputfiles.cpp /home/gqlee/project/public/_ftp.cpp $(PUBINCL) $(PUBCPP) -l_public -lftp -lm -lc

tcpgetfiles:tcpgetfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/tcpgetfiles tcpgetfiles.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

tcpputfiles:tcpputfiles.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/tcpputfiles tcpputfiles.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

fileserver:fileserver.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/fileserver fileserver.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

execsql:execsql.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/execsql execsql.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -l_public -lm -lc

dminingmysql:dminingmysql.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/dminingmysql dminingmysql.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -l_public -lm -lc

xmltodb:xmltodb.cpp _tools.h _tools.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/xmltodb xmltodb.cpp _tools.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -l_public -lm -lc

clean:
	rm -f $(DIR_BIN)/*