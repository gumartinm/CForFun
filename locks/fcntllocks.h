


void gate();

void openGate();

void closeGate();

int print_with_date(FILE *stream, const char *format, ...);

/****************************************************************************************/
/* This method is used by pthread_create                                                */
/*                                                                                      */
/* INPUT PARAMETER: socket file descriptor                                              */
/* RETURNS: void                                                                        */
/****************************************************************************************/
void *thread_lock(void *arg);


int test_threads_concurrency();

int test_require_user();




/****************************************************************************************/
/* This method is used by pthread_create                                                */
/*                                                                                      */
/* INPUT PARAMETER: socket file descriptor                                              */
/* RETURNS: void                                                                        */
/****************************************************************************************/
void sigint_handler();



