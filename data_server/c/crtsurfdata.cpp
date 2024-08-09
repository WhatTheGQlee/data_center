/**
 * @file crtsurfdata
 * @brief Brief description of the file.
 *
 *  生成全国气象站点观测的分钟数据
 *
 * @author GQlee
 * @date 2024-03-13
 *
 */

#include "_public.h"

//  全国气象站点参数结构体
struct st_stcode {
  char prov_name[31];  // 省
  char obt_id[11];     // 站号
  char obt_name[31];   // 站名
  double lat;          // 纬度
  double lon;          // 经度
  double height;       // 海拔高度
};
//  存放全国气象站点参数的容器
vector<struct st_stcode> vst_code;
// 把站点参数文件中加载到vstcode容器中
bool loadSTCode(const char* inifile);

//  全国气象站点分钟观测数据结构
struct st_surfdata {
  char obt_id[11];     // 站点代码。
  char ddatetime[21];  // 数据时间：格式yyyymmddhh24miss
  int tem;             // 气温：单位，0.1摄氏度。
  int pa;              // 气压：0.1百帕。
  int u;               // 相对湿度，0-100之间的值。
  int wd;              // 风向，0-360之间的值。
  int wf;              // 风速：单位0.1m/s
  int r;               // 降雨量：0.1mm。
  int vis;             // 能见度：0.1米。
};
//  存放全国气象站点分钟观测数据的容器
vector<struct st_surfdata> vsurf_data;
char cur_time[21];
// 模拟生成全国气象站点分钟观测数据，存放在vsurf_data容器中
void loadSurfData();

// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
bool loadSurfFile(const char* outpath, const char* datafmt);

CLogFile logfile;  //  日志类
CFile File;        // 文件类
CPActive PActive;  // 进程心跳

void EXIT(int sig);

int main(int argc, char* argv[]) {
  /**
   * @brief Brief description
   *
   * @param inifile 全国气象站点参数文件名
   * @param outpath 全国气象站点数据文件存放的目录
   * @param logfile 本程序运行的日志文件名
   * @return Description of return value
   */
  if (argc != 5 && argc != 6) {
    printf("Using:./crtsurfdata inifile outpath logfile datafmt\n");
    printf(
        "Example:/home/gqlee/project/bin/crtsurfdata "
        "/home/gqlee/project/data_server/ini/stcode.ini "
        "/home/gqlee/tmp/surfdata "
        "/home/gqlee/project/log/data_server.log "
        "csv\n");

    printf("inifile 全国气象站点参数文件名。\n");
    printf("outpath 全国气象站点数据文件存放的目录。\n");
    printf("logfile 本程序运行的日志文件名。\n");
    printf("datafmt 生成数据文件的格式。\n");
    printf("datetime 这是一个可选参数，表示生成指定时间的数据和文件。\n\n\n");

    return -1;
  }

  // 关闭全部的信号和输入输出。
  // 设置信号,在shell状态下可用 "kill + 进程号" 正常终止些进程。
  // 但请不要用 "kill -9 +进程号" 强行终止。
  CloseIOAndSignal(true);
  signal(SIGINT, EXIT);
  signal(SIGTERM, EXIT);

  // 打开程序的日志文件。
  if (logfile.Open(argv[3]) == false) {
    printf("logfile.Open(%s) failed.\n", argv[3]);
    return -1;
  }
  logfile.Write("crtsufdata 开始运行。 \n");

  PActive.AddPInfo(20, "crtsurfdata");

  // 把站点参数文件中加载到vst_code容器中
  if (loadSTCode(argv[1]) == false) {
    printf("inifile %s load failed.\n", argv[1]);
    return -1;
  }

  memset(cur_time, 0, sizeof(cur_time));
  if (argc == 5) {
    LocalTime(cur_time, "yyyymmddhh24miss");
  } else {
    STRCPY(cur_time, sizeof(cur_time), argv[5]);
  }
  // 模拟生成全国气象站点分钟观测数据，存放在vsurf_data容器中
  loadSurfData();

  // 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
  if (strstr(argv[4], "xml") != 0)
    loadSurfFile(argv[2], "xml");
  if (strstr(argv[4], "json") != 0)
    loadSurfFile(argv[2], "json");
  if (strstr(argv[4], "csv") != 0)
    loadSurfFile(argv[2], "csv");

  logfile.Write("crtsufdata 结束运行。 \n");

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool loadSTCode(const char* inifile) {
  if (File.Open(inifile, "r") == false) {
    logfile.Write("inifile %s open failed.\n", inifile);
    return false;
  }

  char strBuffer[301];
  CCmdStr CmdStr;
  struct st_stcode stcode;

  while (true) {
    // 从站点参数文件中读取一行，如果已读取完，跳出循环
    if (File.Fgets(strBuffer, 300, true) == false) {
      break;
    }

    // 把读取到的一行拆分
    // logfile.Write("%s \n", strBuffer);
    CmdStr.SplitToCmd(strBuffer, ",", true);

    if (CmdStr.CmdCount() != 6)
      continue;

    // 把站点参数的每个数据项保存到站点参数结构体中
    memset(&stcode, 0, sizeof(struct st_stcode));
    CmdStr.GetValue(0, stcode.prov_name, 30);
    CmdStr.GetValue(1, stcode.obt_id, 10);
    CmdStr.GetValue(2, stcode.obt_name, 30);
    CmdStr.GetValue(3, &stcode.lat);
    CmdStr.GetValue(4, &stcode.lon);
    CmdStr.GetValue(5, &stcode.height);

    // 把站点参数结构体放入站点参数容器
    vst_code.push_back(stcode);
  }

  return true;
}

void loadSurfData() {
  srand(time(0));

  //  获取当前时间作为观测时间

  struct st_surfdata stsurfdata;

  for (size_t i = 0; i < vst_code.size(); ++i) {
    memset(&stsurfdata, 0, sizeof(struct st_surfdata));

    strncpy(stsurfdata.obt_id, vst_code[i].obt_id, 10);  // 站点代码
    strncpy(stsurfdata.ddatetime, cur_time, 14);         // 数据时间
    stsurfdata.tem = rand() % 351;         // 气温：单位，0.1摄氏度
    stsurfdata.pa = rand() % 265 + 10000;  // 气压：0.1百帕
    stsurfdata.u = rand() % 100 + 1;  // 相对湿度，0-100之间的值。
    stsurfdata.wd = rand() % 360;     // 风向，0-360之间的值。
    stsurfdata.wf = rand() % 150;     // 风速：单位0.1m/s
    stsurfdata.r = rand() % 16;       // 降雨量：0.1mm
    stsurfdata.vis = rand() % 5001 + 100000;  // 能见度：0.1米

    vsurf_data.push_back(stsurfdata);
  }
}

bool loadSurfFile(const char* outpath, const char* datafmt) {
  char strFileName[301];
  sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, cur_time, getpid(),
          datafmt);

  if (File.OpenForRename(strFileName, "w") == false) {
    logfile.Write("File.OpenForRename %s failed. \n", strFileName);
    return false;
  }

  if (strcmp(datafmt, "csv") == 0) {
    File.Fprintf(
        "站点代码\t数据时间\t气温\t气压\t相对湿度\t风向\t风速\t降雨量\t能见度"
        "\n");
  }
  if (strcmp(datafmt, "xml") == 0) {
    File.Fprintf("<data>\n");
  }
  if (strcmp(datafmt, "json") == 0) {
    File.Fprintf("{\"data\":[\n");
  }

  for (size_t i = 0; i < vsurf_data.size(); i++) {
    // 写入一条记录。
    if (strcmp(datafmt, "csv") == 0)
      File.Fprintf(
          "%s\t"
          "%s\t"
          "%-4.1f\t"
          "%.1f\t"
          "%-2d\t"
          "%-3d\t"
          "%-4.1f\t"
          "%.1f\t"
          "%.1f\n",
          vsurf_data[i].obt_id, vsurf_data[i].ddatetime,
          vsurf_data[i].tem / 10.0, vsurf_data[i].pa / 10.0, vsurf_data[i].u,
          vsurf_data[i].wd, vsurf_data[i].wf / 10.0, vsurf_data[i].r / 10.0,
          vsurf_data[i].vis / 10.0);

    if (strcmp(datafmt, "xml") == 0)
      File.Fprintf(
          "<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</"
          "t><p>%.1f</p>"
          "<u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/"
          ">\n",
          vsurf_data[i].obt_id, vsurf_data[i].ddatetime,
          vsurf_data[i].tem / 10.0, vsurf_data[i].pa / 10.0, vsurf_data[i].u,
          vsurf_data[i].wd, vsurf_data[i].wf / 10.0, vsurf_data[i].r / 10.0,
          vsurf_data[i].vis / 10.0);

    if (strcmp(datafmt, "json") == 0) {
      File.Fprintf(
          "{\"obtid\":\"%s\",\"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":\"%."
          "1f\","
          "\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\",\"vis\":"
          "\"%.1f\"}",
          vsurf_data[i].obt_id, vsurf_data[i].ddatetime,
          vsurf_data[i].tem / 10.0, vsurf_data[i].pa / 10.0, vsurf_data[i].u,
          vsurf_data[i].wd, vsurf_data[i].wf / 10.0, vsurf_data[i].r / 10.0,
          vsurf_data[i].vis / 10.0);

      if (i < vsurf_data.size() - 1) {
        File.Fprintf(",\n");
      } else {
        File.Fprintf("\n");
      }
    }
  }

  if (strcmp(datafmt, "xml") == 0) {
    File.Fprintf("</data>\n");
  }
  if (strcmp(datafmt, "json") == 0) {
    File.Fprintf("]}\n");
  }

  File.CloseAndRename();
  UTime(strFileName, cur_time);  // 修改文件的时间属性
  logfile.Write("生成数据文件%s成功,数据时间%s,记录数%d.\n", strFileName,
                cur_time, vsurf_data.size());
  return true;
}

void EXIT(int sig) {
  logfile.Write("程序退出，sig=%d\n\n", sig);
  exit(0);
}