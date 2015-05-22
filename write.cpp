#include<iostream>
#include<unistd.h>
#include <signal.h>
#include <mqueue.h>
#include<cstring>
#include <cstdio>
#include <errno.h>
#include <cstdlib>
using namespace std;



int main(int argc, char* argv[])
{
  string mq_name="/todd_player1_mq";

  if(mq_name=="/todd_player1_mq")
  {
    cerr << "Please change mq_name from \"" << mq_name << "\" to something unique to you" << endl;
    exit(1);
  }

  if(argc!=2)
  {
    cerr << "Usage: " << argv[0] << " message" << endl;
    exit(1);
  }

  mqd_t writequeue_fd; //message queue file descriptor
  if((writequeue_fd=mq_open(mq_name.c_str(), O_WRONLY|O_NONBLOCK))==-1)
  {
    perror("mq_open");
    exit(1);
  }
  cerr << "fd=" << writequeue_fd << endl;
  char message_text[121];
  memset(message_text, 0, 121);
  strncpy(message_text, argv[1], 120);
  if(mq_send(writequeue_fd, message_text, strlen(message_text), 0)==-1)
  {
    perror("mq_send");
    exit(1);
  }
  mq_close(writequeue_fd);
}
