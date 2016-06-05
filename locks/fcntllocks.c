#include <stdlib.h>
#include <stdio.h>
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
#include "fcntllocks.h"



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
	
	opterr = 0;
	while ((c = getopt (argc, argv, "p:t:f:")) != -1) {
		switch (c) {
        case 'p':
            fileName = optarg;
            break;
		case 't':
            threadsNumber = atoi(optarg);
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

    if (threadsNumber > 0) {
	    return test_threads_concurrency();
    } else {
        return test_require_user();
    }
	
	return 0;
}

int test_threads_concurrency()
{
    int returnValue = 0;
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
            returnValue = -1;
            break;
    	}
    }

    sleep(5);
    openGate();

    for (joinIndex = createIndex; joinIndex --> 0; ) {
        if (pthread_join(threadIds[joinIndex], NULL) != 0) {
            print_with_date (stderr, "Thread %d join error\n", joinIndex, strerror(errno));
            returnValue = -1;
        }
    }
    
    return returnValue;
}

void *thread_lock(void * arg)
{
    struct flock fl;
    int threadNumber;
    int fd;
    int flockErr;

    threadNumber = (int) arg;


    fd = open(fileName, O_CREAT | O_RDWR, 0664);
    if (fd == -1) {
        goto end;
        print_with_date (stderr, "Thread %d, open file error\n", threadNumber, strerror(errno));
    }


    gate();

   
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();
    print_with_date (stdout, "Thread %d: before lock\n", threadNumber);
    do {
        flockErr = fcntl(fd, F_SETLKW, &fl);
    } while(flockErr == -1 && errno == EINTR);

    if (flockErr == -1) {
        print_with_date (stderr, "Thread %d: F_WRLCK/F_SETLKW, exclusive lock error", threadNumber, strerror(errno));
        goto end;
    }
    print_with_date (stdout, "Thread %d: after lock\n", threadNumber);


    sleep(5);


    print_with_date(stdout, "Thread %d: before release lock\n", threadNumber);
    fl.l_type   = F_UNLCK;
    do {
        fcntl(fd, F_SETLK, &fl);
    } while(flockErr == -1 && errno == EINTR);

    if (flockErr == -1) {
        print_with_date(stderr, "Thread %d: F_UNLCK unlock error", threadNumber, strerror(errno));
        goto end;
    }
    print_with_date(stdout, "Thread %d: after release lock\n", threadNumber);


end:
    close (fd);
    pthread_exit(0);
}

int test_require_user() {
    struct flock fl;
    int returnValue = -1;
    int fd;
    int flockErr;
      
      
    fd = open(fileName, O_CREAT | O_RDWR, 0664);
    if (fd == -1) {
        print_with_date(stderr, "Open file error\n", strerror(errno));
        goto end;
    }


    fprintf(stdout, "Press ENTER key for locking file: %s\n", fileName);
    fflush(stdout);
    getchar();

 
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;
    fl.l_pid    = getpid();
    print_with_date (stdout, "Before lock\n");
    do {
        flockErr = fcntl(fd, F_SETLKW, &fl);
    } while(flockErr == -1 && errno == EINTR);

    if (flockErr == -1) {
        print_with_date (stderr, "F_WRLCK/F_SETLKW, exclusive lock error", strerror(errno));
        goto end;
    }
    print_with_date (stdout, "After lock\n");


    fprintf(stdout, "Press ENTER key for unlocking file: %s\n", fileName);
    fflush(stdout);
    getchar();


    print_with_date(stdout, "Before release lock\n");
    fl.l_type   = F_UNLCK;
    do {
        fcntl(fd, F_SETLK, &fl);
    } while(flockErr == -1 && errno == EINTR);

    if (flockErr == -1) {
        print_with_date(stderr, "F_UNLCK unlock error", strerror(errno));
        goto end;
    }
    print_with_date(stdout, "After release lock\n");


    returnValue = 0;
end:
    close (fd);
    return returnValue;
}

// TODO: it doesn't show the strerror :(
int print_with_date(FILE *stream, const char *format, ...) {
    va_list arg;
    int done;
    time_t rawtime;
    char buff[100];
    char msg[50];
    char dateformatter[100] = "%s: ";
    struct tm timeinfo;

    time ( &rawtime);
    localtime_r ( &rawtime, &timeinfo);
    strftime(buff, 50, "%Y-%m-%d %H:%M:%S", &timeinfo);

    va_start (arg, format);
    vsnprintf (msg , 50, format, arg);

    strcat(dateformatter, msg);
    done = fprintf (stream, dateformatter, buff, msg);
    va_end (arg);

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
    if (sigaction(SIGINT, &sigintAction, NULL) < 0) {
        exit (EXIT_FAILURE);
    }
    kill(getpid(), SIGINT);
}


