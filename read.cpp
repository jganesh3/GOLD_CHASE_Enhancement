#include<iostream>
#include<unistd.h>
#include <signal.h>
#include <mqueue.h>
#include<cstring>
#include <cstdio>
#include <errno.h>
#include <cstdlib>
using namespace std;

mqd_t readqueue_fd; //message queue file descriptor
string mq_name="/todd_player1_mq";

void read_message(int)
{
  //set up message queue to receive signal whenever
  //message comes in
  struct sigevent mq_notification_event;
  mq_notification_event.sigev_notify=SIGEV_SIGNAL;
  mq_notification_event.sigev_signo=SIGUSR2;
  mq_notify(readqueue_fd, &mq_notification_event);

  //read a message
  int err;
  char msg[121];
  memset(msg, 0, 121);//set all characters to '\0'
  while((err=mq_receive(readqueue_fd, msg, 120, NULL))!=-1)
  {
    cout << "Message received: " << msg << endl;
    memset(msg, 0, 121);//set all characters to '\0'
  }
  //we exit while-loop when mq_receive returns -1
  //if errno==EAGAIN that is normal: there is no message waiting
  if(errno!=EAGAIN)
  {
    perror("mq_receive");
    exit(1);
  }
}


void clean_up(int)
{
  cerr << "Cleaning up message queue" << endl;
  mq_close(readqueue_fd);
  mq_unlink(mq_name.c_str());
  exit(1);
}

int main()
{
  if(mq_name=="/todd_player1_mq")
  {
    cerr << "Please change mq_name from \"" << mq_name << "\" to something unique to you" << endl;
    exit(1);
  }

  //I have added this signal-handling
  //code so that if you type ctrl-c to
  //abort the long, slow loop at the
  //end of main, then your message queue
  //will be properly cleaned up.
  struct sigaction exit_handler;
  exit_handler.sa_handler=clean_up;
  sigemptyset(&exit_handler.sa_mask);
  exit_handler.sa_flags=0;
  sigaction(SIGINT, &exit_handler, NULL);



  //make sure we can handle the SIGUSR2
  //message when the message queue
  //notification sends the signal
  struct sigaction action_to_take;
  //handle with this function
  action_to_take.sa_handler=read_message;
  //zero out the mask (allow any signal to interrupt)
  sigemptyset(&action_to_take.sa_mask);
  action_to_take.sa_flags=0;
  //tell how to handle SIGINT
  sigaction(SIGUSR2, &action_to_take, NULL);

  struct mq_attr mq_attributes;
  mq_attributes.mq_flags=0;
  mq_attributes.mq_maxmsg=10;
  mq_attributes.mq_msgsize=120;

  if((readqueue_fd=mq_open(mq_name.c_str(), O_RDONLY|O_CREAT|O_EXCL|O_NONBLOCK,
          S_IRUSR|S_IWUSR, &mq_attributes))==-1)
  {
    perror("mq_open");
    exit(1);
  }
  //set up message queue to receive signal whenever message comes in
  struct sigevent mq_notification_event;
  mq_notification_event.sigev_notify=SIGEV_SIGNAL;
  mq_notification_event.sigev_signo=SIGUSR2;
  mq_notify(readqueue_fd, &mq_notification_event);


  //magic happens
  for(int i=0; i<100; ++i)
  {
    cout << "counter=" << i << endl;
    sleep(1);
  }

  mq_close(readqueue_fd);
  mq_unlink(mq_name.c_str());
}
