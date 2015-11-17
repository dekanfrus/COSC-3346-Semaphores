#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <list>
#include <iostream>
#include <math.h>

#define MAX_THREADS 512
#define TEN_MILLION 10000000

using namespace std;

buffer_item buffer[BUFFER_SIZE];

sem_t mutex;
sem_t full;
sem_t empty;

pthread_mutex_t display;

struct timespec mainSleep;
struct timespec threadSleep;

int producerThreads = 0, consumerThreads = 0;
int producePtr = 1, consumePtr = 1;
int count = 0, maxCount = 0, minCount = 0;
int produceTid[MAX_THREADS];
int consumeTid[MAX_THREADS];
int mSleep = 0, tSleep = 0;

list<int> pThread, cThread;

string snapshot = "";

bool displaySnapshot = false;
bool run = true;

int buffer_remove_item(pthread_t tid)
{
	int consumedValue;

	sem_wait(&full);

	if (count == 0)
	{
		minCount++;
		if (displaySnapshot == 1)
		{
			pthread_mutex_lock(&display);
			cout << "All buffers empty. Consumer " << tid << " waits." << endl << endl;
			pthread_mutex_unlock(&display);
		}
		sem_post(&empty);
		return -1;
	}
	consumedValue = buffer[consumePtr];
	consumePtr = (consumePtr + 1) % BUFFER_SIZE;
	count--;

	sem_post(&empty);

	return consumedValue;
}


bool buffer_insert_item(buffer_item item, pthread_t tid)
{
	sem_wait(&empty);

	if (count >= BUFFER_SIZE)
	{
		maxCount++;
		if (displaySnapshot == 1)
		{
			pthread_mutex_lock(&display);
			cout << "All buffers full. Producer " << tid << " waits." << endl << endl;
			pthread_mutex_unlock(&display);
		}
		sem_post(&full);
		return true;
	}

	buffer[producePtr] = item;
	producePtr = (producePtr + 1) % BUFFER_SIZE;

	count++;
	sem_post(&full);
}

// Code found online
bool getPrime(buffer_item number)
{
	bool prime = true;

	if (number <= 1)
		return false;
	else if (number == 2)
		return true;
	else if (number % 2 == 0)
		return false;
	else
	{
		int divisor = 3;
		double num_d = static_cast<double>(number);
		int upperLimit = static_cast<int>(sqrt(num_d) + 1);

		while (divisor <= upperLimit)
		{
			if (number % divisor == 0)
				prime = false;
			divisor += 2;
		}
		return prime;
	}
}
void dispBuf()
{
	cout << "(buffers occupied: " << count << ")" << endl;
	cout << "Buffers:\t";

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		cout << buffer[i] << "\t";
	}

	cout << endl << "\t\t----\t----\t----\t----\t---" << endl;
	cout << "\t\t";

	if (count == 0)
	{
		for (int j = 0; j < consumePtr; j++)
		{
			cout << "\t";
		}
		cout << "WR";
	}
	else if (consumePtr <= producePtr)
	{
		for (int k = 0; k < consumePtr; k++)
			cout << "\t";
		cout << "R";

		for (int m = consumePtr; m < producePtr; m++)
			cout << "\t";
		cout << "W";
	}
	else
	{
		for (int n = 0; n < producePtr; n++)
			cout << "\t";
		cout << "W";

		for (int p = producePtr; p < consumePtr; p++)
			cout << "\t";
		cout << "R";
	}

	cout << endl;
}

void *consume(void* ctid)
{
	buffer_item number;
	pthread_t self = pthread_self();

	int *pointer;
	int tid;
	pointer = (int *)ctid;
	tid = *pointer;

	do
	{
		sem_wait(&empty);
		//sem_wait(&mutex);

		number = buffer_remove_item(self);

		if (displaySnapshot == true)
		{
			if (number != -1)
			{
				pthread_mutex_lock(&display);
				cout << "Consumer " << self << " reads " << number;
				if (getPrime(number))
				{
					cout << "  * * * PRIME * * *" << endl;
				}
				else
				{
					cout << endl;
				}

				cThread.push_back(self);
				dispBuf();
				pthread_mutex_unlock(&display);
			}
		}

		//sem_post(&mutex);
		sem_post(&full);

		nanosleep(&threadSleep, NULL);
	} while (run = true);



	pthread_exit(0);
}

void *produce(void *ptid)
{
	buffer_item number = 0;

	bool bufferFull;
	int *pointer;
	int tid;
	pointer = (int *)ptid;
	tid = *pointer;
	pthread_t self = pthread_self();

	do
	{

		sem_wait(&full);
		//sem_wait(&mutex);

		number = rand() % 10000 + 1;

		bufferFull = buffer_insert_item(number, self);


		if (displaySnapshot == true)
		{
			if (!bufferFull)
			{
				pthread_mutex_lock(&display);
				pThread.push_back(self);
				cout << "Producer " << self << " writes " << number << endl;
				dispBuf();
				pthread_mutex_unlock(&display);
			}
		}

		//sem_post(&mutex);
		sem_post(&empty);

		nanosleep(&threadSleep, NULL);

	} while (run = true);

	pthread_exit(0);
}

void displayResults()
{
	cout << "PRODUCER / CONSUMER SIMULATION COMPLETE" << endl;
	cout << "=======================================" << endl;
	cout << "Simulation Time:                       " << mSleep << endl;
	cout << "Maximum Thread Sleep Time:             " << tSleep << endl;
	cout << "Number of Producer Threads:            " << producerThreads << endl;
	cout << "Number of Consumer Threads:            " << consumerThreads << endl;
	cout << "Size of Buffer:                        " << BUFFER_SIZE << endl << endl;
	cout << "Total Number of Items Produced:        " << pThread.size() << endl;
	for (int i = 0; i < producerThreads; i++) {
		//cout << "\t Thread " << i + 1 << ":" << "                     " << pThread[i] << endl;
	}
	cout << "Total Number of Items Consumed:        " << cThread.size() << endl;
	cout << endl;
	for (int i = 0; i < consumerThreads; i++) {
		//cout << "\t Thread " << i + 1 << ":" << "                     " << cThread[i] << endl;
	}
	cout << endl;
	cout << "Number Of Items Remaining in Buffer:   " << count << endl;
	cout << "Number Of Times Buffer Was Full:       " << maxCount << endl;
	cout << "Number Of Times Buffer Was Empty:      " << minCount << endl;

}

int initSem()
{
	if (sem_init(&mutex, 0, 1) == -1) {
		cout << "Failed to initialize semaphore" << endl;
		return 0;
	}

	if (sem_init(&full, 0, 1) == -1) {
		cout << "Failed to initialize semaphore" << endl;
		return 0;
	}

	if (sem_init(&empty, 0, BUFFER_SIZE) == -1) {
		cout << "Failed to initialize semaphore" << endl;
		return 0;
	}

	pthread_mutex_init(&display, NULL);
}

int main(int argc, char *argv[])
{
	initSem();

	pthread_t threads[MAX_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	int i = 0, j = 0;


	if (argc != 6)
	{
		cout << "Not enough arguments were supplied." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
		return 0;
	}


	mSleep = atoi(argv[1]);
	tSleep = atoi(argv[2]);

	producerThreads = atoi(argv[3]);
	consumerThreads = atoi(argv[4]);
	snapshot = argv[5];


	if (snapshot == "yes" || snapshot == "Yes")
	{
		displaySnapshot = true;
	}

	else if (snapshot != "No" || snapshot != "no")
	{
		displaySnapshot = false;
	}
	else
	{
		cout << "Invalid argument " << argv[5] << " ." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
		return 0;
	}

	if (mSleep <= 0 || tSleep <= 0 || producerThreads <= 0 || consumerThreads <= 0)
	{
		cout << "Invalid arguments.  Arguments must be a positive, non-zero integer." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
		return 0;
	}
	else
	{
		mainSleep.tv_sec = atoi(argv[1]);
		mainSleep.tv_nsec = TEN_MILLION;

		threadSleep.tv_sec = atoi(argv[2]);
		threadSleep.tv_nsec = TEN_MILLION;
	}

	for (j = 0; j < producerThreads; j++)
	{
		pthread_t tid;
		produceTid[j] = j;
		pthread_create(&tid, &attr, produce, (void *)&produceTid[j]);
	}

	for (i = 0; i < consumerThreads; i++)
	{
		pthread_t tid;
		consumeTid[i] = i;
		pthread_create(&tid, &attr, consume, (void *)&consumeTid[i]);
	}

	nanosleep(&mainSleep, NULL);

	run = false;

	displayResults();

	return 0;

}