#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>

#define SIZE_OF_KB 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Albjon Gjuzi & Aurel Hoxha");


//ProcessID that will be given as input from the user
pid_t pid = 1;

//Necessary variables to find info about process, memory management and virtual memory
struct task_struct *task;
struct mm_struct *mm;
struct vm_area_struct *vma;
 
unsigned long vm_size = 0; //VM size in bytes will be here
int count = 0; //Count number of virtual memories
unsigned long start_stack = -1; //Variable to save stack start address
unsigned long end_of_stack = -1 ; //Variable to save the stack end address
unsigned long size_of_stack = 0; //Variable to save the size of the stack
unsigned long val, tmp; //Bit manupulations in part 2.C
pgd_t *mypgd; //Used for top-level page table entries
int count_of_vma = 1; //Used for stack start address
int i; //Used for out-page table loop


//Parameter inserted from the terminal
module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(pid, "The processID given as input from user");

int project_3_init(void) 
{
	printk(KERN_ALERT "Project 3 Starting\n");
	
	//Find the process related with the processid given from terminal
	for_each_process(task) //Could also use: task = pid_task(find_vpid(pid), PIDTYPE_PID);
		if(task->pid == pid) 
			break;
		
	//Print process information
	printk(KERN_INFO "The process is %s with pid: %d\n", task->comm, task->pid);
	
	//Point mm to memory map of the process
	mm = task->mm;
	
	//Print info about virtual memory regions
	for(vma = mm->mmap; vma; vma = vma->vm_next) {
		vm_size += vma->vm_end - vma->vm_start; //Increment overall vm size
		//printk(KERN_INFO "VM %-3d: vm-area-start: 0x%-12lx vm-area-end: 0x%-12lx vm-area-size: %-6lu kB\n"
									//, ++count, vma->vm_start, vma->vm_end, (vma->vm_end - vma->vm_start)/SIZE_OF_KB); 
					
      //Check if this block of VMA is in stack
	   if(likely(vma->vm_flags & VM_STACK))
	   {
	   	//start_stack does not give the correct output
	   	if(count_of_vma == 1) {
	   		start_stack = vma->vm_start;
	   		count_of_vma++;
	   	}
	   	
	   	//Increment stack size and update end of it
	   	size_of_stack += vma->vm_end - vma->vm_start;
	    	end_of_stack = vma->vm_end;
	   }
	}
	
	//Print code segment start and end position + its size in Kb
	printk(KERN_INFO "\ncode-segment-start:   0x%-12lx code-segment-end:   0x%-12lx code-segment-size:   %-10lu kB\n",  
			  	   mm->start_code,  mm->end_code, (mm->end_code - mm->start_code));
 	
 	//Print data segment start and end position + its size in Kb
 	printk(KERN_INFO "data-segment-start:   0x%-12lx data-segment-end:   0x%-12lx data-segment-size:   %-10lu kB\n",
               mm->start_data,  mm->end_data, (mm->end_data - mm->start_data));
   
   //Print stack segment start and end position + its size in Kb
   printk(KERN_INFO "stack-segment-start:  0x%lx stack-segment-end:  0x%lx stack-segment-size:  %-10lu kB\n",
      		   start_stack, end_of_stack, (size_of_stack));
   
   //Print heap segment start and end position + its size in Kb
   printk(KERN_INFO "heap-segment-start:   0x%-12lx heap-segment-end:   0x%-12lx heap-segment-size:   %-10lu kB\n",
               mm->start_brk,  mm->brk, (mm->brk - mm->start_brk));
   
   //Print main arguments start and end position + its size in Kb
   printk(KERN_INFO "main-arguments-start: 0x%lx main-arguments-end: 0x%lx main-arguments-size: %-10lu kB\n",
               mm->arg_start,  mm->arg_end, (mm->arg_end - mm->arg_start));
   
   //Print environmental variables start and end position + its size in Kb
   printk(KERN_INFO "env-variables-start:  0x%lx env-variables-end:  0x%lx env-variables-size:  %-10lu kB\n",
               mm->env_start,  mm->env_end, (mm->env_end - mm->env_start));
   
   //Print the number of physical frames used by the process
   printk(KERN_INFO "Number of frames used by the process (RSS) is: %lu\n", 4 * get_mm_rss(mm));
  
   //Print total size of vm
   printk(KERN_INFO "Total Virtual Memory used by process is: %lu kB\n", (vm_size / SIZE_OF_KB) );
   
	//PART C - PRINTING TOP LEVEL PAGE CONTENT
	/*mypgd = mm->pgd;
	
	for(i = 0; i < 512; i++) {
		val = pgd_val(mypgd[i]); //Convert value to unsigned long
		printk(KERN_INFO "%d value is %lx \n", i, val);
		
		if(val != 0) {	  
		   tmp = (val & 1);
		   printk(KERN_INFO "Present = %lu\n", tmp);
		   
		   tmp = (val & 2) >> 1;
		   printk(KERN_INFO "Read/Write = %lu\n", tmp);

			tmp = (val & 4) >> 2;
			printk(KERN_INFO "User/Supervisor = %lu\n", tmp);	
			
			tmp = (val & 8) >> 3;
			printk(KERN_INFO "Page-level write-through = %lu\n", tmp);
			
			tmp = (val & 16) >> 4;
			printk(KERN_INFO "Page-level cache disable = %lu\n", tmp);
			
			tmp = (val & 32) >> 5; 	   
		   printk(KERN_INFO "Accessed = %lu\n", tmp);
		   
		   //6 IS IGNORED
		   
		   tmp = (val & 128) >> 7; 
		   printk(KERN_INFO "PS = %lu\n", tmp);
		   
		   //11:8 ARE IGNORED
		   tmp = (val & 0x7FFFFFFFFF000) >> 12;
		   printk(KERN_INFO "Physical Address = %lx\n", tmp);
		   
		   //62:52 ARE IGNORED
		   tmp = (val >> 63) & 1;
		   printk(KERN_INFO "XD =  %lu\n\n", tmp);
	   }		
	}
*/
	return 0;
}

void project_3_exit(void)
{
	printk(KERN_ALERT "Project 3 Ending :(\n");
}

//Module initialization and exit function determined 
module_init(project_3_init); //Occurs when we type in the command "INSMOD"
module_exit(project_3_exit); //Occurs when we type in the command "RMMOD"
