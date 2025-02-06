#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{
	int result=-1;
	result=system(cmd);
/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
	if (result != 0)
	{
		return false;
	}
	else
	{
    		return true;
    	}
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        //printf("parameter number %d is %s \n",i,command[i]);
    }
    command[count] = NULL;
    va_end(args);
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];
    bool parent = true;
    int cres=0;
    int wcheck=1;
    fflush(stdout);
    pid_t pid = fork();
    if ( pid == -1 ) 
    {
	printf("forking failed with parameters %s %s %s \n", command[0] , command[1] , command[2]);
	return false;
    
    } 
    if ( pid == 0 ) 
    {
		parent = false;
    
    }
    if( parent ) 
    {
    	//code for checking on child process by parent process
    	printf("parent: starting wait for pid %d for executing parameters %s %s %s \n", pid, command[0] , command[1] , command[2]);
    	wcheck=waitpid(pid,&cres,0);
    	if ( wcheck == -1)
    	{
    		printf("parent: no correct return of wait, abort for parameters %s %s %s\n", command[0] , command[1] , command[2]);
    		fflush(stdout);
    		return false;
    	}
    	else
    	{
    		printf("parent: code handed for pid %d by wait: %d for parameters %s %s %s \n", pid, cres, command[0] , command[1] , command[2]); 
    		if ( cres == 0 ) 
    		{
    			printf( "parent: ok with execv parameters %s %s %s \n", command[0] , command[1] , command[2]);
    	   		fflush(stdout);
    	   		return true;
    		}
    		else
    		{
    			printf( "parent: nok with execv parameters %s %s %s \n", command[0] , command[1] , command[2]);
    	   		fflush(stdout);
    	   		return false;
    		}
    	}
    }
    else 
    {
       printf( "child with pid %d: start execv parameters %s %s %s \n",pid, command[0] , command[1] , command[2]);
       execv( command[0] , command );
       //printf( "child: return by error %s\n",  command[0]);
       exit(EXIT_FAILURE);
    }


/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
        printf("parameter number %d: %s \n", i, command[i]);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];
    va_end(args);
    int fd = open(outputfile, O_RDWR|O_CREAT|O_TRUNC);
    if (fd != -1)
    {
        printf("empty file creation ok\n");
    }
    else
    {
    	printf("empty file creation failed\n");
    	return false;
    }
    
    bool parent = true;
    int cres=0;
    int wcheck=1;
    fflush(stdout);
    
    pid_t pid = fork();
    if ( pid == -1 ) 
    {
	printf("forking failed with parameters %s %s %s \n", command[0] , command[1] , command[2]);
	return false;
    
    } 
    if ( pid == 0 ) 
    {
		parent = false;
    
    }
    if( parent ) 
    {
    	//code for checking on child process by parent process
    	//printf("parent: starting wait for pid %d for executing parameters %s %s %s \n", pid, command[0] , command[1] , command[2]);
    	wcheck=waitpid(pid,&cres,0);
    	if ( wcheck == -1)
    	{
    		printf("parent: no correct return of wait, abort for parameters %s %s %s\n", command[0] , command[1] , command[2]);
    		fflush(stdout);
    		return false;
    	}
    	else
    	{
    		freopen("/dev/tty", "w", stdout); /*for gcc, ubuntu*/
    		fsync(fd);
    		close(fd);
    		printf("parent: code handed for pid %d by wait: %d for parameters %s %s %s \n", pid, cres, command[0] , command[1] , command[2]); 
    		if ( cres == 0 ) 
    		{
    			printf( "parent: ok with execv parameters %s %s %s \n", command[0] , command[1] , command[2]);
    	   		fflush(stdout);
    	   		return true;
    		}
    		else
    		{
    			printf( "parent: nok with execv parameters %s %s %s \n", command[0] , command[1] , command[2]);
    	   		fflush(stdout);
    	   		return false;
    		}
    	}
    }
    else 
    {
       printf( "child with pid %d: start execv parameters %s %s %s \n",pid, command[0] , command[1] , command[2]);
       dup2(fd,1);
       execv( command[0] , command );
       //printf( "child: return by error %s\n",  command[0]);
       exit(EXIT_FAILURE);
    }
    
    /*
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);
    printf("target for outputfile: %s \n", outputfile);
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    //command[count] = command[count];
    close(fd);
    exit(EXIT_FAILURE);
    
    */
/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/


    

    return true;
}
