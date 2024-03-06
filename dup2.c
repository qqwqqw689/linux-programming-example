#define _POSIX_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#undef _POSIX_SOURCE
#include <stdio.h>

void print_inode(int fd) {
 struct stat info;
 if (fstat(fd, &info) != 0)
   fprintf(stderr,"fstat() error for fd %d: %s\n",fd,strerror(errno));
 else
   printf("The inode of fd %d is %d\n", fd, (int) info.st_ino);
}

main() {
  int fd;
  char fn[]="dup2.file";

  if ((fd = creat(fn, S_IWUSR)) < 0)
    perror("creat() error");
  else {
    print_inode(fd);
    if ((fd = dup2(0, fd)) < 0)
      perror("dup2() error");
    else {
      puts("After dup2()...");
      print_inode(0);
      print_inode(fd);
      puts("The file descriptors are different but they");
      puts("point to the same file which is different than");
      puts("the file that the second fd originally pointed to.");
      close(fd);
    }
    unlink(fn);
  }
}
