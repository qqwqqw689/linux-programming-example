#include <sys/utsname.h>
#include <stdio.h>

int main() {
 	struct utsname uts;

 	if (uname(&uts) < 0)
	// On error, -1 is returned
		perror("uname() error");
		// void perror(const char *s);
		// print a system error message
 	else {
		printf("Sysname:  %s\n", uts.sysname);
 		printf("Nodename: %s\n", uts.nodename);
		printf("Release:  %s\n", uts.release);
		printf("Version:  %s\n", uts.version);
		printf("Machine:  %s\n", uts.machine);
    	}
    	
    	return 0;
  }
