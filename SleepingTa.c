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

sem_t TA_sleep; //A semaphore to signal and wait TA's sleep.
sem_t NextStudent; //A semaphore to signal and wait for TA's next student.

//To lock and unlock variable ChairsCount to increment and decrement its value.
pthread_mutex_t chair_mutex; 
pthread_mutex_t ta_mutex;

void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	srand(time(NULL));
	int number_of_students; //a variable taken from the user to create
				//student threads. Default is 5 student threads.

	//Initializing Mutex Lock and Semaphores.
	pthread_mutex_init(&chair_mutex, NULL);
	pthread_mutex_init(&ta_mutex, NULL);
	sem_init(&TA_sleep, 0, 0);
	sem_init(&NextStudent, 0, 0);

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

	pthread_cancel(TA);

	//Destroy semaphores and mutex
	pthread_mutex_destroy(&chair_mutex);
	pthread_mutex_destroy(&ta_mutex);
	sem_destroy(&TA_sleep);
	sem_destroy(&NextStudent);

	free(Students); //Free allocated memory
	return 0;
}

void *TA_Activity()
{
	while(1){//if chairs are empty, break the loop.
		printf("TA is sleeping.\n");
		sem_wait(&TA_sleep); //TA is currently sleeping.
		if(ChairsCount > 0){//TA gets next student on chair.
			ChairsCount--;
			printf("TA is currently helping student.\n");
			int sleep_time = rand() % 6 + 5;
			sleep(sleep_time);//Student is being helped by TA
			printf("TA is done helping student.\n");
			printf("TA is done helping student.\n");
			sem_post(&NextStudent);
		}
	}
	pthread_exit(NULL);

}

void *Student_Activity(void *threadID)
{
	int id = *((int *)threadID);

	//For sitting in Waiting Room 
	//For sitting in Waiting Room 
	while(1){
		pthread_mutex_lock(&chair_mutex);// lock
		if(ChairsCount < WR) {
			ChairsCount++; //takes the chair
			printf("Student %d takes a chair.\n", id);
			pthread_mutex_unlock(&chair_mutex);
			break;
		}
		else{
			printf("Student %d couldn't find a seat and has left. Student will return later.\n", id);
			pthread_mutex_unlock(&chair_mutex);
			sleep(10); //student couldn't find chair, leaves and comes back
		}
	}

	//For getting help from TA
	while(1){
		int sleep_time = rand() % 6 + 5;
		printf("Student %d is waiting in chair for %d min(s).\n", id, sleep_time);
		sleep(sleep_time);
		if (pthread_mutex_trylock(&ta_mutex) == 0){
			printf("Student %d wakes up TA and heads into office.\n", id);
			sem_post(&TA_sleep);
			sem_wait(&NextStudent);
			pthread_mutex_unlock(&ta_mutex);
			printf("Student %d has gotten help and will now leave.\n", id);
			break;
		}
		else{
			printf("Student %d tried to wake up TA, but TA was helping another student.\n", id);
		}
	}

	pthread_exit(NULL);
}
