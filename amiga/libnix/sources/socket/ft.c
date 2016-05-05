#include <fcntl.h>
#include <unistd.h>

int main(void)
{
  int fd1 = open("t:t1",O_TRUNC);
  int fd2 = open("t:t2",O_TRUNC);
  int fd3 = dup2(fd1,fd2);
  int fd4 = dup(fd1);
  close(fd2);
  fd2 = open("t:t2",O_RDONLY);
}
