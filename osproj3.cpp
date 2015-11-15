#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

buffer_item buffer[BUFFER_SIZE];

sem_t mutex, full, empty;
sem_init(&mutex, 0, 1);
sem_init(&full, 0, 0);
sem_init(&empty, 0, BUFFER_SIZE);

pthread_mutex_t display;
pthread_mutex_init(&display, NULL);

int mainSleep, threadSleep = 0;
int producerThreads, consumerThreads = 0;
int writePtr, readPtr;
int maxCount, minCount = 0;

string snapshot = "";

bool displaySnapshot = false;

void dispBuf()
{

}

void *consume(void *num)
{

}

void *produce(void *num)
{
	buffer_item number;
	int randomNum, tid;

	tid = *num;

	do
	{
		number = rand() % 500;
		sem_wait(&mutex);

		buffer_insert_item(number);

		sem_wait(&mutex);
		sem_wait(&full);

		if (displaySnapshot == true)
		{
			pthread_mutex_lock(&display);
			cout << "Producer " << tid << " produces " << num << endl;
			dispBuf();
			pthread_mutex_unlock(&display);
		}
		usleep(threadSleep);
		// Increment the number of threads that produced and its thread id
	} while (true);

	pthread_exit(0);
}

void dispSu

int main(int argc, char *argv[])
{
	

	if (argc != 5)
	{
		cout << "Not enough arguments were supplied." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
	}

	mainSleep = atoi(argv[1]);
	threadSleep = atoi(argv[2]);
	producerThreads = atoi(argv[3]);
	consumerThreads = atoi(argv[4]);
	snapshot = atoi(argv[5]);

	if (snapshot != "yes" || snapshot != "Yes" || snapshot != "No" || snapshot != "no")
	{
		cout << "Invalid argument " << argv[5] << " ." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
	}
	else if (snapshot == "yes" || snapshot == "Yes")
	{
		displaySnapshot = true;
	}

	else if (snapshot != "No" || snapshot != "no")
	{
		displaySnapshot = false
	}

	if (mainSleep <= 0 || threadSleep <= 0 || producerThreads <= 0 || consumerThreads <= 0)
	{
		cout << "Invalid arguments.  Arguments must be a positive, non-zero integer." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
	}

	

	for (i = 0; i < producerThreads; i++)
	{
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, produce, NULL);
	}

	for (ii = 0; ii < consumerThreads; ii++)
	{
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, consume, NULL);
	}

	sleep(mainSleep);

	return 0;

}