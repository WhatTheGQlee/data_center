#include "_public.h"

CLogFile logfile;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("\n");
    printf("Using:./checkproc logfilename\n");

    printf(
        "Example:/project/tools1/bin/procctl 10 /project/tools1/bin/checkproc "
        "/tmp/log/checkproc.log\n\n");

    printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它。\n");
    printf("注意：\n");
    printf("  1)本程序由procctl启动，运行周期建议为10秒。\n");
    printf("  2)为了避免被普通用户误杀，本程序应该用root用户启动。\n");
    printf("  3)如果要停止本程序，只能用killall -9 终止。\n\n\n");

    return -1;
  }

  CloseIOAndSignal(true);

  if (logfile.Open(argv[1], "a+") == false) {
    printf("logfile.Open(%s) failed.\n", argv[1]);
    return -1;
  }

  int shm_id = 0;

  if ((shm_id = shmget((key_t)SHMKEYP, MAXNUMP * sizeof(struct st_procinfo),
                       0666 | IPC_CREAT)) == -1) {
    logfile.Write("创建/获取共享内存(%x)失败\n", SHMKEYP);
    return false;
  }

  struct st_procinfo *shm = (struct st_procinfo *)shmat(shm_id, 0, 0);

  for (size_t i = 0; i < MAXNUMP; i++) {
    // 如果记录的pid==0，表示空记录，continue;
    if (shm[i].pid == 0) continue;
    // 向进程发送信号0，判断它是否还存在，如果不存在，从共享内存中删除该记录，continue;
    int iret = kill(shm[i].pid, 0);
    if (iret == -1) {
      logfile.Write("进程%d(%s)已经不存在\n", (shm + i)->pid, (shm + i)->pname);
      memset(shm + i, 0, sizeof(struct st_procinfo));
      continue;
    }

    time_t now = time(0);
    //  进程未超时
    if (now - shm[i].atime < shm[i].timeout) {
      continue;
    }
    //  进程超时
    logfile.Write("进程pid=%d(%s)已经超时\n", (shm + i)->pid, (shm + i)->pname);

    kill(shm[i].pid, 15);
    for (size_t j = 0; j < 5; j++) {
      sleep(1);
      iret = kill(shm[i].pid, 0);
      if (iret == -1) break;  // 进程已退出
    }

    // 如果进程仍存在，就发送信号9，强制终止它
    if (iret == -1) {
      logfile.Write("进程pid=%d(%s)正常终止\n", (shm + i)->pid,
                    (shm + i)->pname);
    } else {
      kill(shm[i].pid, 9);
      logfile.Write("进程pid=%d(%s)强制终止。\n", (shm + i)->pid,
                    (shm + i)->pname);
    }

    memset(shm + i, 0, sizeof(struct st_procinfo));
  }

  shmdt(shm);

  return 0;
}