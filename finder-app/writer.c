#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>


FILE *fp;
const int cargc=2;
int aargc=0;
const int spriority=(LOG_DEBUG|LOG_USER);
const int epriority=(LOG_ERR|LOG_USER);


int main (int argc, char * argv[])
{
//prepare variables for logging
const char *wst=argv[2];
const char *fpath=argv[1];

//open system logger
openlog(argv[0], LOG_PID, LOG_USER);

//check if number of arguments otherwise, abort
aargc=argc-1;
if (aargc != cargc)
{
//	printf ("wrong number of arguments %d, expected was %d, with first argument the filepath and second argument the string to write to the file. \n", aargc, cargc);
	syslog(epriority, "wrong number of arguments %d, expected was %d, with first argument the filepath and second argument the string to write to the file. \n", aargc, cargc);
	closelog();
	return 1;
}

//log message to syslog showing the arguments used now that arguments are checked
//printf ("writing %s to %s  \n", wst, fpath);
syslog(spriority, "Writing %s to %s", wst, fpath);



//try to open and truncate / newly create file with path given 
if ((fp = fopen(argv[1], "wb")) == NULL)
	{
	printf ("path does not exist or is locked so file cannot be created or truncated\n");
	syslog(epriority, "path does not exist or is locked so file cannot be created or truncated\n");
	closelog();
	return 1;
	}
else
	{
//	printf ("file exists and was truncated or was created anew\n");
	fputs(argv[2], fp);
	fclose(fp);
	closelog();
	return 0;
	}

}
