#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include <sys/stat.h>   
#include<dirent.h>
#include <errno.h>
#include<inttypes.h>
#include<sys/wait.h> 
#include<dirent.h>
#include<libgen.h>

// Samitra Chakraborty 160624


 off_t getSize(char* cwd){
    // printf("we are in function\n");

     off_t size=(off_t)0;

     DIR *direc;
     direc=opendir(cwd);
     DIR *test;
     struct dirent* rd;
     char temp[256];

      while( (rd=readdir(direc))!=NULL ){

        //printf("we are in loop\n");
        if(strcmp(rd->d_name,".")==0 || strcmp(rd->d_name,"..")==0 ){
            continue;
        }
        strcpy(temp,cwd);           
           
       
        
        strcat(temp,"/");        
        strcat(temp,rd->d_name);        
        //printf("%s\n",temp);         

        test=opendir(temp);
        if(test){
            size+=getSize(temp);
           
            
        }

        else if(ENOTDIR==errno){
          //  printf("we are here in dir %s\n", temp);
           int file=open(temp,O_RDONLY);
            //off_t currentPos = lseek(fd, (size_t)0, SEEK_CUR);
            size += lseek(file, (size_t)0, SEEK_END);
            lseek(file, (size_t)0, SEEK_SET);
          
        }
        

       
    }
   // printf("size in getSize is%zd\n",size);

    return size;


 }

int main(int argc,char** argv){

    if(argc!=2){
        printf("Error:Enter two arguements\n");
        exit(-1);
    }
    //int fd[100][2];



    DIR* direc;
    off_t size=(off_t)0;

    direc=opendir(argv[1]);
    if(direc){                      // check if dir or not
        
        //recurSearch(argv[1]);

        int count=0;
         int fd[2000][2];
         char buf[15];

         
         
  //  off_t sizeChild=(size_t)0;
        char temp[256];
        char temp1[256];
         DIR* test;
         pid_t pid;
        pid_t cpid;
        int status;
    
    

         struct dirent *rd;
         struct dirent *rd1;

    while( (rd=readdir(direc))!=NULL ){           //loop through dir

      //  printf("we are in loop\n");
        if(strcmp(rd->d_name,".")==0 || strcmp(rd->d_name,"..")==0 ){
            continue;
        }
        strcpy(temp,argv[1]);           
           
       
        
        strcat(temp,"/");        
        strcat(temp,rd->d_name);  
        strcpy(temp1,rd->d_name) ;    
        //printf("%s\n",temp);         

        test=opendir(temp);
        if(test){
            
           

            if(pipe(fd[count])<0){                     //get sub dir
                perror("pipe");
                exit(-1);
            }


          
           pid=fork();

           if(!pid){

               close(fd[count][0]);
               dup2(fd[count][1],1);
               off_t size1;
               
               
               size1=getSize(temp);
                
               
                       // if child recurse and exec
               printf("%zd",size1);
               exit(1);

              
           }
           else{
              
               cpid=wait(&status);
               off_t childSize=(off_t)0;
               close(fd[count][1]);
               dup2(fd[count][0],0);
               
               if( read(fd[count][0],buf,15)<0 ){//read input to buffer
                    perror("read");
                    exit(-1);
                }
                buf[15]=0;
               
               int s=atoi(buf);
               childSize=(off_t)s;                   // print to parent
               size+=childSize;    
                printf("%s ",temp1);
               printf("%zd\n",childSize);   
               memset(buf,0,sizeof(buf));        


           }

            count++;  
            
        }
            
       
    }   
    

    DIR* direc1=opendir(argv[1]);                               // get remaining files in root

    while( (rd1=readdir(direc1))!=NULL ){
       


         if(strcmp(rd1->d_name,".")==0 || strcmp(rd1->d_name,"..")==0 ){
            continue;
        }
        strcpy(temp,argv[1]);           
           
       
        
        strcat(temp,"/");        
        strcat(temp,rd1->d_name);        
             

        test=opendir(temp);
        if(!test){
            int file=open(temp,O_RDONLY);
            
            size += lseek(file, (size_t)0, SEEK_END);
            lseek(file, (size_t)0, SEEK_SET);
        }

    }
    char* folderName;
    folderName=basename(argv[1]);

    
    printf("%s %zd \n",folderName,size);        

      
          
    }
   
    else if(ENOENT==errno){ //if path not valid
       
        printf("No such file or Directory\n");
        exit(-1);

    }

    return 0;
}
