//*********************************************************
//
// John Alves
// Operating Systems
// Project #3: Process Synchronization Using Pthreads: 
// The Producer / Consumer Problem With Prime Number Detector 
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
#include <vector>
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

// These are used to store the thread id's for both
// consumer and producer threads
pthread_t produceTid[MAX_THREADS];
pthread_t consumeTid[MAX_THREADS];

// These store the integer value of time that the
// user wishes the main program to run for, and 
// how long the threads should sleep.
int mSleep = 0, tSleep = 0;

// Integer arrays used for holding the number of times
// a thread successfully produces or consumes from the buffer
int pThread [MAX_THREADS], cThread [MAX_THREADS];

// Holds the overall number of produced/consumed items
int pSize = 0, cSize = 0;

// String to hold the user's decision to show the snapshot
// while the application is running
string snapshot = "";

// Bool values to stop the application from running, and
// whether or not to display the snapshot.
bool displaySnapshot = false;
bool run = true;

//****************************************************************************************************
//
// Buffer Remove Item Function
// 
// This function is called by the consumer function/thread and will 
// consume an item in the buffer, if there are any.  If there aren't
// any items to consume, it will return to the consumer function.
// It also displays the value that is consumed.
//
// Global Parameters
// ----------------
// buffer			buffer_item		The shared memory buffer
// consumerThreads	int				The number of consumer threads as determined by the user
// count			int				Holds the number of items in the buffer
// minCount			int				Holds the number of times the buffer is empty
// sem_wait			semaphore		Ensures only one consumer thread accesses the buffer at a time
// display			mutex			Prevents other threads from sending output to the console
// displaySnapshot	bool			Determines whether or not to print snapshot information
// consumeTid		pthread_t array	Holds the thread id's of the consumer threads
// cThread			int array		Holds the number of items consumed by each thread
// consumePtr		int pointer		Points to the next location in the buffer to be consumed
//
// Reference Parameters
// --------------------
// tid				pthread_t		The calling thread ID
//
// Local Variables
// ---------------
// consumedValue	int				This holds the value that is consumed
// 
//****************************************************************************************************
int buffer_remove_item(pthread_t tid)
{
	int consumedValue;

	// Decrement until 0, then lock
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

	for (int i = 0; i < consumerThreads; i++)
	{
		if (consumeTid[i] == tid)
			cThread[i] += 1;
	}

	// Increase the total number of items consumed
	cSize++;

	// Store the value that is consumed from the buffer, then move the pointer
	// to the next location in the buffer.  Decrement count to indicate a value
	// has been consumed and a buffer location is available
	consumedValue = buffer[consumePtr];
	consumePtr = (consumePtr + 1) % BUFFER_SIZE;
	count--;

	sem_post(&empty);

	return consumedValue;
}

//****************************************************************************************************
//
// Buffer Insert Item Function
// 
// This function is called by the producer function/thread and will 
// produce an item in the buffer, if the buffer is not full.  If the buffer is
// full, it will return to the produce function and return true.
// It also displays the value that is produced.
//
// Global Parameters
// ----------------
// buffer			buffer_item		The shared memory buffer
// producerThreads	int				The number of producer threads as determined by the user
// count			int				Holds the number of items in the buffer
// maxCount			int				Holds the number of times the buffer is full
// sem_wait			semaphore		Ensures only one consumer thread accesses the buffer at a time
// display			mutex			Prevents other threads from sending output to the console
// displaySnapshot	bool			Determines whether or not to print snapshot information
// producerTid		pthread_t array	Holds the thread id's of the producer threads
// pThread			int array		Holds the number of items produced by each thread
// producePtr		int pointer		Points to the next location in the buffer to be produced
//
// Reference Parameters
// --------------------
// tid				pthread_t		The calling thread ID
// number			buffer_item		The number to be added to the buffer
//
// Local Variables
// ---------------
// 
//****************************************************************************************************
bool buffer_insert_item(buffer_item item, pthread_t tid)
{
	sem_wait(&empty);

	// If the buffer is full, increase the counter display that the producer is waiting
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

	// Loop through the producerTid array that holds the producer thread ids
	// until the current thread id is found.  At that point, increase the pThread
	// value for that corresponds with the producer tid.  This keeps track of how many
	// items each producer produces.
	for (int i = 0; i < producerThreads; i++)
	{
		if (produceTid[i] == tid)
			pThread[i] += 1;
	}

	// Add the item to the buffer and advance the producer pointer to the next place
	// in the buffer.
	buffer[producePtr] = item;
	producePtr = (producePtr + 1) % BUFFER_SIZE;

	// Increase the total number of items produced
	pSize++;

	count++;
	sem_post(&full);
}

//****************************************************************************************************
//
// Get Prime Function
// 
// This function determines if the number consumed is prime or not.
// The code was found online:
// http://www.cplusplus.com/forum/general/1125/
//
//
// Reference Parameters
// --------------------
// number			buffer_item		The number that was consumed
//
// Local Variables
// ---------------
// prime			bool			Will return true/false depending if the value is prime or not
// divisor			int				Used to divide the number variable
// num_d			double			Temp variable to hold the number variable contents
// 
//****************************************************************************************************
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
// This function is only displayed if the user wishes to see snapshot
// information of the program while it's running.  It will output
// The buffer contents, where the consume and producer pointers are
// and how many buffers are occupied. 
//
// Local Variables
// --------------------
// i, j, k, m, n, p		int		Counters for various for loops
//
// Global Variables
// ---------------
// count
// consumePtr
// producePtr
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
// This function is called by each consume thread.  It will in turn call
// the buffer_remove_item function.  If a value is consumed/removed then
// this function will display that information (if the user wants snapshot
// information).  If the user wants the snapshot information to be displayed,
// it will print the value that each thread is consuming as well as if the
// value is prime or not.  The function will then sleep for the user
// specified time.
//
// Value Parameters
// ----------------
// ctid				pthread_t		Holds the thread ID of each consumer thread
// 
// Global Variables
// ----------------
// consumeTid		pthread_t		An array to hold all consumer thread ids
// displaySnapshot	bool			Determines if the snapshot should be displayed
// run				bool			When set to false by the main function, all threads will exit
//
// Local Variables
// ---------------
// number			buffer_item		Holds the value that is consumed
// self				pthread_t		Holds the current thread's ID
// pointer			int				Used to convert the thread ID to an int
// tid				int				Holds the converted thread ID
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

	// Store the consumer thread ID into the array
	consumeTid[tid] = self;

	do
	{
		sem_wait(&empty);

		// Remove an item from the buffer, and store it in number.
		number = buffer_remove_item(self);

		// Display the snapshot if the user requested to see it.
		if (displaySnapshot == true)
		{
			// The remove item function will return the value that
			// was consumed.  If the buffer was empty, it returns -1
			// This if statement will only execute if a value was 
			// successfully consumed.
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
				
				dispBuf();
				pthread_mutex_unlock(&display);
			}
		}
		// Release the lock
		sem_post(&full);

		// Sleep for the length of time the user specified
		nanosleep(&threadSleep, NULL);
	} while (run = true);

	pthread_exit(0);
}

//********************************************************************
//
// Produce Function
//
// This function is called by each producer thread.  It will in turn call
// the buffer_insert_item function.  If a value is produced then
// this function will display that information (if the user wants snapshot
// information).  If the user wants the snapshot information to be displayed,
// it will print the value that each thread is producing. 
// The function will then sleep for the user specified time.
//
// Value Parameters
// ----------------
// ptid				pthread_t		Holds the thread ID of each producer thread
// 
// Global Variables
// ----------------
// produceTid		pthread_t		An array to hold all producer thread ids
// displaySnapshot	bool			Determines if the snapshot should be displayed
// run				bool			When set to false by the main function, all threads will exit
//
// Local Variables
// ---------------
// number			buffer_item		Holds the value that is produced
// self				pthread_t		Holds the current thread's ID
// pointer			int				Used to convert the thread ID to an int
// tid				int				Holds the converted thread ID
// bufferFull		bool			Determines if the buffer is full or not
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

	// Store the thread ID into the producer thread id array
	produceTid[tid] = self;	

	do
	{
		sem_wait(&full);

		// Generate a random number to be inserted into the buffer
		number = rand() % 10000 + 1;

		// Attempt to insert the number into the buffer
		bufferFull = buffer_insert_item(number, self);


		if (displaySnapshot == true)
		{
			// If the buffer isn't full, display the number that was produced
			if (!bufferFull)
			{
				pthread_mutex_lock(&display);

				cout << "Producer " << self << " writes " << number << endl;
				
				// Call the dispBuf function to display the buffer snapshot
				dispBuf();
				pthread_mutex_unlock(&display);
			}
		}

		sem_post(&empty);

		// Sleep for the user specified time
		nanosleep(&threadSleep, NULL);

	} while (run = true);

	pthread_exit(0);
}

//********************************************************************
//
// Display Results Function
//
// This function displays the results of the producer/consumer
// threads and the buffer contents upon completition of the program
//
// Global Variables
// ---------------
// mSleep				int			The integer value of the main sleep
// tSleep				int			The integer value of the thread sleep
// producerThreads		int			The number of producer threads
// consumerThreads		int			The number of consumer threads
// pSize				int			The number of produced values
// cSize				int			The number of consumed values
// pThread				int array	The producer thread IDs
// cThread				int array	The consumer thread IDs
// count				int			The current count (indicates the number remaining in the buffer)
// maxCount				int			The number of times the buffer was full
// minCount				int			The number of times the buffer was empty
// 
//*******************************************************************
void displayResults()
{
	cout << endl;
	cout << "PRODUCER / CONSUMER SIMULATION COMPLETE" << endl;
	cout << "=======================================" << endl;
	cout << "Simulation Time:                       " << mSleep << endl;
	cout << "Maximum Thread Sleep Time:             " << tSleep << endl;
	cout << "Number of Producer Threads:            " << producerThreads << endl;
	cout << "Number of Consumer Threads:            " << consumerThreads << endl;
	cout << "Size of Buffer:                        " << BUFFER_SIZE << endl << endl;
	cout << "Total Number of Items Produced:        " << pSize << endl;

	for (int i = 0; i < producerThreads; i++) {
	cout << "\t Thread " << i + 1 << ":" << "                     " << pThread[i] << endl;
	}

	cout << endl;
	cout << "Total Number of Items Consumed:        " << cSize << endl;
	for (int i = 0; i < consumerThreads; i++) {
	cout << "\t Thread " << i + 1 << ":" << "                     " << cThread[i] << endl;
	}

	cout << endl;
	cout << "Number Of Items Remaining in Buffer:   " << count << endl;
	cout << "Number Of Times Buffer Was Full:       " << maxCount << endl;
	cout << "Number Of Times Buffer Was Empty:      " << minCount << endl;

}

//********************************************************************
//
// Initialize Semaphores Function
//
// This function initializes the semaphores and the mutex.
// 
//
// Global Variables
// ---------------
// full		semaphore	Unlocks the function when it is greater than 0		
// empty	semaphore	Locks the function when it reaches 0
// display	mutex		Locks the output of information
// 
//*******************************************************************
int initSem()
{

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
// This function is the main program.  It will read in the command line
// parameters and store them in variables.  It will also check for proper
// syntax and do some error checking.
//
// Global Variables
// ---------------
// Most of the global variables are utilized in this function.
// The explanation for them is at the top of the program.
//*******************************************************************
int main(int argc, char *argv[])
{
	initSem();

	pthread_t threads[MAX_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// Check to see if the right number of arguments have been supplied
	if (argc != 6)
	{
		cout << "Not enough arguments were supplied." << endl;
		cout << "Ex: ./osproj3 30 3 2 2 yes" << endl;
		return 0;
	}

	// Store the Main Sleep and Thread Sleep arguments into integer variables
	mSleep = atoi(argv[1]);
	tSleep = atoi(argv[2]);

	// Store the producer and consumer thread numbers into integer variables
	producerThreads = atoi(argv[3]);
	consumerThreads = atoi(argv[4]);

	// Store the snapshot argument into a string variable
	snapshot = argv[5];

	// Set the bool variable accordingly
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

	// Ensure that the arguments are valid, otherwise display an error and quit
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
	// Create the producer threads
	for (int j = 0; j < producerThreads; j++)
	{
		pthread_t tid;
		produceTid[j] = j;
		pthread_create(&tid, &attr, produce, (void *)&produceTid[j]);
	}
	// Create the consumer threads
	for (int i = 0; i < consumerThreads; i++)
	{
		pthread_t tid;
		consumeTid[i] = i;
		pthread_create(&tid, &attr, consume, (void *)&consumeTid[i]);
	}
	// Sleep for the specified amount of time
	nanosleep(&mainSleep, NULL);
	// Set run to false in order to stop the threads from running
	run = false;
	// Display the results from the program
	displayResults();
	// PROFIT
	return 0;

}
