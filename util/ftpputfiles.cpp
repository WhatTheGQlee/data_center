#include "_ftp.h"

struct st_arg {
  char host[31];  // 远程服务器的IP和端口。
  int mode;  // 传输模式，1-被动模式，2-主动模式，缺省采用被动模式。
  char username[31];     // 远程服务器ftp的用户名。
  char password[31];     // 远程服务器ftp的密码。
  char remotepath[301];  // 远程服务器存放文件的目录。
  char localpath[301];   // 本地文件存放的目录。
  char matchname[101];   // 待上传文件匹配的规则。
  int ptype;  // 上传后服务器文件的处理方式：1-什么也不做；2-删除；3-备份。
  char localpathbak[301];  // 上传后服务器文件的备份目录。
  char okfilename[301];    // 已上传成功文件名清单。
  int timeout;             // 进程心跳的超时时间。
  char pname[51];  // 进程名，建议用"ftpputfiles_后缀"的方式。
} starg;

// 文件信息的结构体。
struct st_fileinfo {
  char filename[301];  // 文件名。
  char mtime[21];      // 文件时间。
};

vector<struct st_fileinfo> vokfile;  // 存放已上传文件容器。
vector<struct st_fileinfo> vlistfile;  // 存放上传前列出服务器文件名的容器。
vector<struct st_fileinfo> vuploadedfile;  // 存放比较后已上传文件名的容器。
vector<struct st_fileinfo> vtodolistfile;  // 存放待上传文件名的容器。

// 加载okfilename文件中的内容到容器vlistfile1中。
bool LoadOKFile();

// 比较vlistfile2和vlistfile1，得到vlistfile3和vlistfile4。
bool CompVector();

// 把容器vlistfile3中的内容写入okfilename文件，覆盖之前的旧okfilename文件。
bool WriteToOKFile();

// 如果ptype==1，把上传成功的文件记录追加到okfilename文件中。
bool AppendToOKFile(struct st_fileinfo* stfileinfo);

CLogFile logfile;
CFile File;
Cftp ftp;
CPActive PActive;  // 进程心跳。

char strxmlbuffer[1024];

void EXIT(int sig);

void _help();

// 把xml解析到参数starg结构中。
bool _xmltoarg(char* strxmlbuffer);

// 上传文件功能的主函数。
bool _ftpputfiles();

// 把localpath目录下的文件加载到vlistfile容器中。
bool LoadLocalFile();

int main(int argc, char* argv[]) {
  if (argc != 3) {
    _help();
    return -1;
  }

  CloseIOAndSignal();
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  if (logfile.Open(argv[1], "a+") == false) {
    printf("打开日志文件失败（%s）。\n", argv[1]);
    return -1;
  }

  if (File.Open(argv[2], "r") == false) {
    printf("打开xml文件失败（%s）。\n", argv[2]);
    return -1;
  }

  File.Fread(strxmlbuffer, 1024);
  File.Close();

  // 解析xml，得到程序运行的参数。
  if (_xmltoarg(strxmlbuffer) == false)
    return -1;

  PActive.AddPInfo(starg.timeout, starg.pname);  // 把进程的心跳写入共享内存

  // 登录ftp服务器。
  if (ftp.login(starg.host, starg.username, starg.password, starg.mode) ==
      false) {
    logfile.Write("ftp.login(%s,%s,%s) failed.\n", starg.host, starg.username,
                  starg.password);
    return -1;
  }
  // logfile.Write("ftp.login ok.\n");

  _ftpputfiles();

  ftp.logout();

  return 0;
}

void _help() {
  printf("\n");
  printf("Using:/home/gqlee/project/bin/ftpputfiles logfilename xmlfile\n\n");

  printf(
      "Sample:/home/gqlee/project/bin/procctl 30 "
      "/home/gqlee/project/bin/ftpputfiles "
      "/home/gqlee/project/log/data_server.log "
      "/home/gqlee/project/util/ftpput.xml"
      "\n");

  printf("本程序是通用的功能模块，用于把远程ftp服务器的文件上传到本地目录。\n");
  printf("logfilename是本程序运行的日志文件。\n");
  printf("xmlbuffer为文件上传的参数，如下：\n");
  printf("<host>127.0.0.1:21</host> 远程服务器的IP和端口。\n");
  printf(
      "<mode>1</mode> 传输模式，1-被动模式，2-主动模式，缺省采用被动模式。\n");
  printf("<username>wucz</username> 远程服务器ftp的用户名。\n");
  printf("<password>wuczpwd</password> 远程服务器ftp的密码。\n");
  printf(
      "<remotepath>/tmp/idc/surfdata</remotepath> "
      "远程服务器存放文件的目录。\n");
  printf("<localpath>/idcdata/surfdata</localpath> 本地文件存放的目录。\n");
  printf(
      "<matchname>SURF_ZH*.XML,SURF_ZH*.CSV</matchname> 待上传文件匹配的规则。"
      "不匹配的文件不会被上传，本字段尽可能设置精确，不建议用*"
      "匹配全部的文件。\n");
  printf(
      "<ptype>1</ptype> "
      "文件上传成功后，远程服务器文件的处理方式：1-什么也不做；2-删除；3-"
      "备份，如果为3，还要指定备份的目录。\n");
  printf(
      "<remotepathbak>/tmp/idc/surfdatabak</remotepathbak> "
      "文件上传成功后，服务器文件的备份目录，此参数只有当ptype="
      "3时才有效。\n");
  printf(
      "<okfilename>/idcdata/ftplist/ftpputfiles_surfdata.xml</okfilename> "
      "已上传成功文件名清单，此参数只有当ptype=1时才有效。\n");
  printf(
      "<checkmtime>true</checkmtime> "
      "是否需要检查服务端文件的时间，true-需要，false-"
      "不需要，此参数只有当ptype=1时才有效，缺省为false。\n");
  printf(
      "<timeout>80</timeout> "
      "上传文件超时时间，单位：秒，视文件大小和网络带宽而定。\n");
  printf(
      "<pname>ftpputfiles_surfdata</pname> "
      "进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n\n");
}

void EXIT(int sig) {
  printf("程序退出，sig=%d\n\n", sig);
  exit(0);
}

bool _xmltoarg(char* strxmlbuffer) {
  memset(&starg, 0, sizeof(struct st_arg));

  GetXMLBuffer(strxmlbuffer, "root", strxmlbuffer);

  GetXMLBuffer(strxmlbuffer, "host", starg.host, 30);  // 远程服务器的IP和端口。
  if (strlen(starg.host) == 0) {
    logfile.Write("host is null.\n");
    return false;
  }

  GetXMLBuffer(
      strxmlbuffer, "mode",
      &starg.mode);  // 传输模式，1-被动模式，2-主动模式，缺省采用被动模式。
  if (starg.mode != 2)
    starg.mode = 1;

  GetXMLBuffer(strxmlbuffer, "username", starg.username,
               30);  // 远程服务器ftp的用户名。
  if (strlen(starg.username) == 0) {
    logfile.Write("username is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "password", starg.password,
               30);  // 远程服务器ftp的密码。
  if (strlen(starg.password) == 0) {
    logfile.Write("password is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "remotepath", starg.remotepath,
               300);  // 远程服务器存放文件的目录。
  if (strlen(starg.remotepath) == 0) {
    logfile.Write("remotepath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "localpath", starg.localpath,
               300);  // 本地文件存放的目录。
  if (strlen(starg.localpath) == 0) {
    logfile.Write("localpath is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "matchname", starg.matchname,
               100);  // 待上传文件匹配的规则。
  if (strlen(starg.matchname) == 0) {
    logfile.Write("matchname is null.\n");
    return false;
  }

  // 上传后服务器文件的处理方式：1-什么也不做；2-删除；3-备份。
  GetXMLBuffer(strxmlbuffer, "ptype", &starg.ptype);
  if ((starg.ptype != 1) && (starg.ptype != 2) && (starg.ptype != 3)) {
    logfile.Write("ptype is error.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "localpathbak", starg.localpathbak,
               300);  // 上传后服务器文件的备份目录。
  if ((starg.ptype == 3) && (strlen(starg.localpathbak) == 0)) {
    logfile.Write("localpathbak is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "okfilename", starg.okfilename,
               300);  // 已上传成功文件名清单。
  if ((starg.ptype == 1) && (strlen(starg.okfilename) == 0)) {
    logfile.Write("okfilename is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "timeout",
               &starg.timeout);  // 进程心跳的超时时间。
  if (starg.timeout == 0) {
    logfile.Write("timeout is null.\n");
    return false;
  }

  GetXMLBuffer(strxmlbuffer, "pname", starg.pname, 50);  // 进程名。
  if (strlen(starg.pname) == 0) {
    logfile.Write("pname is null.\n");
    return false;
  }

  return true;
}

bool _ftpputfiles() {
  // 把ftp.nlist()方法获取到的list文件加载到容器vfilelist中。
  if (LoadLocalFile() == false) {
    logfile.Write("LoadListFile() failed.\n");
    return false;
  }

  PActive.UptATime();  // 更新进程的心跳。

  if (starg.ptype == 1) {
    // 加载okfilename文件中的内容到容器vlistfile1中。
    LoadOKFile();
    // 比较vlistfile2和vlistfile1，得到vlistfile3和vlistfile4。
    CompVector();
    // 把容器vlistfile3中的内容写入okfilename文件，覆盖之前的旧okfilename文件。
    WriteToOKFile();
    // 把vlistfile4中的内容复制到vlistfile2中。
    vlistfile.clear();
    vlistfile.swap(vtodolistfile);
  }

  PActive.UptATime();  // 更新进程的心跳。

  char strremotefilename[301];
  char strlocalfilename[301];

  // 遍历容器vfilelist。
  for (size_t i = 0; i < vlistfile.size(); i++) {
    SNPRINTF(strremotefilename, sizeof(strremotefilename), 300, "%s/%s",
             starg.remotepath, vlistfile[i].filename);
    SNPRINTF(strlocalfilename, sizeof(strlocalfilename), 300, "%s/%s",
             starg.localpath, vlistfile[i].filename);

    // 调用ftp.put()方法从服务器上传文件。
    logfile.Write("put %s ...", strremotefilename);

    if (ftp.put(strlocalfilename, strremotefilename, true) == false) {
      logfile.WriteEx("failed.\n");
      break;
    }
    logfile.WriteEx("ok.\n");

    PActive.UptATime();  // 更新进程的心跳。

    // 如果ptype==1，把上传成功的文件记录追加到okfilename文件中。
    if (starg.ptype == 1) {
      AppendToOKFile(&vlistfile[i]);
    }

    // 删除文件
    if (starg.ptype == 2) {
      if (REMOVE(strlocalfilename) == false) {
        logfile.Write("ftp.ftpdelete(%s) failed.\n", strlocalfilename);
        return false;
      }
    }

    // 转存到备份目录
    if (starg.ptype == 3) {
      char strlocalfilenamebak[301];
      SNPRINTF(strlocalfilenamebak, sizeof(strlocalfilenamebak), 300, "%s/%s",
               starg.localpathbak, vlistfile[i].filename);
      if (RENAME(strlocalfilename, strlocalfilenamebak) == false) {
        logfile.Write("RENAME(%s,%s) failed.\n", strlocalfilename,
                      strlocalfilenamebak);
        return false;
      }
    }
  }

  return true;
}

// 把localpath目录下的文件加载到vlistfile容器中。
bool LoadLocalFile() {
  vlistfile.clear();
  CDir Dir;
  Dir.SetDateFMT("yyyymmddhh24miss");
  if (Dir.OpenDir(starg.localpath, starg.matchname) == false) {
    logfile.Write("Dir.OpenDir(%s) 失败。\n", starg.localpath);
    return false;
  }
  struct st_fileinfo stfileinfo;

  while (1) {
    memset(&stfileinfo, 0, sizeof(struct st_fileinfo));
    if (Dir.ReadDir() == false) {
      break;
    }
    strcpy(stfileinfo.filename, Dir.m_FileName);  // 文件名，不包括目录名。
    strcpy(stfileinfo.mtime, Dir.m_ModifyTime);  // 文件时间。
    vlistfile.push_back(stfileinfo);
  }

  return true;
}

// 加载okfilename文件中的内容到容器vlistfile1中。
bool LoadOKFile() {
  vokfile.clear();

  if (File.Open(starg.okfilename, "r") == false) {
    return true;
  }

  char strbuffer[501];
  struct st_fileinfo stfileinfo;

  while (1) {
    memset(&stfileinfo, 0, sizeof(struct st_fileinfo));
    if (File.Fgets(strbuffer, 300, true) == false) {
      break;
    }
    GetXMLBuffer(strbuffer, "filename", stfileinfo.filename);
    GetXMLBuffer(strbuffer, "mtime", stfileinfo.mtime);
    vokfile.push_back(stfileinfo);
  }

  File.Close();
  return true;
}

// 比较vlistfile2和vlistfile1，得到vlistfile3和vlistfile4。
bool CompVector() {
  vuploadedfile.clear();
  vtodolistfile.clear();

  size_t i = 0, j = 0;
  // 遍历vlistfile2。
  for (i = 0; i < vlistfile.size(); i++) {
    // 在vlistfile1中查找vlistfile2[ii]的记录。
    for (j = 0; j < vokfile.size(); j++) {
      // 如果找到了，把记录放入vlistfile3。
      if ((strcmp(vlistfile[i].filename, vokfile[j].filename) == 0) &&
          (strcmp(vlistfile[i].mtime, vokfile[j].mtime) == 0)) {
        vuploadedfile.push_back(vlistfile[i]);
        break;
      }
    }

    // 如果没有找到，把记录放入vlistfile4。
    if (j == vokfile.size()) {
      vtodolistfile.push_back(vlistfile[i]);
    }
  }

  return true;
}

// 把容器vlistfile3中的内容写入okfilename文件，覆盖之前的旧okfilename文件。
bool WriteToOKFile() {
  if (File.Open(starg.okfilename, "w") == false) {
    logfile.Write("File.Open(%s) failed.\n", starg.okfilename);
    return false;
  }
  for (size_t i = 0; i < vuploadedfile.size(); i++) {
    File.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n",
                 vuploadedfile[i].filename, vuploadedfile[i].mtime);
  }
  File.Close();
  return true;
}

// 如果ptype==1，把上传成功的文件记录追加到okfilename文件中。
bool AppendToOKFile(struct st_fileinfo* stfileinfo) {
  if (File.Open(starg.okfilename, "a") == false) {
    logfile.Write("File.Open(%s) failed.\n", starg.okfilename);
    return false;
  }

  File.Fprintf("<filename>%s</filename><mtime>%s</mtime>\n",
               stfileinfo->filename, stfileinfo->mtime);
  File.Close();
  return true;
}
