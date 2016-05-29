


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
void *thread_lock (void *arg);


/****************************************************************************************/
/* This method is used by pthread_create                                                */
/*                                                                                      */
/* INPUT PARAMETER: socket file descriptor                                              */
/* RETURNS: int                                                                         */
/****************************************************************************************/
int main_process ();




/****************************************************************************************/
/* This method is used by pthread_create                                                */
/*                                                                                      */
/* INPUT PARAMETER: socket file descriptor                                              */
/* RETURNS: void                                                                        */
/****************************************************************************************/
void sigint_handler();



