#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/poll.h>
#include <pthread.h>
#include <time.h>

FILE *fp;
//const int cargc=2;
//int aargc=0;
const int spriority=(LOG_DEBUG|LOG_USER);
const int epriority=(LOG_ERR|LOG_USER);
pthread_mutex_t fmutex;



#define PORT "9000"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

#define MAXDATASIZE 2 // max number of bytes we can get at once 

#define MAXRECSIZE 10 //maximum mumber of bytes to be received by by one call of rcv, -1 character (so we can store \0 to the last index)    

#define STARTPSIZE 30 //standard mumber of bytes to be reserved for storing a complete package 


int sockfd;
int new_fd;  // listen on sock_fd, new connection on new_fd


bool nomore_connect=false;
bool daemon_act=false;

//path to file file to append to create, if required
const char *stfile="/var/tmp/aesdsocketdata";


struct threaddata {

	//name of client
	char ts[INET6_ADDRSTRLEN];
	//connection descriptor
	int tcond;
	//pointer to thread
	pthread_t *thread_pointer;
	//pointer to next thread data
	struct threaddata* next;
};

struct threaddata *head=NULL;

pthread_t *timer_thread_pointer;

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
	switch (s) {
	case SIGCHLD:
	    while(waitpid(-1, NULL, WNOHANG) > 0);
	    break;
	case SIGINT:
	case SIGTERM:
	    //Gracefully exits, 
	    //completing any open connection operations, 
	    //closing any open sockets, 
	    //and deleting the file /var/tmp/aesdsocketdata.
	    //signal to not accept any more connection requests after this one is finished processing
	    nomore_connect=true;
	    printf ("Caught signal, exiting\n");
            syslog(spriority, "Caught signal, exiting\n");
	    break;
	default: /*Should never get this case*/
	    break;
    }
    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//thread function taking connection descriptor as parameter (copy of value so)
static void *runcon (void *arg)
{
    	struct threaddata *tdata = (struct threaddata *)arg;
	FILE *tfp;
	char trchar;
	int tnumbytes;
	int tpbufs=STARTPSIZE;
	int mc;
	int tpcounter=0;
	char *trptr, *tpptr;
	trptr = (char*) malloc(MAXRECSIZE * sizeof(char));
	tpptr = (char*) malloc(tpbufs * sizeof(char));
	*tpptr='\0';
	printf ("Accepted connection from %s, thread started, waiting for data\n", tdata->ts);
	syslog(spriority, "server: got connection from %s, thread started, waiting for data\n", tdata->ts);
	while ((tnumbytes = recv(tdata->tcond, trptr, MAXRECSIZE-1, 0)) > 0)
	{
		//add retrieved data to packet buffer
		//add end of string character to end of reception so string operations work
		*(trptr + tnumbytes)='\0';
		//printf("server: received data %s \n",rptr);
		//expand package buffer if necessary (compare with numbytes+1 because of end of string character)
		while ((tpcounter+tnumbytes+1) > tpbufs)
		{
			//double size of package buffer and reallocate
			tpbufs=tpbufs*2;
			tpptr = realloc(tpptr, tpbufs * sizeof(char));
			if (tpptr != NULL)
			{
				printf("server: successfully expanded package buffer to size %d \n", tpbufs);
			}
			else
			{
				printf ("packet buffer expansion failed\n");
				syslog(epriority, "packet buffer expansion failed\n");
				free(trptr);
				free(tpptr);
				close(tdata->tcond);
				return 1;
			}
		}
		//concatenate strings into package buffer pptr and update counters, print package data received so far
		strcat(tpptr,trptr);
		tpcounter=tpcounter+tnumbytes;
		//printf("server: received package data so far %s, counter value is %d \n",tpptr, pcounter);
		//check if end of package newline character has been received
		if (*(trptr + tnumbytes - 1) == '\n')
		{	
			//printf("server: received complete package '%s'\n",tpptr);

			//package received completely, append to file and send back complete file after locking mutex, then unlock mutex
			mc = pthread_mutex_lock(&fmutex);
			if (mc != 0)
			{
				printf ("file write mutex lock not successfull, exit thread\n");
				syslog(epriority, "file write mutex lock not successfull, exit thread\n");
				free(trptr);
				free(tpptr);
				close(tdata->tcond);
				return 1;	
			}
			else
			{
				printf ("file write mutex lock successfull\n");
				syslog(epriority, "file write mutex lock successfull\n");
			}
			if ((tfp = fopen(stfile, "a+")) == NULL)
			{
				printf ("path does not exist or is locked so file cannot be created or appended\n");
				syslog(epriority, "path does not exist or is locked so file cannot be created or appended\n");
				mc = pthread_mutex_unlock(&fmutex);
				free(trptr);
				free(tpptr);
				close(tdata->tcond);
				return 1;
			}
			else
			{
				//printf ("file %s opened to append package content %s \n", stfile, tpptr );
				syslog(spriority, "file %s opened to append package content %s \n", stfile, tpptr );
				fwrite(tpptr, 1, tpcounter, tfp);
				fflush;
				fclose(tfp);					
				//reset overall package counters and set end string marker
				*tpptr='\0';
				tpcounter=0;
				//read out the fie line by line and send back to client
				tfp = fopen(stfile, "r");
				//we dont check fp as we scuccessfully just wrote to the file
				//read all contents untile end of file
				while (fread(&trchar,1, 1, tfp) == 1) 
				{
					/* append byte by byte */
					// expand buffer if required (possible usecase as you read lines from previous transmissions)
					//compare with pcounter+1 because of end of string character required after end of line
					if ((tpcounter+1) > tpbufs)
					{
						//double size of package buffer and reallocate
						tpbufs=tpbufs*2;
						tpptr = realloc(tpptr, tpbufs * sizeof(char));
						if (tpptr != NULL)
						{
							printf("server: successfully expanded package buffer to size %d \n", tpbufs);
						}
						else
						{
							printf ("packet buffer expansion failed\n");
							syslog(epriority, "packet buffer expansion failed\n");
							mc = pthread_mutex_unlock(&fmutex);
							fclose(tfp);
							free(trptr);
							free(tpptr);
							close(tdata->tcond);
							return 1;
						}
					}
					//add character just read from file to buffer
					*(tpptr + tpcounter)=trchar;
					tpcounter++;
					//if end of line is reached, add end of string character, and send to client
					//them reset counter to zero and buffer string to empty string "" (but size of buffer remains) 
					if (trchar=='\n')
					{
						//end of line reached, finish and send package, then reset buffer
						*(tpptr + tpcounter)='\0';
						//printf("server: send back complete package line retrieved from file '%s'\n",pptr);
						send(tdata->tcond,tpptr,tpcounter,0);
						*tpptr='\0';
						tpcounter=0;	
					}
					
				}
				fclose(tfp);
				*tpptr='\0';
				tpcounter=0;
				mc = pthread_mutex_unlock(&fmutex);
				if (mc != 0)
				{
					printf ("file write mutex unlock not successfull, exit thread\n");
					syslog(epriority, "file write mutex unlock not successfull, exit thread\n");
					free(trptr);
					free(tpptr);
					close(tdata->tcond);
					return 1;	
				}
				else
				{
					printf ("file write mutex unlock successfull\n");
					syslog(epriority, "file write mutex unlock successfull\n");
				}
			}
		}
	}
	if (tnumbytes==0)
	{
	printf ("Closed connection from %s\n", tdata->ts);
	syslog(spriority, "Closed connection from %s\n", tdata->ts);
	}
	else
	{
	printf ("server: receive failed with return %d \n", tnumbytes);
	syslog(epriority, "server: receive failed with return %d \n", tnumbytes);
	free(trptr);
	free(tpptr);
	close(tdata->tcond);
	return 1;
	}
	//finish connection, free buffers
	free(trptr);
	free(tpptr);
	close(tdata->tcond);
	return 0;
 
}
static void *timecon (void *arg)
{
	FILE *tfp;
	int tmc;
	char tis[100];
	int tcounter=0;
	double tdiff;
	time_t base;
	time_t next;
	struct tm *timeptr;
	base = time(NULL);

	while (nomore_connect==false)
	{
		
		//sleep for 10 ms, then take time stamp, if 10s have elapsed, lock mutex and write timestamp to file as string
		sleep(0.01);
		next = time(NULL);
		tdiff=difftime(next,base);
		if(tdiff>=10)
		{
			base=next;	
			timeptr = localtime(&base);
			strftime(tis,sizeof(tis),"timestamp:%Y %m %d %H %M %S\n", timeptr);
			printf("%s",tis);
			for (tcounter=0;tcounter<100;tcounter++)
			{
				if (tis[tcounter]=='\0')
				{
					break;
				}
			}
			tmc = pthread_mutex_lock(&fmutex);
			if (tmc != 0)
			{
				printf ("file write mutex lock not successfull, exit thread\n");
				syslog(epriority, "file write mutex lock not successfull, exit thread\n");
				return 1;	
			}
			else
			{
				printf ("file write mutex lock successfull\n");
				syslog(epriority, "file write mutex lock successfull\n");
			}
			if ((tfp = fopen(stfile, "a+")) == NULL)
			{
				printf ("path does not exist or is locked so file cannot be created or appended\n");
				syslog(epriority, "path does not exist or is locked so file cannot be created or appended\n");
				tmc = pthread_mutex_unlock(&fmutex);
				return 1;
			}
			else
			{
				//printf ("file opened to append time\n");
				syslog(spriority, "file opened to append time\n");
				fwrite(tis,sizeof(char),tcounter,tfp);
				fflush;
				fclose(tfp);					
				tmc = pthread_mutex_unlock(&fmutex);
				if (tmc != 0)
				{
					printf ("file write mutex unlock not successfull, exit thread\n");
					syslog(epriority, "file write mutex unlock not successfull, exit thread\n");
					return 1;	
				}
				else
				{
					printf ("file write mutex unlock successfull\n");
					syslog(epriority, "file write mutex unlock successfull\n");
				}
			}
		}
	}
	return 0;
 
}
//start timer thread
int timer_thread_start()
{
	timer_thread_pointer=(pthread_t *) malloc(sizeof(pthread_t));
	if ( timer_thread_pointer == NULL ) 
	{
		printf("Memory allocation failure for thread data structure\n");
		return 1;
	} 
	else 
	{
		int rc = pthread_create(timer_thread_pointer,NULL, timecon, NULL);
		if( rc != 0 ) 
		{
			//print error message, set error flag
			printf("Attempt to create timer thread failed with %d\n",rc);
			return 1;
		}
		else
		{
			//all ok
			printf("timer thread created ok\n");
			return 0;
		}
	}
}


//add new struct to end of linked list with data
// s - name of client 
//newfd - connection descriptor

int add_thread_to_list (int ccd, char *cs)
{
	struct threaddata *search=NULL;
	struct threaddata *new=NULL;
	search=head;
	int count = 0;
	new=malloc(sizeof(struct threaddata));
	if ( new == NULL ) 
	{
		printf("Memory allocation failure for thread data structure\n");
		return 1;
	} 
	//fill in data
	strcpy(new->ts,cs);
	new->tcond=ccd;
	printf ("client name %s \n", new->ts);
	printf ("connection descriptor %d \n", new->tcond);
	new->next=NULL;
	if (search != NULL)
	{
		//at least 1 link existing
		count++;
		while (search->next != NULL)
		{
			search=search->next;
		}
		
		search->next=new;
	}
	else
	{
		//empty chain, create first items
		printf("first link in chain\n");
		head=new;
	}
	//initialize thread after allocating memory to thread pointer in struct
	new->thread_pointer = (pthread_t *) malloc(sizeof(pthread_t));
	if ( new->thread_pointer== NULL ) 
	{
		printf("Memory allocation failure for thread memory itself entry\n");
		return 1;
	} 
	else 
	{
		int rc = pthread_create(new->thread_pointer,NULL, runcon, new);
		if( rc != 0 ) 
		{
			//print error message, set error flag
			printf("Attempt to create thread failed with %d\n",rc);
			return 1;
		}
		else
		{
			//all ok
			printf("thread created ok\n");
			return 0;
		}
	}
}

int wait_threads_shut()
{
	int rc = 0;
	int count = 0;
	bool allgood=true;
	struct threaddata *search=NULL;
	pthread_t *shutp=NULL;
	
	//shut down connection threads one by one
	while (head != NULL)
	{
		count++;
		search=head;
		head=head->next;
		shutp=search->thread_pointer;
		printf("Attempt to pthread_join thread number %d\n",count);
		rc = pthread_join(*shutp,NULL);
		if( rc != 0 ) 
		{
			//print error message, set error flag
			printf("Attempt to pthread_join thread number %d with descriptor %d failed with %d\n",count,search->tcond,rc);
			allgood=false;
		}
		else
		{
			//all ok free thread buffer and structure
			printf("Attempt to pthread_join thread number %d with descriptor %d succeeded with %d\n",count,search->tcond,rc);
			free (shutp);
			free (search);
		}
		if (head == NULL)
		{
			printf("end of chain reached\n");
		}
		else
		{
			printf("next thread up to join is number %d with descriptor %d succeeded with %d\n",count,head->tcond,rc);
		}
	}
	//shut down timer thread
	rc = pthread_join(*timer_thread_pointer,NULL);
	if( rc != 0 ) 
	{
		//print error message, set error flag
		printf("Attempt to pthread_join timer thread failed with %d\n",rc);
		allgood=false;
	}
	else
	{
		//all ok free thread buffer and structure
		printf("Attempt to pthread_join timer thread succeeded with %d\n",rc);
	}
	if (allgood==true)
	{
		printf("Shutdown all  threads ok\n");
		return 0;
	}
	else
	{
		printf("Shutdown threads nok\n");
		return 1;
	}
}

//main function
int main (int argc, char * argv[])
{
	//open system logger
	openlog(argv[0], LOG_PID, LOG_USER);
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	char nlcheck='\n';
	int rv;
	char rchar;

	int numbytes;
	int pcounter;  
	//char buf[MAXDATASIZE];

	int    timeout=100; //100ms tiemout for poll 
	struct pollfd fds;
	int    nfds = 1;
	int pollres=0;
	int checkpar=1;
	int pid=-1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	//start as daemon if -d handed over as argument
	if (argc == 2)
	{
	    checkpar=strcmp(argv[1], "-d");
	    if (checkpar == 0)
	    {
	    	printf ("starting as daemon\n");
	    	syslog(spriority, "starting as daemon\n");
	    	daemon_act=true;	
	    }
	}


	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
	    printf ("getaddrinfo: %s\n", gai_strerror(rv));
	    syslog(epriority, "getaddrinfo: %s\n", gai_strerror(rv));
	    closelog();
	    return 1;
	}

	 // loop through all the results in servinfo and bind to the first we can
	 for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		        p->ai_protocol)) == -1) {
		    printf ("error server: socket");
	    	    syslog(epriority, "error server: socket");
		    //closelog();
		    continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		        sizeof(int)) == -1) {
		    printf ("error setsockopt");
	    	    syslog(epriority, "error setsockopt");
		    closelog();
		    exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		    close(sockfd);
		    printf ("error server: bind");
	    	    syslog(epriority, "error server: bind");
		    continue;
		}
		break;
	    }


	freeaddrinfo(servinfo); // all done with this structure, necessary info is on p

	if (p == NULL)  {
		printf ("server: failed to bind\n");
		syslog(epriority, "server: failed to bind\n");
		closelog();
		exit(1);
	}
	if (daemon_act==true)
	{
		pid=fork();
		if (pid < 0)
		{
			exit(EXIT_FAILURE);
		}
		if (pid >0)
		{
			//parent process
			//shut down
			closelog();
			exit(0);
		} 
		if (pid == 0)
		{
			//child process turn it into daemon
			setsid();
	    		chdir("/");
	    		stdin = fopen("/dev/null", "r");
			stdout = fopen("/dev/null", "w+");
			stderr = fopen("/dev/null", "w+");
		}
	}
	if (listen(sockfd, BACKLOG) == -1) {
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf ("server: sigaction setup failed\n");
		syslog(epriority, "server: sigaction setup failed\n");
		closelog();
		exit(1);
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		printf ("server: sigaction setup failed\n");
		syslog(epriority, "server: sigaction setup failed\n");
		closelog();
		exit(1);
	}    
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		printf ("server: sigaction setup failed\n");
		syslog(epriority, "server: sigaction setup failed\n");
		closelog();
		exit(1);
	}
	//intialize file (truncate or create)
	if ((fp = fopen(stfile, "w+")) == NULL)
	{
		printf ("path does not exist or is locked so file cannot be created or truncated\n");
		syslog(epriority, "path does not exist or is locked so file cannot be created or truncated\n");
		closelog();
		return 1;
	}
	else
	{
		printf ("file %s created/truncated \n", stfile );
		syslog(spriority, "file %s created/truncated \n", stfile );
		fclose(fp);
	}
	timer_thread_start ();
	//printf("server: waiting for connections...\n");
	while(nomore_connect==false) 
	{  // main accept() loop
		printf("server: wait for next connection on poll\n");
		fds.fd = sockfd;
		fds.events = POLLIN;
		//printf("Waiting on poll()...\n");
		pollres = 0; 
		while ((pollres <= 0) && (nomore_connect==false))
		{
			pollres = poll(&fds, nfds, timeout);
		}
		if (nomore_connect==true)
		{
		    if (pollres > 0)
		    {
		    	printf("start shutdown, disregard connection requests waiting in queue \n");
		    }
		    continue;
		}
		sin_size = sizeof their_addr;
		//printf ("before call accept\n");
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) 
		{
		    printf ("server: accept failed\n");
		    syslog(epriority, "server: accept failed\n");
		    continue;
		}
		inet_ntop(their_addr.ss_family,
		    get_in_addr((struct sockaddr *)&their_addr),
		    s, sizeof s);
		//printf ("before start thread\n");
		if(add_thread_to_list (new_fd, s)!=0)
		{
			//close connection if thrread setup for handling not successful
			close(new_fd);
		}   
	}
        //shutdown trigger receiver, wait for threads to end and then clean up  
	if(nomore_connect==true)
	{
		//wait for all connection operations to terminate
		wait_threads_shut();
		//close open socket
		close(sockfd); 	
		//remove temp file
		remove(stfile);
	}
	closelog();
	return 0;
}











