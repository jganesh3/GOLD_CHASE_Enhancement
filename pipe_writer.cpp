#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include<iostream>
#include<unistd.h>
#include <signal.h>
#include <mqueue.h>
#include<cstring>
#include <cstdio>
#include <errno.h>
#include <cstdlib>
using namespace std;


template<typename T>
int WRITE(int fd, T* obj_ptr, int count)
{
  char* addr=(char*)obj_ptr;
  int bytes_sent=count;
  int written=0;

  while(bytes_sent > 0){
    written+=write(fd,(void*)obj_ptr,count);
    if(written==-1)
    {
      perror("This is shit\n");
      exit(1);
    }
    bytes_sent=bytes_sent-written;
  }


  return written;

}


int main()
{
    int fd;

    string myfifo = "/tmp/myfifo";

    /* create the FIFO (named pipe) */
    mkfifo(myfifo.c_str(), 0666);

    /* write "Hi" to the FIFO */
    fd = open(myfifo.c_str(), O_WRONLY);
    int sent=WRITE(fd, "Hi", sizeof("Hi")); //2
    sent+=  WRITE(fd, "Ganesh", sizeof("Ganesh")); //6
    sent+=WRITE(fd, " Joshi", sizeof(" Joshi")); //6
    printf("Number of bytes sent %d",sent);
    close(fd);

    /* remove the FIFO */
    unlink(myfifo.c_str());

    return 0;
}
