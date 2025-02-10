#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    //cast data for better access and set return to false
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    //sleep, then try to lock mutex
    usleep(thread_func_args->twait_to_obtain_ms*1000);
 
    int rc = pthread_mutex_lock(thread_func_args->mutex);
    if ( rc != 0 )
    {
        printf("pthread_mutex_lock failed with %d\n",rc);
        thread_func_args->thread_complete_success=false;
    } 
    else
    {
    	printf("pthread_mutex_lock ok\n");
    	
    	//wait, then try to unlock
    	usleep(thread_func_args->twait_to_release_ms*1000);
    	rc = pthread_mutex_unlock(thread_func_args->mutex);
    	if ( rc != 0 ) 
	{
	   printf("pthread_mutex_unlock failed with %d\n",rc);
	   thread_func_args->thread_complete_success=false;
	}
	else
	{
	   printf("pthread_mutex_unlock ok\n");
	   thread_func_args->thread_complete_success=true;
	}
    } 
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
     bool ereturn=false;
     //set and allocate up data struct        
     struct thread_data *params;
     params = (struct thread_data *)malloc(sizeof(struct thread_data)); 
     if (params == NULL ) 
     {
     	printf("Memory allocation failure for thread params failed\n");
        return false;
     }
     
     // fill struct with function parameter data (directly set values, pointer to mutex) and initialize mutex 
     params->mutex=mutex;
     params->twait_to_obtain_ms=wait_to_obtain_ms;
     params->twait_to_release_ms=wait_to_release_ms;
     params->thread_complete_success=false;
     
     int rc = 0;
     //start thread 
     rc = pthread_create(thread,NULL,threadfunc,params);
     if ( rc != 0 )
     {
         printf("pthread_thread_create failed with %d\n",rc);
         ereturn=false;
     }
     else
     {
     	ereturn=true;
     }
    return ereturn;
}

