#include<pthread.h>
#include "common.h"

/*TODO:  
     Insert approproate synchronization statements
     to make it work with multiple threads
*/
void *hashit(void *arg)
/*Argument is the end pointer*/
{
   char *cptr;
   unsigned long *chash;
   pthread_mutex_lock(&lock);

   char *endptr = (char *)arg;   // END pointer
   pthread_mutex_unlock(&lock);

    

   while(1){

           pthread_mutex_lock(&lock);

         
        if(dataptr >= endptr){

               pthread_mutex_unlock(&lock);
              break;
        }
       
        cptr = dataptr;
        dataptr += BLOCK_SIZE;
        
        chash = optr;
        optr++;
         pthread_mutex_unlock(&lock); 
   
        /*   Perform the real calculation. The following line should not be inside any locks*/
        *chash = calculate_and_store_hash(cptr, endptr); 
  }
  pthread_exit(NULL); 
}
