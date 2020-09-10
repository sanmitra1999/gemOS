#include <types.h>
#include <mmap.h>


/*** CHANGE ACCESS******/

int change_perm(struct exec_context *ctx, u64 addr,int prot ) 
{
    u64 *pte_entry = get_user_pte(ctx, addr, 0);
    
    if(!pte_entry)
             return -1; 
   
   
   *pte_entry &=0xFFFFFFFD;
   // printk("hexadecimal %x\n",*pte_entry);
   if(prot== PROT_WRITE|| prot==(PROT_READ|PROT_WRITE)){

       *pte_entry |= 0x2;
   }
    
  
    asm volatile ("invlpg (%0);" 
                    :: "r"(addr) 
                    : "memory");   // Flush TLB
      return 0;
}


/** map func*******/

u32 map_physical_page1(unsigned long base, u64 address, u32 access_flags, u32 upfn)
{
   void *os_addr;
   u64 pfn;
   unsigned long *ptep  = (unsigned long *)base + ((address & PGD_MASK) >> PGD_SHIFT);    
   if(!*ptep)
   {
      pfn = os_pfn_alloc(OS_PT_REG);
      *ptep = (pfn << PAGE_SHIFT) | 0x7; 
      os_addr = osmap(pfn);
      bzero((char *)os_addr, PAGE_SIZE);
   }else 
   {
      os_addr = (void *) ((*ptep) & FLAG_MASK);
   }
   ptep = (unsigned long *)os_addr + ((address & PUD_MASK) >> PUD_SHIFT); 
   if(!*ptep)
   {
      pfn = os_pfn_alloc(OS_PT_REG);
      *ptep = (pfn << PAGE_SHIFT) | 0x7; 
      os_addr = osmap(pfn);
      bzero((char *)os_addr, PAGE_SIZE);
   } else
   {
      os_addr = (void *) ((*ptep) & FLAG_MASK);
   }
   ptep = (unsigned long *)os_addr + ((address & PMD_MASK) >> PMD_SHIFT); 
   if(!*ptep){
      pfn = os_pfn_alloc(OS_PT_REG);
      *ptep = (pfn << PAGE_SHIFT) | 0x7; 
      os_addr = osmap(pfn);
      bzero((char *)os_addr, PAGE_SIZE);
   } else
   {
      os_addr = (void *) ((*ptep) & FLAG_MASK);
   }
   ptep = (unsigned long *)os_addr + ((address & PTE_MASK) >> PTE_SHIFT); 
   if(!upfn)
      upfn = os_pfn_alloc(USER_REG);
   *ptep = ((u64)upfn << PAGE_SHIFT) | 0x5;
   /*if(access_flags == PROT_READ)
      *ptep |= 0x1;
    else{
         *ptep |= 0x2;
    }  */

    if(access_flags == PROT_READ){
       *ptep |= 0x1;
    }
    else{

        *ptep |= 0x2;

    }       
   // printk("ptep of main is %x\n",*ptep);
   return upfn;    
}






/**
 * Function will invoked whenever there is page fault. (Lazy allocation)
 * 
 * For valid acess. Map the physical page 
 * Return 1
 * 
 * For invalid access,
 * Return -1. 
 */
int vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
   if(error_code==4){
      // printk("no read access to unmapped\n");

       unsigned long pt_base = (u64) current->pgd << PAGE_SHIFT;      

        u32 upfn = map_physical_page1(pt_base, addr, PROT_READ, upfn);
              
       return 1;
   }
    else if(error_code==6){
       //printk("no write access to unmapped\n");
       int flag=0;

       //****** CHECK IF VM AREA THERE OR NOT
      struct vm_area* temp=current->vm_area;
       while(temp!=NULL){
           if(addr == temp->vm_start){
               
              

              if(temp->access_flags == PROT_READ){                   
                   
                   return -1;
               }   

               flag=1;           

                  
                             
               
           }

           break;

           temp=temp->vm_next;
       }

      unsigned long pt_base = (u64) current->pgd << PAGE_SHIFT;          

        u32 upfn = map_physical_page1(pt_base, addr, PROT_WRITE, upfn);        
              
        return 1;
            

       
   }
    else if(error_code==7){
     
       return -1;
   }
}

/**
 * mprotect System call Implementation.
 */
int vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot)
{
    if(current->used_mem==0){
        return -1;
    }
    struct vm_area* temp=current->vm_area;
    struct vm_area* prevTemp=current->vm_area;    


    while(temp!=NULL){

        if(addr>=temp->vm_start && addr<=temp->vm_end){ 

            if(addr+length>temp->vm_end){
                return -1;
            } 

            if(addr==temp->vm_start && addr+length==temp->vm_end){

                if(temp->access_flags != prot){

                    temp->access_flags=prot;
                }
                 int check=change_perm(current,addr,prot);
                 return 0;

            }                
         
           
            else if(addr==temp->vm_start ){

                if(temp->access_flags != prot){    

                    if(stats->num_vm_area>=128){
                        return -1;
                    }                               
                    struct vm_area *newArea = alloc_vm_area();                    
                    newArea->access_flags = prot;  
                    temp->vm_start=addr+length;
                    newArea->vm_start=temp->vm_start;
                    newArea->vm_next=temp;
                    newArea->vm_end=addr+length;
                    if(temp==current->vm_area){
                        current->vm_area=newArea;
                    }                    
                                       
                         
                   
                }
                int check=change_perm(current,addr,prot);
                
                return 0;
            }


             else if(addr+length==temp->vm_end )   {

                 if(temp->access_flags != prot){

                     if(temp->vm_next!=NULL && prot==temp->vm_next->access_flags){
                         temp->vm_end=temp->vm_end-length;
                         temp->vm_next->vm_start=addr+length;
                     }
                     else {                        
                                                 
                         
                         if(stats->num_vm_area>=128){
                            return -1;
                        }    

                         struct vm_area *newArea = alloc_vm_area(); 
                         newArea->access_flags = prot;  
                         newArea->vm_start=addr;
                         newArea->vm_end=addr+length;
                         newArea->vm_next= temp->vm_next;
                         temp->vm_next=newArea;
                         temp->vm_end=temp->vm_end-length;


                     }
                     
                 }
                 int check=change_perm(current,addr,prot);
                 return 0;
             }

            else {
               if(temp->access_flags != prot){

                   if(stats->num_vm_area>=128){
                            return -1;
                        }   
                   
                    struct vm_area *newArea = alloc_vm_area(); 
                    struct vm_area *newArea1 = alloc_vm_area();                        
                   
                     newArea->access_flags = prot;  
                     newArea1->access_flags = temp->access_flags; 

                     newArea->vm_start=addr;
                     newArea->vm_next=newArea1;
                     newArea->vm_end=addr+length;
                     newArea1->vm_start=addr+length;
                     newArea1->vm_end=temp->vm_end;
                     newArea1->vm_next=temp->vm_next;
                     temp->vm_end=newArea->vm_start;                     
                     temp->vm_next=newArea;       
                   
                }
                int check=change_perm(current,addr,prot);
                
                return 0;

            }               
                       
               
        }
        
        prevTemp=temp;
        temp=temp->vm_next;        

    }

    return -1;// no such vm_area

}
/**
 * mmap system call implementation.
 */


long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{

    if (addr>MMAP_AREA_END || addr < MMAP_AREA_START)
    {
        return -1;
    }

    length = (length % 4096 != 0) ? ((length / 4096) + 1)*4096 : length; 

    if (addr == 0)
    {

                     

        if (current->vm_area == NULL)
        { //create head

            //printk("head here\n");
           // printk("cuurent mem is %x\n",current->used_mem);

            struct vm_area *newArea = alloc_vm_area();

            current->vm_area = newArea;
            newArea->vm_start = MMAP_AREA_START;
            newArea->vm_end = newArea->vm_start + length;
            newArea->vm_next = NULL;
            current->used_mem += length;
            newArea->access_flags = prot;
            
           // printk("end is %d\n",newArea->vm_end);
           if(flags == MAP_POPULATE){

                unsigned long pt_base = (u64) current->pgd << PAGE_SHIFT;

                for (int i = 0; i < (length/4096); i++)
                {
                  u32 upfn = map_physical_page1(pt_base, newArea->vm_start+i*4096, prot, upfn);
                  
                }
                


              // u32 upfn = map_physical_page1(pt_base, newArea->vm_start, prot, upfn);
           }

            return newArea->vm_start;
        }

        else
        {

             
           
            struct vm_area *temp = current->vm_area;         
            struct vm_area *temp1 = current->vm_area->vm_next;

            while (temp != NULL)
            {

                if (temp1 == NULL)
                {
                                         
                  
                    if (temp->access_flags == prot)
                    {

                        temp->vm_end = temp->vm_end + length;
                        current->used_mem += length;
                        return temp->vm_start;
                    }
                    else
                    {

                        if(stats->num_vm_area>=128){
                            return -1;
                        }   

                        struct vm_area *newArea = alloc_vm_area();
                        temp->vm_next = newArea;
                        newArea->vm_start = temp->vm_end ;
                        newArea->vm_end = newArea->vm_start + length;
                        newArea->vm_next = NULL;
                        newArea->access_flags = prot;
                        current->used_mem += length;
                        return newArea->vm_start;
                    }
                }

                else if (length <= temp1->vm_start - temp->vm_end )
                {
                   
                    if(length == temp1->vm_start - temp->vm_end  && temp->access_flags == prot && temp1->access_flags == prot){
                       // dealloc_vm_area(temp1);
                       
                       
                        temp->vm_end = temp1->vm_end ;
                        current->used_mem += length;
                        temp->vm_next=temp->vm_next->vm_next;
                        dealloc_vm_area(temp1);
                        
                        return temp->vm_start;


                    }

                    else if (temp->access_flags == prot)
                    {

                        temp->vm_end = temp->vm_end + length;
                        current->used_mem += length;
                        return temp->vm_start;
                    }
                    else if (temp1->access_flags == prot)
                    {

                        

                        temp1->vm_start = temp1->vm_start - length;
                        current->used_mem += length;
                        return temp1->vm_start;
                    }
                    else
                    {       

                        if(stats->num_vm_area>=128){
                            return -1;
                        }                    

                        struct vm_area *newArea = alloc_vm_area();
                        temp->vm_next = newArea;
                        newArea->vm_next = temp1;
                        newArea->vm_start = temp->vm_end ;
                        newArea->vm_end = newArea->vm_start + length;
                        newArea->access_flags = prot;
                        current->used_mem += length;
                        return newArea->vm_start;
                    }
                }

                temp = temp->vm_next;
                temp1 = temp1->vm_next;
            }

            return -1;
        }
    }

    // ******* HINT ADDR GIVEN**********

    else
    { // if  hint addr given

        if (addr > MMAP_AREA_END || addr < MMAP_AREA_START)
        {
            return -1;
        }

        if (current->vm_area == NULL)
        { //create head

            if(stats->num_vm_area>=128){
                            return -1;
                        }   

            struct vm_area *newArea = alloc_vm_area();

            current->vm_area = newArea;
            newArea->vm_start = addr;
            newArea->vm_end = newArea->vm_start + length;
            newArea->vm_next = NULL;
            current->used_mem += length;
            newArea->access_flags = prot;

            return newArea->vm_start;
        }
        else
        {
           

            struct vm_area *temp = current->vm_area;
            struct vm_area *temp1;

            while (temp != NULL) //******   SEARCH IF ALREADY MAPPED
            {

                if (addr <= temp->vm_end && addr >= temp->vm_start)
                {
                    if(flags==MAP_FIXED){
                        return -1;
                    }

                    // struct vm_area *temp = current->vm_area;
                    temp1 = temp->vm_next;

                    if (temp->access_flags == prot)
                    {

                        temp->vm_end = temp->vm_end + length;
                        current->used_mem += length;
                        return temp->vm_start;
                    }
                    else if ( temp1!=NULL && temp1->access_flags == prot)
                    {

                        temp1->vm_start = temp->vm_start - length;
                        current->used_mem += length;
                        return temp1->vm_start;
                    }

                    else
                    {

                        if(stats->num_vm_area>=128){
                            return -1;
                        }   

                        struct vm_area *newArea = alloc_vm_area();
                        temp->vm_next = newArea;
                        newArea->vm_start = temp->vm_end ;
                        newArea->vm_end = newArea->vm_start + length;
                        current->used_mem += length;
                        newArea->access_flags = prot;
                        newArea->vm_next = temp1;
                        return newArea->vm_start;
                    }
                }

                temp = temp->vm_next;

            } // end while

            //***** FOR AREA NOT MAPPED YET
           
            temp = current->vm_area;
            temp1 = temp->vm_next;

            while (temp != NULL)
            {
                if (temp1 == NULL)
                {    
                    if(stats->num_vm_area>=128){
                            return -1;
                        }                
                 
                        struct vm_area *newArea = alloc_vm_area();
                        temp->vm_next = newArea;
                        newArea->vm_start = addr;
                        newArea->vm_end = newArea->vm_start + length;
                        current->used_mem += length;
                        newArea->access_flags = prot;
                        newArea->vm_next = temp1;
                        return newArea->vm_start;
                    
                }

                else if (addr >= temp->vm_end && addr <= temp1->vm_start)
                {
                    if(stats->num_vm_area>=128){
                            return -1;
                        }   
                   
                        struct vm_area *newArea = alloc_vm_area();
                        temp->vm_next = newArea;
                        newArea->vm_start = addr;
                        newArea->vm_end = newArea->vm_start + length;
                        current->used_mem += length;
                        newArea->access_flags = prot;
                        newArea->vm_next = temp1;
                        return newArea->vm_start;
                    
                }

                temp = temp->vm_next;
            }
        }
    }
}
/**
 * munmap system call implemenations
 */

int vm_area_unmap(struct exec_context *current, u64 addr, int length)
{   
    struct vm_area* temp=current->vm_area;
    struct vm_area* prevTemp=NULL;       
    
    while(temp!=NULL){        

        if(addr >= temp->vm_start && addr < temp->vm_end){

            if(addr+length>temp->vm_end){
                length=temp->vm_end-addr;
            }

            //*** when entire area is to be deallocated

           // printk(" endpoint  is%x\n",temp->vm_end);
            if(addr==temp->vm_start && addr+length==temp->vm_end){
                prevTemp->vm_next=temp->vm_next;
                // printk("page no deleted is %x\n",addr);
                current->used_mem -= length;
                dealloc_vm_area(temp);
                if(temp==current->vm_area){
                    current->vm_area=NULL;
                }
                return 0;
                
            }

            else if(addr == temp->vm_start){
               
                temp->vm_start=temp->vm_start+length;                
                current->used_mem-=length;
               
                return 0;
            }

            else if(addr+length==temp->vm_end){
                temp->vm_end-=length;
                current->used_mem-=length;
                return 0;
            }            

            else{                 
                              

                struct vm_area* newArea=alloc_vm_area();              
                
                newArea->vm_start=temp->vm_start;
                newArea->vm_end=addr;
                temp->vm_start=addr+length;
                newArea->access_flags=temp->access_flags;
                newArea->vm_next=temp;
                if(prevTemp==NULL){
                    current->vm_area=newArea;
                  //  printk("start in munmap is:%x\n",current->vm_area->vm_start);
                }
                else{
                    prevTemp->vm_next=newArea;
                }

                
               
                current->used_mem-=length;
                return 0;
            }
        }
        prevTemp=temp;
        temp=temp->vm_next;      
        

    }
    return -1;
}
