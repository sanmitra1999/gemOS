#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<common.h>

/* XXX NOTE XXX  
       Do not declare any static/global variables. Answers deviating from this 
       requirement will not be graded.
*/
void init_rwlock(rwlock_t *lock)
{
   /*Your code for lock initialization*/

   lock->value= 0x1000000000000 ;
 //  printf("hello\n");


}

void write_lock(rwlock_t *lock)
{
   /*Your code to acquire write lock*/
  // printf("here\n");
  
   
   while(/*lock->value!=0x1000000000000*/ atomic_add(&lock->value,0)!=1  ){
      
   }
  
  lock->value=0x0000000000000;

   
}

void write_unlock(rwlock_t *lock)
{
   /*Your code to release the write lock*/
  
   lock->value=  0x1000000000000 ;
}


void read_lock(rwlock_t *lock)
{
   /*Your code to acquire read lock*/
   

   while(atomic_add(&lock->value,0)!=1){


   }
  //  printf("here\n");
   if(atomic_add(&lock->value,0)==1){
      write_lock(lock);
   }

  int ret=atomic_add(&lock->value,-1);
 // printf("ret is %d\n",ret);


}

void read_unlock(rwlock_t *lock)
{
   /*Your code to release the read lock*/
  
  // printf("hex is %lx\n",lock->value);
   int ret=atomic_add(&lock->value,1);
  // printf("check %d\n",ret);

  
   if(ret==0 ){
       
      
       write_unlock(lock);

   }
   /*else{
     //  printf("here\n");
          int ret=atomic_add(&lock->value,1);
        if(ret==1){
              write_unlock(lock);

          }
   }*/
}
