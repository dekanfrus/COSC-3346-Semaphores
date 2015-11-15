#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <list>
#include <iostream>

#define MAX_THREADS 512;

using namespace std;

buffer_item buffer[BUFFER_SIZE];

sem_t mutex, full, empty;
sem_init(&mutex, 0, 1);
sem_init(&full, 0, 0);
sem_init(&empty, 0, BUFFER_SIZE);

pthread_mutex_t display;
pthread_mutex_init(&display, NULL);

int mainSleep, threadSleep = 0;
int producerThreads, consumerThreads = 0;
int producePtr, consumePtr;
int count, maxCount, minCount = 0;
int produceTid[MAX_THREADS];
int consumeTid[MAX_THREADS];

list<int> pThread, cThread;

string snapshot = "";

bool displaySnapshot = false;
bool run = true;

int buffer_remove_item(buffer_item *item)
{
	int consumedValue;

	sem_wait(&full);

	if (count == 0)
	{
		minCount++;
		if (displaySnapshot == 1)
		{
			pthread_mutex_lock(&display);
			cout << "All buffers empty. Consumer " << *item << " waits." << endl;
			pthread_mutex_unlock(&display);
		}

		return -1;
	}
	consumedValue = buffer[consumePtr];
	consumePtr = (consumePtr + 1) % BUFFER_SIZE;
	count--;

	sem_post(&empty);

	return consumedValue;
}


int buffer_insert_item(buffer_item item)
{
	sem_wait(&empty);
	if (count >= BUFFER_SIZE)
	{
		maxCount++;
		return 0;
	}

	buffer[producePtr] = item;
	writePtr = (producePtr + 1) % BUFFER_SIZE;

	count++;
	sem_post(&full);
}

bool getPrime(number);
{
	bool prime = false;

	// Check number for prime. Set the bool accordingly

	if (prime)
	{
		return true;
	}
	else if (!prime)
	{
		return false;
	}
}
void dispBuf()
{


}

void *consume(void *ctid)
{
	buffer_item number;

	while (run == true)
	{
		sem_wait(&full);
		sem_wait(&mutex);

		number = buffer_remove_item(ctid);

		if (displaySnapshot == true)
		{
			pthread_mutex_lock(&display);
			cout << "Consume " << tid << " reads " << numConsume;
			if (getPrime(number))
			{
				cout << "* * * PRIME * * *" << endl;
			}
			else
			{
				cout << endl;
			}

			dispBuf();
			pthread_mutex_unlocked(&display);
		}

		sem_post(&mutex);
		sem_post(&full);

		cThread.push[ctid];

		usleep(threadSleep);
	}
	


	pthread_exit(0);
}

void *produce(void *ptid)
{
	buffer_item number;
	int randomNum, tid;

	tid = *ptid;

	while (run == true)
	{
		number = rand() % 500;
		sem_wait(&mutex);

		buffer_insert_item(number);

		sem_post(&mutex);
		sem_post(&full);

		if (displaySnapshot == true)
		{
			pthread_mutex_lock(&display);
			cout << "Producer " << tid << " writes " << number << endl;
			dispBuf();
			pthread_mutex_unlock(&display);
		}

		pThread.push[tid];
		usleep(threadSleep);
		
	}

	pthread_exit(0);
}

void dispSum

int main(int argc, char *argv[])
{
	pthread_t threads[MAX_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int i, j = 0;
	

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

	for (i; i < consumerThreads; i++)
	{
		cThread.push_back(0);
		consumeTid[i] = i;
		pthread_create(&threads[i], &attr, consume, (void *)&consumeTid[i]);
	}

	for (j; j < producerThreads; j++, i++)
	{
		pThread.push_back(0);
		produceTid[j] = j;
		pthread_create(&threads[i], &attr, produce, (void *)&produceTid[j]);
	}

	usleep(mainSleep);

	run = false;

	return 0;

}