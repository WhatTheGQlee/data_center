#include "_public.h"

void EXIT(int sig);

int main(int argc, char *argv[]) {
  // 程序的帮助。
  if (argc != 4) {
    printf("\n");
    printf(
        "Using:/home/gqlee/project/bin/deletefiles pathname matchstr "
        "timeout\n\n");

    printf(
        "Example:/home/gqlee/project/bin/deletefiles /home/gqlee/project/log "
        "\"*.log.20*\" "
        "0.02\n");
    printf(
        "/home/gqlee/project/bin/deletefiles /home/gqlee/project/tmp/surfdata "
        "\"*.xml,*.json\" 0.01\n");
    printf(
        "/home/gqlee/project/bin/procctl 300 "
        "/home/gqlee/project/bin/deletefiles "
        "/home/gqlee/project/log \"*.log.20*\" 0.02\n");
    printf(
        "/home/gqlee/project/bin/procctl 300 "
        "/home/gqlee/project/bin/deletefiles "
        "/home/gqlee/project/tmp/surfdata \"*.csv\" 0.01\n\n");

    printf("这是一个工具程序，用于删除历史的数据文件或日志文件。\n");
    printf(
        "本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部删除"
        "，timeout可以是小数。\n");
    printf("本程序不写日志文件，也不会在控制台输出任何信息。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  // CloseIOAndSignal(true);
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 获取文件超时的时间点。
  char strTimeOut[21];
  LocalTime(strTimeOut, "yyyy-mm-dd hh24:mi:ss",
            0 - (int)(atof(argv[3]) * 24 * 60 * 60));

  CDir Dir;
  if (Dir.OpenDir(argv[1], argv[2], 10000, true) == false) {
    printf("Dir.OpenDir(%s) failed.\n", argv[1]);
    return -1;
  }

  while (1) {
    if (Dir.ReadDir() == false) break;

    if (strcmp(Dir.m_ModifyTime, strTimeOut) < 0) {
      if (REMOVE(Dir.m_FullFileName) == 0) {
        printf("REMOVE %s ok.\n", Dir.m_FullFileName);
      } else {
        printf("REMOVE %s failed.\n", Dir.m_FullFileName);
      }
    }
  }

  return 0;
}

void EXIT(int sig) {
  printf("程序退出，sig=%d\n\n", sig);
  exit(0);
}
