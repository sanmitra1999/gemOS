#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<assert.h>
#include <fcntl.h>
#include<sys/types.h>
#include <sys/stat.h>
#include<dirent.h>
#include <errno.h>

// Samitra Chakraborty 160624

void printLines(char* cwd,char* arg,int flag){ //print lines which have the word in them
   
    int fd,r,j=0; 
    char temp,line[BUFSIZ]; 

    if( (fd=open(cwd,O_RDONLY)) == -1){
         printf("No such file or directory\n");
         exit(-1);
    }

    if((fd=open(cwd,O_RDONLY)) != -1) 
    { 
        while((r=read(fd,&temp,sizeof(char)))!= 0) 
        { 
            if(temp!='\n') 
            { 
                line[j]=temp; 
                j++; 
            } 
            else 
            { 
                line[j]='\0';
                if(strstr(line,arg)!=NULL) {
                    if(flag==1){
                        printf("\033[0;35m");
                         printf("%s:",cwd);
                        printf("\033[0m");
                    }
                    char word[256];
                    
                     printf("%s\n",line); 
                }
                   
                memset(line,0,sizeof(line)); 
                j=0; 
            } 

        } 
    }  
  
}


void recursiveSearch(char* cwd,char* arg){ // go through all sub directories
    
   
   
    char temp[256];
   
   
    DIR* dir=opendir(cwd);
    DIR* test;
    struct dirent *rd;
   
    
    

    while( (rd=readdir(dir))!=NULL ){

      
        if(strcmp(rd->d_name,".")==0 || strcmp(rd->d_name,"..")==0 ){
            continue;
        }
        strcpy(temp,cwd);           
           
       
        
        strcat(temp,"/");        
        strcat(temp,rd->d_name);        
        

        test=opendir(temp);
        if(test){
         
           recursiveSearch(temp,arg);
            
        }

        else{
         
          
            
            printLines(temp,arg,1);
        }

       
    }
  

}

int main(int argc,char** argv){

    int flag=0;

    if(argc!=3){
        printf("Error:Enter two arguements\n");
        exit(-1);
    }

    /*path format if complete path is given*/

   
    DIR *directoryToOpen1;
    directoryToOpen1=opendir(argv[2]);
    if(directoryToOpen1){        
        
        recursiveSearch(argv[2],argv[1]);
          
    }
    else if(ENOTDIR==errno){ // if not a directory
         printLines(argv[2],argv[1],0);
    }
    else if(ENOENT==errno){ //if path not valid

        
        printf("No such file or Directory\n");
        exit(-1);

    }
   

    
    
    return 0;
}
