#include <pthread.h> //Create POSIX threads.
#include <time.h> //Wait for a random time.
#include <unistd.h> //Thread calls sleep for specified number of seconds.
#include <semaphore.h> //To create semaphores
#include <stdlib.h>
#include <stdio.h> //Input Output
#define WR 3
pthread_t *Students; //N threads running as Students.
pthread_t TA; //Separate Thread for TA.
int ChairsCount = 0;
int CurrentIndex = 0;

sem_t TA_sleep; //A semaphore to signal and wait TA's sleep.
sem_t WaitingRoom[3]; //An array of 3 semaphores to signal and wait chair to wait for the TA.
sem_t NextStudent; //A semaphore to signal and wait for TA's next student.

//To lock and unlock variable ChairsCount to increment and decrement its value.
pthread_mutex_t mutex; 

void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	int number_of_students; //a variable taken from the user to create
				//student threads. Default is 5 student threads.
	srand(time(NULL));

	//Initializing Mutex Lock and Semaphores.
	pthread_mutex_init(&mutex, NULL);
	sem_init(&TA_sleep, 0, 1);
	sem_init(&NextStudent, 0, 1);

	for (int i = 1; i <= WR; i++) {
		sem_init(&WaitingRoom[i], 0, 1);
	}	



	if(argc<2)
	{	
		printf("Number of Students not specified. Using default (5) students.\n");
		number_of_students = 5;
	}
	else
	{
		printf("Number of Students specified. Creating %d threads.\n", number_of_students);
		number_of_students = atoi(argv[1]);
	}

	int id[number_of_students];

	//Allocate memory for Students
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	//Creating one TA thread and N Student threads.
	for (int i = 0; i < number_of_students; i++) {
		id[i] = i+1;
		pthread_create(&Students[i], NULL, Student_Activity, &id[i]);
	}

	pthread_create(&TA, NULL, TA_Activity, NULL);

	//Waiting for TA thread and N Student threads.
	for (int i = 0; i < number_of_students; i++) {
		pthread_join(Students[i], NULL);
	}

	pthread_join(TA, NULL);

	
	//Destroy semaphores and mutex
	pthread_mutex_destroy(&mutex);
	sem_destroy(&TA_sleep);
	sem_destroy(&NextStudent);

	for (int i = 0; i < WR; i++) {
		sem_destroy(&WaitingRoom[i]);
	}

	free(Students); //Free allocated memory
	return 0;
}

void *TA_Activity()
{
	do{//if chairs are empty, break the loop.
		sem_wait(&TA_sleep); //TA is currently sleeping.
		pthread_mutex_lock(&mutex);// lock		
		printf("TA ran, this is the Current Index %d\n", CurrentIndex);
		CurrentIndex--;
		pthread_mutex_unlock(&mutex);//unlock
	} while(CurrentIndex > 0);
	pthread_exit(NULL);

}

void *Student_Activity(void *threadID)
{
	CurrentIndex++;//Student needs help from the TA
	sem_post(&TA_sleep); //wake up the TA.
	int id = *((int *)threadID);
	printf("ID %d \n", id);
	pthread_exit(NULL);
}
