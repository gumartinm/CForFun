#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/file.h>
#include <stdarg.h>
#include <time.h>
#include "locks.h"



pthread_mutex_t gateMutex;
pthread_cond_t gateBroadCast;
bool isGateOpen = false;
int threadsNumber = 20;


struct sigaction sigintAction;  /*Stores the init SIGINT sigaction value*/
char *fileName = "/tmp/locks";






int main (int argc, char *argv[]) 
{
	int c;                      /*Getopt parameter*/
	/*Default values*/
    struct sigaction sa;        /*sig actions values*/
    bool isThread = false;
	
	opterr = 0;
	while ((c = getopt (argc, argv, "p:tf:")) != -1) {
		switch (c) {
        case 'p':
            fileName = optarg;
            break;
		case 't':
            threadsNumber = atoi(optarg);
			break;
		case 'f':
            isThread = true;
			break;
		case '?':
			if ((optopt == 'p') || (optopt == 't') || (optopt == 'f'))
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Invalid option '-%c'.\n", optopt);
			else
				fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
			return -1;
		default:
			abort ();
		}
	}

	/*This program does not admit options*/
	if (optind < argc) {
		fprintf (stderr, "This program does not admit options just argument elements with their values.\n");
		return -1;
	}
	

    /* If running from console, user may finish this process using SIGINT (Ctrl-C)*/
    /* Check to make sure that the shell has not set up an initial action of SIG_IGN before I establish my own signal handler.
     * As seen on http://www.gnu.org/software/libc/manual/html_node/Initial-Signal-Actions.html#Initial-Signal-Actions
     */
    memset (&sa, 0, sizeof(sa));
    memset (&sigintAction, 0, sizeof(sigaction));
    if (sigaction (SIGINT, NULL, &sa) < 0) {
        fprintf (stderr, "%s: %s\n", "SIGINT retrieve current signal handler failed:", strerror(errno));

        return -1;
    }

    if (sa.sa_handler != SIG_IGN) {
        /* Save the current SIGINT sigaction value. We use it to restore SIGINT handler in my custom SIGINT handler.*/
        memcpy (&sigintAction, &sa, sizeof(sigaction));

        sa.sa_handler = &sigint_handler;
        sa.sa_flags = SA_RESTART;
        if (sigemptyset(&sa.sa_mask) < 0) {
            fprintf (stderr, "%s: %s\n", "SIGINT empty mask", strerror(errno));

            return -1;
        }
        if (sigaction(SIGINT, &sa, NULL) < 0) {
            fprintf (stderr, "%s: %s\n", "SIGINT set signal handler failed", strerror(errno));

            return -1;
        }
    }

	
	if (main_process () < 0)
		return -1;
	
	return 0;
}

int main_process ()
{
    int createIndex;
    int joinIndex;
	pthread_t threadIds [threadsNumber];                 /*Threads identifier numbers*/
	
    pthread_mutex_init(&gateMutex, NULL);
    pthread_cond_init(&gateBroadCast, NULL);
    closeGate();


    for (createIndex = 0; createIndex < threadsNumber; createIndex++) {
        print_with_date (stdout, "Thread %d created\n", createIndex);
    	if (pthread_create (&threadIds[createIndex], NULL, &thread_lock, (void *) createIndex) != 0 ) {
            print_with_date (stderr, "Thread %d creation failed\n", createIndex, strerror(errno));
            break;
    	}
    }

    sleep(5);
    openGate();

    for (joinIndex = createIndex; joinIndex --> 0; ) {
        if (pthread_join(threadIds[joinIndex], NULL) != 0) {
            print_with_date (stderr, "Thread %d join error\n", joinIndex, strerror(errno));   
        }
    }
    
    return 0;
}

void *thread_lock(void * arg) {
    int threadNumber;
    int fd;
    int flockErr;

    threadNumber = (int) arg;

    gate();

    fd = open(fileName, O_CREAT | O_RDWR, 0664);
    if (fd == -1) {
        print_with_date (stderr, "Thread %d, open file error", threadNumber, strerror(errno));
    }
   
    print_with_date (stdout, "Thread %d: before lock\n", threadNumber);
    do {
        flockErr = flock(fd, LOCK_EX);
    } while(flockErr == -1 && errno == EINTR);


    if (flockErr == -1) {
        print_with_date (stderr, "Thread %d: flock error", threadNumber, strerror(errno));
    }
    print_with_date (stdout, "Thread %d: after lock\n", threadNumber);

    sleep(5);

    print_with_date (stdout, "Thread %d: before release lock\n", threadNumber);
    do {
        flockErr = flock(fd, LOCK_UN);
    } while(flockErr == -1 && errno == EINTR);
    print_with_date (stdout, "Thread %d: after release lock\n", threadNumber);

    close (fd);

    pthread_exit(0);
}

int print_with_date(FILE *stream, const char *format, ...)
{
    va_list arg;
    int done;
    time_t rawtime;
    char buff[50];
    char dateformatter[100] = "%s: ";
    struct tm timeinfo;

    time ( &rawtime);
    localtime_r ( &rawtime, &timeinfo);
    strftime(buff, 50, "%Y-%m-%d %H:%M:%S", &timeinfo);

    va_start (arg, format);
    strcat(dateformatter, format);
    done = fprintf (stream, dateformatter, buff, arg);

    if (done < 0) {
        fflush (stream);
        return done;
    } else {
        done = fflush (stream);
        return done;
    }    
}

void gate() {
    
    pthread_mutex_lock(&gateMutex);

    while(!isGateOpen) {
        pthread_cond_wait(&gateBroadCast, &gateMutex);
    }

    pthread_mutex_unlock(&gateMutex);
}

void openGate() {
    pthread_mutex_lock(&gateMutex);

    isGateOpen = true;
    pthread_cond_broadcast(&gateBroadCast);

    pthread_mutex_unlock(&gateMutex);
}

void closeGate() {
    pthread_mutex_lock(&gateMutex);

    isGateOpen = false;

    pthread_mutex_unlock(&gateMutex);
}

void sigint_handler(int sig)
{
    /*TODO: kill child processes, finish threads and release allocate memory*/
    /* From http://www.cons.org/cracauer/sigint.html
     * Since a shellscript may in turn be called by a shellscript, you need to make sure that you properly
     * communicate the discontinue intention to the calling program. WIFSIGNALED(status) and WTERMSIG(status)
     * tell whether the child says "I exited on SIGINT". These values are used by the shell to discontinue
     * the whole shell script in execution. If I use exit the shell has no way to know the user pressed Ctrl-C
     * in order to stop the shell script in execution. This has just meaning when this program is executed in a shell script
     * but because I do not know how the user is going to use it I must always finish every SIGINT handler in this way.
     * So from a handler for SIGINT I must always finish with kill(SIGINT, SIG_DFL) (default kills the application)
     */
    if (sigaction(SIGINT, &sigintAction, NULL) < 0) {
        exit (EXIT_FAILURE);
    }
    kill(getpid(), SIGINT);
}


