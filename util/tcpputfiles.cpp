/*
 * 程序名：tcpputfiles.cpp，采用tcp协议，实现文件上传的客户端。
 */
#include "_public.h"

// 程序运行的参数结构体。
struct st_arg {
  int clienttype;  // 客户端类型，1-上传文件；2-下载文件。
  char ip[31];     // 服务端的IP地址。
  int port;        // 服务端的端口。
  int ptype;  // 文件上传成功后本地文件的处理方式：1-删除文件；2-移动到备份目录。
  char clientpath[301];  // 本地文件存放的根目录。
  char clientpathbak[301];  // 文件上传后本地文件备份的根目录，ptype==2时有效。
  bool andchild;  // 是否上传clientpath目录下各级子目录的文件。
  char matchname[301];  // 待上传文件名的匹配规则，如"*.TXT,*.XML"。
  char srvpath[301];    // 服务端文件存放的根目录。
  int timetvl;     // 扫描本地目录文件的时间间隔，单位：秒。
  int timeout;     // 进程心跳的超时时间。
  char pname[51];  // 进程名，建议用"tcpputfiles_后缀"的方式。
} starg;

CLogFile logfile;

// 程序退出和信号2、15的处理函数。
void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构中。
bool _xmltoarg(char* strxmlbuffer);

CTcpClient TcpClient;
CFile File;
char strxmlbuffer[1024];

bool Login(const char* strxmlbuffer);  // 登录业务。

bool ActiveTest();  // 心跳。

char strrecvbuffer[1024];  // 发送报文的buffer。
char strsendbuffer[1024];  // 接收报文的buffer。

// 文件上传的主函数，执行一次文件上传的任务。
bool _tcpputfiles();
bool bcontinue =
    true;  // 如果调用_tcpputfiles发送了文件，bcontinue为true，初始化为true。

// 把文件的内容发送给对端。
bool SendFile(const int sockfd, const char* filename, const int filesize);

// 删除或者转存本地的文件。
bool AckMessage(const char* strrecvbuffer);

CPActive PActive;  // 进程心跳。

int main(int argc, char* argv[]) {
  if (argc != 3) {
    _help();
    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 打开日志文件。
  if (logfile.Open(argv[1], "a+") == false) {
    printf("打开日志文件失败（%s）。\n", argv[1]);
    return -1;
  }

  //  打开xml文件
  if (File.Open(argv[2], "r") == false) {
    printf("打开xml文件失败（%s）。\n", argv[2]);
    return -1;
  }

  File.Fread(strxmlbuffer, 1024);
  File.Close();

  // 解析xml，得到程序运行的参数。
  if (_xmltoarg(strxmlbuffer) == false)
    return -1;

  PActive.AddPInfo(starg.timeout,
                   starg.pname);  // 把进程的心跳信息写入共享内存。

  // 向服务端发起连接请求。
  if (TcpClient.ConnectToServer(starg.ip, starg.port) == false) {
    logfile.Write("TcpClient.ConnectToServer(%s,%d) failed.\n", starg.ip,
                  starg.port);
    EXIT(-1);
  }

  // 登录业务。
  if (Login(strxmlbuffer) == false) {
    logfile.Write("Login() failed.\n");
    EXIT(-1);
  }

  while (true) {
    // 调用文件上传的主函数，执行一次文件上传的任务。
    if (_tcpputfiles() == false) {
      logfile.Write("_tcpputfiles() failed.\n");
      EXIT(-1);
    }

    if (bcontinue == false) {
      sleep(starg.timetvl);

      if (ActiveTest() == false)
        break;
    }

    PActive.UptATime();
  }

  EXIT(0);
}

// 心跳。
bool ActiveTest() {
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "<activetest>ok</activetest>");
  // logfile.Write("发送：%s\n",strsendbuffer);
  if (TcpClient.Write(strsendbuffer) == false)
    return false;  // 向服务端发送请求报文。

  if (TcpClient.Read(strrecvbuffer, 20) == false)
    return false;  // 接收服务端的回应报文。
  // logfile.Write("接收：%s\n",strrecvbuffer);

  return true;
}

// 登录业务。
bool Login(const char* argv) {
  memset(strsendbuffer, 0, sizeof(strsendbuffer));
  memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

  SPRINTF(strsendbuffer, sizeof(strsendbuffer), "%s<clienttype>1</clienttype>",
          argv);
  logfile.Write("发送：%s\n", strsendbuffer);
  if (TcpClient.Write(strsendbuffer) == false)
    return false;  // 向服务端发送请求报文。

  if (TcpClient.Read(strrecvbuffer, 20) == false)
    return false;  // 接收服务端的回应报文。
  logfile.Write("接收：%s\n", strrecvbuffer);

  logfile.Write("登录(%s:%d)成功。\n", starg.ip, starg.port);

  return true;
}

void EXIT(int sig) {
  logfile.Write("程序退出，sig=%d\n\n", sig);

  exit(0);
}

void _help() {
  printf("\n");
  printf("Using:/project/tools1/bin/tcpputfiles logfilename xmlbuffer\n\n");

  printf(
      "Sample:/project/tools1/bin/procctl 20 /project/tools1/bin/tcpputfiles "
      "/log/idc/tcpputfiles_surfdata.log "
      "\"<ip>192.168.174.133</ip><port>5005</port><ptype>1</ptype><clientpath>/"
      "tmp/tcp/surfdata1</clientpath><andchild>true</"
      "andchild><matchname>*.XML,*.CSV,*.JSON</matchname><srvpath>/tmp/tcp/"
      "surfdata2</srvpath><timetvl>10</timetvl><timeout>50</"
      "timeout><pname>tcpputfiles_surfdata</pname>\"\n");
  printf(
      "       /project/tools1/bin/procctl 20 /project/tools1/bin/tcpputfiles "
      "/log/idc/tcpputfiles_surfdata.log "
      "\"<ip>192.168.174.132</ip><port>5005</port><ptype>2</ptype><clientpath>/"
      "tmp/tcp/surfdata1</clientpath><clientpathbak>/tmp/tcp/surfdata1bak</"
      "clientpathbak><andchild>true</andchild><matchname>*.XML,*.CSV,*.JSON</"
      "matchname><srvpath>/tmp/tcp/surfdata2</srvpath><timetvl>10</"
      "timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</"
      "pname>\"\n\n\n");

  printf("本程序是数据中心的公共功能模块，采用tcp协议把文件上传给服务端。\n");
  printf("logfilename   本程序运行的日志文件。\n");
  printf("xmlbuffer     本程序运行的参数，如下：\n");
  printf("ip            服务端的IP地址。\n");
  printf("port          服务端的端口。\n");
  printf(
      "ptype         "
      "文件上传成功后的处理方式：1-删除文件；2-移动到备份目录。\n");
  printf("clientpath    本地文件存放的根目录。\n");
  printf(
      "clientpathbak "
      "文件成功上传后，本地文件备份的根目录，当ptype==2时有效。\n");
  printf(
      "andchild      "
      "是否上传clientpath目录下各级子目录的文件，true-是；false-"
      "否，缺省为false。\n");
  printf("matchname     待上传文件名的匹配规则，如\"*.TXT,*.XML\"\n");
  printf("srvpath       服务端文件存放的根目录。\n");
  printf(
      "timetvl       扫描本地目录文件的时间间隔，单位：秒，取值在1-30之间。\n");
  printf(
      "timeout       "
      "本程序的超时时间，单位：秒，视文件大小和网络带宽而定，建议设置50以上。"
      "\n");
  printf(
      "pname         "
      "进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");
}

// 把xml解析到参数starg结构
bool _xmltoarg(char* strxmlbuffer) {
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer, "root", strxmlbuffer);

  GetXMLBuffer(strxmlbuffer, "ip", starg.ip);
  if (strlen(starg.ip) == 0) {
    logfile.Write("ip is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "port", &starg.port);
  if (starg.port == 0) {
    logfile.Write("port is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);
  if ((starg.ptype != 1) && (starg.ptype != 2)) {
    logfile.Write("ptype not in (1,2).\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "clientpath", starg.clientpath);
  if (strlen(starg.clientpath) == 0) {
    logfile.Write("clientpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "clientpathbak", starg.clientpathbak);
  if ((starg.ptype == 2) && (strlen(starg.clientpathbak) == 0)) {
    logfile.Write("clientpathbak is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "andchild", &starg.andchild);

  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname);
  if (strlen(starg.matchname) == 0) {
    logfile.Write("matchname is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "srvpath", starg.srvpath);
  if (strlen(starg.srvpath) == 0) {
    logfile.Write("srvpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "timetvl", &starg.timetvl);
  if (starg.timetvl == 0) {
    logfile.Write("timetvl is null.\n");
    return false;
  }

  // 扫描本地目录文件的时间间隔，单位：秒。
  // starg.timetvl没有必要超过30秒。
  if (starg.timetvl > 30)
    starg.timetvl = 30;

  // 进程心跳的超时时间，一定要大于starg.timetvl，没有必要小于50秒。
  GetXMLBuffer(strxmlbuffer, "timeout", &starg.timeout);
  if (starg.timeout == 0) {
    logfile.Write("timeout is null.\n");
    return false;
  }
  if (starg.timeout < 50)
    starg.timeout = 50;

  GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50);
  if (strlen(starg.pname) == 0) {
    logfile.Write("pname is null.\n");
    return false;
  }

  return true;
}

// 文件上传的主函数，执行一次文件上传的任务。
bool _tcpputfiles() {
  CDir Dir;

  // 调用OpenDir()打开starg.clientpath目录。
  if (Dir.OpenDir(starg.clientpath, starg.matchname, 10000, starg.andchild) ==
      false) {
    logfile.Write("Dir.OpenDir(%s) 失败。\n", starg.clientpath);
    return false;
  }

  int delayed = 0;  // 未收到对端确认报文的文件数量。
  int buflen = 0;   // 用于存放strrecvbuffer的长度。

  bcontinue = false;

  while (true) {
    memset(strsendbuffer, 0, sizeof(strsendbuffer));
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));

    // 遍历目录中的每个文件，调用ReadDir()获取一个文件名。
    if (Dir.ReadDir() == false)
      break;

    bcontinue = true;

    // 把文件名、修改时间、文件大小组成报文，发送给对端。
    SNPRINTF(strsendbuffer, sizeof(strsendbuffer), 1000,
             "<filename>%s</filename><mtime>%s</mtime><size>%d</size>",
             Dir.m_FullFileName, Dir.m_ModifyTime, Dir.m_FileSize);

    // logfile.Write("strsendbuffer=%s\n",strsendbuffer);
    if (TcpClient.Write(strsendbuffer) == false) {
      logfile.Write("TcpClient.Write() failed.\n");
      return false;
    }

    // 把文件的内容发送给对端。
    logfile.Write("send %s(%d) ...", Dir.m_FullFileName, Dir.m_FileSize);
    if (SendFile(TcpClient.m_connfd, Dir.m_FullFileName, Dir.m_FileSize) ==
        true) {
      logfile.WriteEx("ok.\n");
      delayed++;
    } else {
      logfile.WriteEx("failed.\n");
      TcpClient.Close();
      return false;
    }

    PActive.UptATime();

    // 接收对端的确认报文。
    while (delayed > 0) {
      memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
      if (TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen, -1) == false)
        break;
      // logfile.Write("strrecvbuffer=%s\n",strrecvbuffer);

      // 删除或者转存本地的文件。
      delayed--;
      AckMessage(strrecvbuffer);
    }
  }

  // 继续接收对端的确认报文。
  while (delayed > 0) {
    memset(strrecvbuffer, 0, sizeof(strrecvbuffer));
    if (TcpRead(TcpClient.m_connfd, strrecvbuffer, &buflen, 10) == false)
      break;
    // logfile.Write("strrecvbuffer=%s\n",strrecvbuffer);

    // 删除或者转存本地的文件。
    delayed--;
    AckMessage(strrecvbuffer);
  }

  return true;
}

// 把文件的内容发送给对端。
bool SendFile(const int sockfd, const char* filename, const int filesize) {
  int onread = 0;      // 每次调用fread时打算读取的字节数。
  int bytes = 0;       // 调用一次fread从文件中读取的字节数。
  char buffer[1000];   // 存放读取数据的buffer。
  int totalbytes = 0;  // 从文件中已读取的字节总数。
  FILE* fp = NULL;

  // 以"rb"的模式打开文件。
  if ((fp = fopen(filename, "rb")) == NULL)
    return false;

  while (true) {
    memset(buffer, 0, sizeof(buffer));

    // 计算本次应该读取的字节数，如果剩余的数据超过1000字节，就打算读1000字节。
    if (filesize - totalbytes > 1000)
      onread = 1000;
    else
      onread = filesize - totalbytes;

    // 从文件中读取数据。
    bytes = fread(buffer, 1, onread, fp);

    // 把读取到的数据发送给对端。
    if (bytes > 0) {
      if (Writen(sockfd, buffer, bytes) == false) {
        fclose(fp);
        return false;
      }
    }

    // 计算文件已读取的字节总数，如果文件已读完，跳出循环。
    totalbytes = totalbytes + bytes;

    if (totalbytes == filesize)
      break;
  }

  fclose(fp);

  return true;
}

// 删除或者转存本地的文件。
bool AckMessage(const char* strrecvbuffer) {
  char filename[301];
  char result[11];

  memset(filename, 0, sizeof(filename));
  memset(result, 0, sizeof(result));

  GetXMLBuffer(strrecvbuffer, "filename", filename, 300);
  GetXMLBuffer(strrecvbuffer, "result", result, 10);

  // 如果服务端接收文件不成功，直接返回。
  if (strcmp(result, "ok") != 0)
    return true;

  // ptype==1，删除文件。
  if (starg.ptype == 1) {
    if (REMOVE(filename) == false) {
      logfile.Write("REMOVE(%s) failed.\n", filename);
      return false;
    }
  }

  // ptype==2，移动到备份目录。
  if (starg.ptype == 2) {
    // 生成转存后的备份目录文件名。
    char bakfilename[301];
    STRCPY(bakfilename, sizeof(bakfilename), filename);
    UpdateStr(bakfilename, starg.clientpath, starg.clientpathbak, false);
    if (RENAME(filename, bakfilename) == false) {
      logfile.Write("RENAME(%s,%s) failed.\n", filename, bakfilename);
      return false;
    }
  }

  return true;
}
