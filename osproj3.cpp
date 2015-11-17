//*********************************************************
//
// John Alves
// Operating Systems
// Project #3: Producer Consumer With Semaphores
// November 18, 2015
// Instructor: Dr. Ajay K. Katangur
// 
//*********************************************************

//*********************************************************
//
// Includes and Defines
//
//*********************************************************
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

//*********************************************************
//
// Global variable declarations
//
//*********************************************************

// Buffer to hold random numbers
buffer_item buffer[BUFFER_SIZE];

// Semaphores used to synchronize threads and ensure only
// One access the buffer at a time
sem_t mutex;
sem_t full;
sem_t empty;

// Mutex to lock the displaying of snapshot information
pthread_mutex_t display;

// Structs to hold the sleep times for the main program
// as well as the threads
struct timespec mainSleep;
struct timespec threadSleep;

// Holds the number of threads the user wishes
// to run for both producers and consumers.
// This value comes from the command line.
int producerThreads = 0, consumerThreads = 0;

// Pointers to indicate where in the buffer
// the current operation is taking place.
int producePtr = 0, consumePtr = 0;

// Holds values for when the buffer is empty, full
// and and how many times for each
int count = 0, maxCount = 0, minCount = 0;

// These are used to create the threads and ensure
// the number of threads does not exceed the max
int produceTid[MAX_THREADS];
int consumeTid[MAX_THREADS];

// These store the integer value of time that the
// user wishes the main program to run for, and 
// how long the threads should sleep.
int mSleep = 0, tSleep = 0;

// Integer linked list used for holding the number of times
// a thread successfully produces or consumes from the buffer
list<int> pThread, cThread;

// String to hold the user's decision to show the snapshot
// while the application is running
string snapshot = "";

// Bool values to stop the application from running, and
// whether or not to display the snapshot.
bool displaySnapshot = false;
bool run = true;

//********************************************************************
//
// Buffer Remove Item Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
int buffer_remove_item(pthread_t tid)
{
	int consumedValue;

	// Decrement until 0, then lock/wait/stop
	sem_wait(&full);

	if (count == 0)
	{
		// Increase the number of times the buffer is empty
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
	// Store the value that is consumed from the buffer, then move the pointer
	// to the next location in the buffer.  Decrement count to indicate a value
	// has been consumed and a buffer location is available
	consumedValue = buffer[consumePtr];
	consumePtr = (consumePtr + 1) % BUFFER_SIZE;
	count--;

	sem_post(&empty);

	return consumedValue;
}

//********************************************************************
//
// Buffer Insert Item Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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
// http://www.cplusplus.com/forum/general/1125/
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

//********************************************************************
//
// Display Buffer Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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

//********************************************************************
//
// Consume Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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

		sem_post(&full);

		nanosleep(&threadSleep, NULL);
	} while (run = true);



	pthread_exit(0);
}

//********************************************************************
//
// Produce Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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

		sem_post(&empty);

		nanosleep(&threadSleep, NULL);

	} while (run = true);

	pthread_exit(0);
}

//********************************************************************
//
// Display Results Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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
	/*for (int i = 0; i < producerThreads; i++) {
		cout << "\t Thread " << i + 1 << ":" << "                     " << pThread[i] << endl;
	}*/
	cout << "Total Number of Items Consumed:        " << cThread.size() << endl;
	cout << endl;
	/*for (int i = 0; i < consumerThreads; i++) {
		cout << "\t Thread " << i + 1 << ":" << "                     " << cThread[i] << endl;
	}*/
	cout << endl;
	cout << "Number Of Items Remaining in Buffer:   " << count << endl;
	cout << "Number Of Times Buffer Was Full:       " << maxCount << endl;
	cout << "Number Of Times Buffer Was Empty:      " << minCount << endl;

}

//********************************************************************
//
// Initialize Semaphores Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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

//********************************************************************
//
// Main Function
//
// 
//
// Value Parameters
// ----------------
// 
// 
// 
//
// Reference Parameters
// --------------------
// 
//
// Local Variables
// ---------------
// 
// 
//*******************************************************************
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