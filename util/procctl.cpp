#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Using:./procctl timetvl program argv ...\n");
    printf(
        "Example:./procctl 60 /home/gqlee/project/bin/crtsurfdata "
        "/home/gqlee/project/data_server/ini/stcode.ini "
        "/home/gqlee/project/tmp/surfdata "
        "/home/gqlee/project/log/data_server.log "
        "csv\n\n");

    printf("本程序是服务程序的调度程序, 周期性启动服务程序或shell脚本.\n");
    printf(
        "timetvl "
        "运行周期 单位:秒 被调度的程序运行结束后,"
        "在timetvl秒后会被procctl重新启动.\n");
    printf("program 被调度的程序名,必须使用全路径.\n");
    printf("argvs   被调度的程序的参数.\n");
    printf("注意,本程序不会被kill杀死,但可以用kill -9强行杀死.\n\n\n");

    return -1;
  }

  // 关闭信号和IO，本程序不希望被打扰。
  for (size_t i = 0; i < 64; i++) {
    signal(i, SIG_IGN);
    close(i);
  }

  // 生成子进程，父进程退出，让程序运行在后台，由系统1号进程托管。
  if (fork() != 0) {
    exit(0);
  }

  // 启用SIGCHLD信号，让父进程可以wait子进程退出的状态。
  signal(SIGCHLD, SIG_DFL);

  char *p_argv[argc];
  for (size_t i = 2; i < argc; i++) {
    p_argv[i - 2] = argv[i];
  }
  p_argv[argc - 2] = nullptr;

  while (1) {
    if (fork() == 0) {
      execv(argv[2], p_argv);
      exit(0);
    } else {
      int p_status;
      wait(&p_status);
      sleep(atoi(argv[1]));
    }
  }
}