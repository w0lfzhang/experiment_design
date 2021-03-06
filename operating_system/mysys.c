/*about wait/waitpid:
wait:进程一旦调用了wait，就立即阻塞自己，由wait自动分析是否当前进程的某个子进程已经退出，如果让它找到了这样一个已经变成僵尸的子进程，wait就会收集这个子进程的信息，并把它彻底销毁后返回；如果没有找到这样一个子进程，wait就会一直阻塞在这里，直到有一个出现为止。
参数status用来保存被收集进程退出时的一些状态，它是一个指向int类型的指针。
waitpid:从本质上讲，系统调用waitpid和wait的作用是完全相同的，但waitpid多出了两个可由用户控制的参数pid和options，从而为我们编程提供了另一种更灵活的方式。下面我们就来详细介绍一下这两个参数：
pid：从参数的名字pid和类型pid_t中就可以看出，这里需要的是一个进程ID。但当pid取不同的值时，在这里有不同的意义。
         pid>0时，只等待进程ID等于pid的子进程，不管其它已经有多少子进程运行结束退出了，只要指定的子进程还没有结束，waitpid就会一直等下去。
         pid=-1时，等待任何一个子进程退出，没有任何限制，此时waitpid和wait的作用一模一样。
         pid=0时，等待同一个进程组中的任何子进程，如果子进程已经加入了别的进程组，waitpid不会对它做任何理睬。
         pid<-1时，等待一个指定进程组中的任何子进程，这个进程组的ID等于pid的绝对值。
options：options提供了一些额外的选项来控制waitpid，目前在Linux中只支持WNOHANG和WUNTRACED两个选项，这是两个常数，可以用"|"运算符把它们连接起来使用，比如：
ret=waitpid(-1,NULL,WNOHANG | WUNTRACED);
如果我们不想使用它们，也可以把options设为0，如：
ret=waitpid(-1,NULL,0);
如果使用了WNOHANG参数调用waitpid，即使没有子进程退出，它也会立即返回，不会像wait那样永远等下去。

static inline pid_t wait(int * wait_stat)
{
    return waitpid(-1,wait_stat,0);
}
*/

#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <errno.h>
#include <sys/wait.h>

int mysys(const char *cmdstring)
{
  pid_t pid;
  int status;

  if(cmdstring == NULL)
  {
    return 1;
  }

  if((pid = fork()) < 0)
  {
    status = -1;
  }
  else if(pid == 0)
  {
    execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
    _exit(127);
  }
  else
  {
    while(waitpid(pid, &status, 0) < 0)
    {
      if(errno != EINTR)
      {
        status = -1;
        break;
      }
    }
  }
  
  return status;
}

int main()
{
    printf("--------------------------------------------------\n");
    mysys("echo HELLO WORLD");
    printf("--------------------------------------------------\n");
    system("ls /");
    printf("--------------------------------------------------\n");
    return 0;
}       

