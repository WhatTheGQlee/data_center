#include "_ftp.h"

Cftp ftp;

int main() {
  if (ftp.login("47.120.62.211:21", "test", "123456") == false) {
    printf("ftp.login(47.120.62.211:21) failed.\n");
    return -1;
  }

  printf("ftp.login(47.120.62.211:21) ok.\n");

  if (ftp.mtime("/tmp.json") == false) {
    printf("ftp.mtime(/tmp.json) failed.\n");
    return -1;
  }
  printf("ftp.mtime(/tmp.json) ok. mtime=%s.\n", ftp.m_mtime);

  if (ftp.size("/tmp.json") == false) {
    printf("ftp.size(/tmp.json) failed.\n");
    return -1;
  }
  printf("ftp.size(/tmp.json) ok. size=%d.\n", ftp.m_size);

  if (ftp.get("/tmp.json", "/home/gqlee/project/tmp/tmp.json.bak", true) ==
      false) {
    printf("ftp.get() failed.\n");
    return -1;
  }
  printf("ftp.get ok.\n");

  if (ftp.put("/home/gqlee/project/util/deletefiles.cpp",
              "/deletefiles.cpp.bak", true) == false) {
    printf("ftp.put() failed.\n");
    return -1;
  }
  printf("ftp.put ok.\n");

  ftp.logout();
  return 0;
}