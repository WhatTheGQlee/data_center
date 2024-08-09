# 开发框架头文件路径
PUBINCL = -I/home/gqlee/project/public

# 开发框架cpp文件名
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
DIR_BIN=../../bin

# 编译参数。
CFLAGS = -g 

# C++标准
CPPSTD = -std=c++11

all:crtsurfdata

crtsurfdata:crtsurfdata.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/crtsurfdata crtsurfdata.cpp $(PUBINCL) $(PUBCPP) -l_public -lm -lc

ftpclient:ftpclient.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/ftpclient ftpclient.cpp /home/gqlee/project/public/_ftp.cpp $(PUBINCL) $(PUBCPP) -l_public -lftp -lm -lc

obtcodetodb:obtcodetodb.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/obtcodetodb obtcodetodb.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) -l_public -lm -lc

obtmindtodb:obtmindtodb.cpp idcapp.h idcapp.cpp
	g++ $(CFLAGS) $(CPPSTD) -o $(DIR_BIN)/obtmindtodb obtmindtodb.cpp $(PUBINCL) $(PUBCPP) $(MYSQLINCL) $(MYSQLLIB) $(MYSQLLIBS) $(MYSQLCPP) idcapp.cpp -l_public -lm -lc

clean:
	rm $(DIR_BIN)/crtsurfdata