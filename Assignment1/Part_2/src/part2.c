#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>

// Sanmitra Chakraborty 160624

int main(int argc, char **argv)
{
  

    if (strcmp(argv[1],"@")==0) // part 2.1
    {



        pid_t cpid;
        pid_t cpid1;
        int status, status1;
        int flag = 0;

        char buf[50];

        int fd[2];
        if (pipe(fd) < 0)
        {
            perror("pipe");
            exit(-1);
        }

        int countLines = 0;
        int pid;
        pid = fork();
        if (!pid)
        { //child reads and outputs into stdout

            dup2(fd[1], 1);
            close(fd[0]);
            char *arg_list[] = {"grep", "-r", argv[2], argv[3], 0};

            execvp(arg_list[0], arg_list);
        }

        else
        {

            dup2(fd[0], 0);
            close(fd[1]);

            //cpid= wait(&status);
            while (1)
            {

                //scanf("%s",buf);
                if (read(fd[0], buf, 10) < 0)
                {
                    perror("read");
                    exit(-1);
                }

                if (buf[0] == 0)
                {
                    break;
                }

                buf[10] = 0;
                //printf("%s\n",buf);

                char temp = 'a';
                int i = 0;

                while (temp != '\0')
                {

                    temp = buf[i];
                    if (temp == '\n')
                    {
                        countLines++; //counts lines
                    }
                    i++;
                }

                memset(buf, 0, sizeof(buf));
            }
            printf("%d\n", countLines);

            
        }
    }

    //Part 2.2
    else if (strcmp(argv[1],"$")==0)
    {

        pid_t cpid, cpid1;
        int status, status1;

        char buf[200];
        char buf1[50];
        int countLines = 0;

        int fd[2];
        if (pipe(fd) < 0)
        {
            perror("pipe");
            exit(-1);
        }

        int pid;
        int pid1;
        pid = fork();
        if (!pid)
        { //child reads and outputs into stdout

            int fd1[2];
            if (pipe(fd1) < 0)
            {
                perror("pipe");
                exit(-1);
            }

            pid1 = fork();

            if (!pid1)
            {

                close(fd1[0]);
                close(fd[1]);
                close(fd[0]);

                dup2(fd1[1], 1);

                char *arg_list[] = {"grep", "-rF", argv[2], argv[3], 0};

                execvp(arg_list[0], arg_list);
            }

            else
            {
                //cpid1=wait(&status1);
                close(fd[0]);
                close(fd1[1]);                // opens file and weites into it

                dup2(fd1[0], 0);
                dup2(fd[1], 1);
                int file = open(argv[4], O_CREAT | O_RDWR | O_TRUNC, 0666); 

                while (1)
                {

                    if (read(fd1[0], buf1, 10) < 0)
                    { 
                        perror("read");
                        exit(-1);
                    }
                    if (buf1[0] == 0)
                    {
                        break;
                    }
                    buf1[10] = 0;
                   
                    write(file, buf1, strlen(buf1));

                    memset(buf1, 0, sizeof(buf1));
                }

                close(file);
                printf("%s", argv[4]);
            }
        }

        else
        {
            // cpid= wait(&status);
            close(fd[1]);
            dup2(fd[0], 0);

            if (strcmp(argv[5], "wc") == 0)
            {

                char fileName[200];

                // scanf("%s",fileName);                   //gets file name and counts lines
                fgets(fileName, 200, stdin);
                char temp;
                int countLines=0;
                
                int fp,r;
                 if((fp=open(fileName,O_RDONLY)) != -1) {

                      while((r=read(fp,&temp,sizeof(char)))!= 0) {

                          if(temp=='\n'){
                              countLines++;
                          }
                      }
                 }



                 printf("%d\n",countLines);

            }

            else if (strcmp(argv[5], "sort") == 0)
            {

                char fileName[100];

                scanf("%s", fileName);
                char *arg_list[] = {"sort", fileName, 0};  //gets filename and sorts

                execvp(arg_list[0], arg_list);
            }
        }
    }

    return 0;
}