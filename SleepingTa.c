#include <pthread.h> //Create POSIX threads.
#include <time.h> //Wait for a random time.
#include <unistd.h> //Thread calls sleep for specified number of seconds.
#include <semaphore.h> //To create semaphores
#include <stdlib.h>
#include <stdio.h> //Input Output
#define WR 3
pthread_t *Students; //N threads running as Students.
pthread_t TA; //Separate Thread for TA.

sem_t TA_sleep; //A semaphore to signal and wait TA's sleep.
sem_t student_leave; //A semaphore to signal and wait for student to fully leave
sem_t TA_available; //A semaphore to allow only one person to go through TA
sem_t chairs_sem; //A semaphore to allow one person to sit in chair after someone leaves

pthread_mutex_t mutex; 

void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	srand(time(NULL)); //for random time
	int number_of_students; //a variable taken from the user to create
				//student threads. Default is 5 student threads.

	//Initializing Mutex Lock and Semaphores.
 	if (pthread_mutex_init(&mutex, NULL) != 0) {
        	perror("pthread_mutex_init");
       	 	exit(EXIT_FAILURE);
   	 }

    	if (sem_init(&TA_sleep, 0, 0) != 0) {
       		perror("sem_init TA_sleep");
       	 	exit(EXIT_FAILURE);
   	 }
   	 
    	if (sem_init(&TA_available, 0, 0) != 0) {
       		perror("sem_init TA_available");
       		exit(EXIT_FAILURE);
    	}
    	
   	if (sem_init(&student_leave, 0, 0) != 0) {
        	perror("sem_init student_leave");
        	exit(EXIT_FAILURE);
   	 }
   	 
  	if (sem_init(&chairs_sem, 0, WR) != 0) {
        	perror("sem_init chairs_sem");
        	exit(EXIT_FAILURE);
    	}
    	
	if(argc<2)
	{	
		printf("Number of Students not specified. Using default (5) students.\n");
		number_of_students = 5;
	}
	else
	{
		number_of_students = atoi(argv[1]);
		printf("Number of Students specified. Creating %d threads.\n", number_of_students);
	}

	int id[number_of_students];

	//Allocate memory for Students
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	pthread_create(&TA, NULL, TA_Activity, NULL);
	
	//Creating one TA thread and N Student threads.
	for (int i = 0; i < number_of_students; i++) {
		id[i] = i;
		pthread_create(&Students[i], NULL, Student_Activity, &id[i]);
	}

	//Waiting for TA thread and N Student threads.
	for (int i = 0; i < number_of_students; i++) {
		pthread_join(Students[i], NULL);
	}

	//exits TA_activity after it is finished
	pthread_cancel(TA);

	//Destroy semaphores and mutex
	pthread_mutex_destroy(&mutex);
	
	sem_destroy(&TA_sleep);
	sem_destroy(&student_leave);		
	sem_destroy(&TA_available);
	sem_destroy(&chairs_sem);


	free(Students); //Free allocated memory
	return 0;
}

void *TA_Activity()
{
	while(1){//if chairs are empty, break the loop.
		printf("TA is sleeping.\n");
		sem_wait(&TA_sleep); //TA is currently sleeping.
		sem_post(&TA_available);
		int sleep_time = rand() % 6 + 5;
		sleep(sleep_time);//Student is being helped by TA
		sem_post(&student_leave);	
		printf("TA is done helping student.\n");
	}
}

void *Student_Activity(void *threadID)
{
	int id = *((int *)threadID);

	pthread_mutex_lock(&mutex);
	
	//For sitting in Waiting Room 
	if(sem_trywait(&chairs_sem) != 0) {
		printf("Student %d couldn't find a seat and has left. Student will return later.\n", id);
		pthread_mutex_unlock(&mutex);
		sem_wait(&chairs_sem);
		
	}
	else{
		pthread_mutex_unlock(&mutex);
	}
	
	//For getting help from TA
	pthread_mutex_lock(&mutex);
	printf("Student %d takes a chair\n", id);	
	sem_post(&TA_sleep);
	pthread_mutex_unlock(&mutex);
	
	sem_wait(&TA_available);
	printf("Student %d wakes up TA and heads into office.\n", id);
	sem_post(&chairs_sem);
	sem_wait(&student_leave);

	pthread_exit(NULL);
}
