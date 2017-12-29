//ROUND ROBIN - TIME QUANTUM
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <random>

using namespace std;

const int BURSTRANGE = 8;
const int ARRIVALRANGE = 3;
const int NOFBURST = 10;
const int NOOFPROCESSES = 400;
const int RRQUANTUM = 6;
// static int waitingTime = 0;
// static int waitingTimeBlocked = 0;
// static int waitingArrivalTime = 0;
static int counter;
static int processcount;
static int arrivalTimeTrack;
static int cpuutil;
bool STSExecutionComplete;
bool hasSTSExecuted = false;

typedef struct Burst
{
	int bTime;
	int starttime, endtime;
	struct Burst *next;
} Burst;

typedef struct ProcessData
{
	int pID;
	int aTime;
	int nBurst;
	int waitingTime;
	int waitingTimeBlocked;
	Burst *pBurstList;
} ProcessData;

typedef struct node
{
	ProcessData data;
	struct node *next;
} Process;

typedef struct ReadyQueue
{
	int timeRemaining;
	int burstNo;
	Process *process;
	struct ReadyQueue *next;
} ReadyQueue;

//Process *readyQueue = NULL;
ReadyQueue *readyQueue = NULL;
Process *blockQueue = NULL;
Process *PinitialList = NULL;
Process *Pcompleted = NULL;
Process *running = NULL;
Process *STS = NULL;

Burst *CreateBursts(Process *);
void ManageBlockQueue(int);
void ManageReadyQueue(int);
ReadyQueue* ManageRunning(int);
void PrintBlockQueue();
void PrintReadyQueue();
void PrintRunning();
void PrintCompleted();
void ManageArrival(int);
void PrintWaitingTime();
void AddNewFreshProcess();
void randomGenerationProcess();

//RandomGenerationFunction in C++14
void randomGenerationProcess()
{
	//random_device rd;

	// Initialize Mersenne Twister pseudo-random number generator
	mt19937 gen(time(NULL));

	// Generate pseudo-random numbers
	// uniformly distributed in range (1, 100)
	uniform_int_distribution<> dis(1, 100);

	// Generate ten pseudo-random numbers
	bool flag = false;
	for (int i = 0; i < 3; i++)
	{
		int randomX = dis(gen);
		if (randomX > 70 && randomX < 77)
			flag = true;	
		//cout << "\nRandom X = " << randomX;
	}
	if (flag)
		AddNewFreshProcess();
}




void AddNewFreshProcess()
{
	Process *P;
	
	int c = STSExecutionComplete ? (counter - 1) : counter;
	if (arrivalTimeTrack >= c)
		arrivalTimeTrack = arrivalTimeTrack + (rand() % ARRIVALRANGE + 1);
	else
		arrivalTimeTrack = c + (rand() % ARRIVALRANGE + 1);
	
	
	P = (Process *)malloc(sizeof(Process)); //Memory allocation

	P->data.pID = processcount++;
	P->data.aTime = arrivalTimeTrack;
	P->data.nBurst = rand() % ARRIVALRANGE + 1;
	P->data.waitingTime = 0;
	P->data.waitingTimeBlocked = 0;

	P->data.pBurstList = CreateBursts(P);
	P->next = NULL;
	
	if (PinitialList == NULL)
		PinitialList = P;
	else
	{
		Process *temp = PinitialList;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = P;
	}
	printf ("\nNew Process P%d added which arrived at %d", P->data.pID, P->data.aTime);
}


bool CheckCompletion()
{
	if (readyQueue == NULL && blockQueue == NULL && running == NULL && PinitialList == NULL)
		return true;
	return false;
}

bool STSisCPUFree()
{
	if (running == NULL)
		return true;
	else
		return false;
}

void STSManageTheTime(int cnt, Burst *t)
{
	t->starttime = cnt;
	t->endtime = cnt + t->bTime;
}

void STSTransferToCPU(Process *temp, int cnt, int quantum, Burst *t)
{
	running = temp;
	//printf ("\nProcess P%d has started running", temp->data.pID);
	// Burst *t = temp->data.pBurstList->next; //The 1st burst is always the IO burst so we start from 2nd burst
	// while(t->starttime != -1)
	// {
	// 	//Since we're jumping from 2nd to 4th we need to check whether the
	// 	//4th burst exists or not otherwise we'll access a random location
	// 	//**Remember the number of bursts are always odd in numbers**
	// 	//**Remember the first and last bursts are always IO bursts**
	// 	if ((t->next)->next != NULL)
	// 		t = (t->next)->next; //CPU bursts are alternating, NOT next to each other
	// }
	temp->next = NULL;
	//STSManageTheTime(cnt, t);
	t->starttime = cnt;
	t->endtime = t->starttime + quantum;
	printf("\nProcess P%d will now start its execution.", temp->data.pID);
}

int FindTimeRemaining(Process *temp)
{
	Burst *t = temp->data.pBurstList->next;
	while (t->starttime != -1)
	{
		t = t->next->next; //t = Next of the next IO burst.
	}
	return t->bTime;
}

int FindBurstNumber(Process *temp)
{
	int c = 1;
	Burst *t = temp->data.pBurstList->next;
	while (t->starttime != -1)
	{
		t = t->next->next; //t = Next of the next IO burst.
		c = c + 2;
	}
	return c;
}

void STSTransferToReadyQueue(Process *temp, int cnt)
{
	if (readyQueue == NULL)
	{
		readyQueue = (ReadyQueue *)malloc(sizeof(ReadyQueue));
		readyQueue->process = temp;
		readyQueue->timeRemaining = FindTimeRemaining(temp);
		readyQueue->burstNo = FindBurstNumber(temp);
		readyQueue->next = NULL;
	}
	else
	{
		ReadyQueue *traverse;
		ReadyQueue *newprocess;

		newprocess = (ReadyQueue *)malloc(sizeof(ReadyQueue));
		newprocess->process = temp;
		newprocess->timeRemaining = FindTimeRemaining(temp);
		newprocess->burstNo = FindBurstNumber(temp);
		newprocess->next = NULL;

		traverse = readyQueue;
		while (traverse->next != NULL)
		{
			traverse = traverse->next;
		}
		traverse->next = newprocess;
	}
	printf("\nProcess P%d has been added to the Ready Queue.", temp->data.pID);
}

void STSTransferToCompletedQueue(Process *temp, int cnt)
{
	Process *traverse;
	if (Pcompleted == NULL)
		Pcompleted = temp;
	else
	{
		traverse = Pcompleted;
		while (traverse->next != NULL)
		{
			traverse = traverse->next;
		}
		traverse->next = temp;
	}
	temp->next = NULL;
	printf("\nProcess P%d is finished.", temp->data.pID);
}

void STSAddToBlockQueue(Process *temp, int cnt)
{
	Burst *t;
	if (blockQueue == NULL)
		blockQueue = temp;

	else
	{
		Process *trav = blockQueue;
		while (trav->next != NULL)
		{
			trav = trav->next;
		}
		trav->next = temp;
	}

	t = temp->data.pBurstList;
	while (t->starttime != -1)
	{
		if (t->next != NULL)	 //For the 4th, 6th...bursts whether they're NULL or not.
			t = (t->next)->next; //IO bursts are alternating, NOT next to each other
	}
	STSManageTheTime(cnt, t);
	temp->next = NULL;
	printf("\nProcess P%d has been added to the block Queue.", temp->data.pID);
}

Process *STSRemoveFromBlockQueue(Process *toreturn)
{
	if (blockQueue == toreturn) //That the process is the first process in the block queue.
	{
		blockQueue = toreturn->next; //2nd process or NULL is there is only 1 process in the blockQueue.
	}
	else
	{
		Process *temp = blockQueue, *tempahead = blockQueue->next;
		while (tempahead != toreturn)
		{
			tempahead = tempahead->next;
			temp = temp->next;
		}
		temp->next = tempahead->next;
	}
	return toreturn;
}

bool STSProcess()
{
	if (!STSExecutionComplete)
	{
		if (STSisCPUFree())
		{
			running = STS;
			STSExecutionComplete = true;
			counter++; //global DON'T CHANGE
			printf("\nSTS is under execution.");
			return true;
		}
		else
			return false;
	}
	return true;
}

void RemoveSTS()
{
	if (running == STS)
	{
		printf("\nSTS will now stop execution.");
		running = NULL;
		hasSTSExecuted = true;		  //Previously, it has executed.
		STSExecutionComplete = false; //Completion status from the previous counter is removed.
	}
	else
	{
		hasSTSExecuted = false;
	}
}

Process *CreateElements()
{
	//srand(time(NULL));
	srand(18121995);
	Process *firstProcess, *P;
	int arrivalTime = 0;

	firstProcess = (Process *)malloc(sizeof(Process)); //Memory allocation

	firstProcess->data.pID = 1;
	firstProcess->data.aTime = arrivalTime;
	firstProcess->data.nBurst = rand() % ARRIVALRANGE + 1;
	firstProcess->data.waitingTime = 0;
	firstProcess->data.waitingTimeBlocked = 0;

	firstProcess->data.pBurstList = CreateBursts(firstProcess);
	firstProcess->next = NULL;

	P = firstProcess;

	//Since we have already created the first process i is from 2 to n.(here in this case, 3)
	for (processcount = 2; processcount <= NOOFPROCESSES; processcount++)
	{

		P->next = (Process *)malloc(sizeof(Process));
		P = P->next;
		P->data.pID = processcount;
		arrivalTime += rand() % ARRIVALRANGE + 1; //Cumulative
		P->data.aTime = arrivalTime;
		P->data.nBurst = rand() % NOFBURST + 1;
		P->data.waitingTime = 0;
		P->data.waitingTimeBlocked = 0;
		//Call to CreateBursts with the Process being passed as a pointer and the
		//first burst address getting returned.
		P->data.pBurstList = CreateBursts(P);
		P->next = NULL;
	}
	arrivalTimeTrack = arrivalTime;
	return firstProcess;
}

Burst *CreateBursts(Process *P)
{
	Burst *firstBurst = P->data.pBurstList = (Burst *)malloc(sizeof(Burst));
	Burst *temp = firstBurst;
	firstBurst->bTime = rand() % BURSTRANGE + 1; //First Burst
	firstBurst->starttime = -1;
	firstBurst->endtime = -1;

	for (int j = 1; j <= P->data.nBurst * 2; j++) //First burst created so j from 1 to n * 2
	{
		temp->next = (Burst *)malloc(sizeof(Burst));
		temp = temp->next;
		temp->bTime = rand() % BURSTRANGE + 1; //Remaining Bursts assignment
		if (j % 2 != 0)
			cpuutil += temp->bTime;
		temp->starttime = -1;
		temp->endtime = -1;
		temp->next = NULL;
	}
	return firstBurst;
}

void printLL(Process *head)
{
	while (head != NULL)
	{
		printf("PID : %d\n", head->data.pID);
		printf("Arrival Time : %d\n", head->data.aTime);
		printf("No. of Burst : %d\n", head->data.nBurst);
		Burst *h = head->data.pBurstList;

		printf("Bursts : ");
		while (h != NULL)
		{
			printf("%d ", h->bTime);
			//printf("%s  ", h->isDone ? " True" : " False");
			h = h->next;
		}
		printf("\n--------------------------------------------\n");
		head = head->next;
	}
}

void ManageArrival(int cnt)
{
	if (PinitialList != NULL)
	{
		Process *temp = PinitialList;
		while (temp != NULL)
		{
			if (STSProcess())
			{
				if (temp->data.aTime + 1 <= counter)
				{

					printf("\nIn process of adding the newly arrived Process P%d", temp->data.pID);
					PinitialList = temp->next; //We're removing the current process from the initial list i.e. PinitialList, so the PinitialList
											   //head will point at the next element in the list
					//printf ("\nProcess P%d has been added to the block queue at %dms.", temp->data.pID, cnt);
					//Calling STS
					STSAddToBlockQueue(temp, counter); //ACTUAL INCREMENTED COUNTER
					temp = PinitialList;			   //temp will now point at the next element in the list as the current element has been removed
													   //break; //Assuming two processes won't arrive at the same time, i.e. their arrival time differs.
				}

				else
				{
					//printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, counter - 1);
					temp = temp->next;
				}
			}

			//If NOT Ready then print conf. that process
			//won't be added in the next burst.
			else //If CPU is not free and occupied by some process.
			{
				if (temp->data.aTime == counter)
					printf("\nProcess P%d has arrived but won't be added since STS is not running", temp->data.pID);

				//else
				//	printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, counter);

				temp = temp->next;
				//temp->data.aTime++;
			}
		}
	}
}

void ManageBlockQueue(int cnt)
{
	if (blockQueue != NULL)
	{
		bool tomove = false;
		Process *temp = blockQueue;
		Process *para;
		while (temp != NULL)
		{
			Burst *t = temp->data.pBurstList;
			while (t != NULL)
			{
				tomove = false;											 //Initially tomove set to false
				if ((t->starttime != -1) && (t->endtime + 1 >= counter)) //Counter should be less than or equal to the second after end time
				{

					if (t->starttime < cnt && ((t->endtime) > counter)) //Old - t->endtime > counter
						printf("\nProcess P%d is currently in the Block Queue doing I/O.", temp->data.pID);

					if (STSProcess())
					{
						if (t->endtime + 1 == counter)
						{
							para = temp->next;

							if (t->next == NULL)
								STSTransferToCompletedQueue(STSRemoveFromBlockQueue(temp), counter);

							else
								STSTransferToReadyQueue(STSRemoveFromBlockQueue(temp), counter);
							/*I have disabled direct transition from blocked to running instead I'll add
										to the ready queue for that second(counter) and if the CPU is free at that second/coun.
										it will get transitioned from there in ManageReadyQueue() */

							tomove = true; //tomove flag set to true
							break;		   //No point in further checking next I/O burst of that process so break.
						}
					}
					else
					{
						if (t->endtime == counter)
						{
							temp->data.waitingTimeBlocked++;
							t->endtime++;
							printf("\n--Process P%d will now stay in the Block Queue.", temp->data.pID);
							printf("\n---Endtime extended to %d.", t->endtime);
							break;
						}
					}
				}
				if (t->next != NULL)	 //IF the next burst is not NULL then the burst after the next burst won't be NULL because IO burst will be the final burst
					t = (t->next)->next; //Alternate Burst
				else
					t = NULL; //IF the next burst is NULL then the burst after the next burst will be NULL too
			}
			if (tomove)
				temp = para; //Next process
			else
				temp = temp->next; //The Normal Next Process
		}
	}
	//else
	//	printf("\nBlock Queue is empty");
}

void ManageReadyQueue(int cnt)
{
	if (readyQueue != NULL && running == NULL)
	{
		ReadyQueue *temp = readyQueue;
		ReadyQueue *temp2;
		while (temp != NULL)
		{
			Process *runprocess = temp->process;
			runprocess->data.waitingTime++;
			int burstno = temp->burstNo; //From STSTransferToReadyQueue
			Burst *t = temp->process->data.pBurstList;
			for (int i = 1; i <= burstno; ++i)
				t = t->next;
			if (hasSTSExecuted)
			{
				if (STSisCPUFree())
				{
					int quantum = RRQUANTUM;
					if (temp->timeRemaining >= RRQUANTUM)
						temp->timeRemaining -= RRQUANTUM;
					else
					{
						quantum = temp->timeRemaining;
						temp->timeRemaining = 0;
					}
					//temp->timeRemaining = ((temp->timeRemaining >= RRQUANTUM) ? (temp->timeRemaining - RRQUANTUM) : (0));
					STSTransferToCPU(runprocess, cnt, quantum, t);
					break;
				}
			}
			else
			{
				STSProcess();
				break;
			}
			temp = temp->next;
		}

		if (temp != NULL) //WHILE BROKE NOT BECAUSE OF TEMP BEING NULL
		{
			temp = temp->next;
			while (temp != NULL)
			{
				temp->process->data.waitingTime++;
				temp = temp->next;
			}
		}
	}
	else
	{
		ReadyQueue *temp = readyQueue;
		while (temp != NULL)
		{
			if (running != temp->process)
				temp->process->data.waitingTime++;
			temp = temp->next;
		}
	}
	// if (readyQueue != NULL)
	// {
	// 	bool tomove = false;
	// 	Process *temp = readyQueue;
	// 	Process *para;
	// 	while (temp != NULL)
	// 	{
	// 		Burst *t = temp->data.pBurstList; //1st burst which is IO burst
	// 		while (t != NULL)
	// 		{
	// 			tomove = false; //Initially tomove set to false
	// 			if ((t->starttime != -1) && (t->endtime < cnt)) //IOBurst has been completed of that process ......odd bursts
	// 			{												//At least 1 second(for STS) should pass
	// 															//since that completion. Therefore the greater sign for counter.
	// 				if ((t->next != NULL) && ((t->next)->starttime == -1)) //There "exists" a CPU Burst after the
	// 				{												       //IO burst which has NOT been started
	// 					if (!hasSTSExecuted) //If the STS has NOT been executed previously
	// 					{									 //so STSProcess() will execute or return true is still executing
	// 						temp->data.waitingTime++;
	// 						printf("\nProcess P%d is still in the Ready Queue as the CPU is not free.", temp->data.pID);
	// 						break;
	// 					}
	// 					else if(STSisCPUFree() && hasSTSExecuted)    //CPU must be free as well as STS has been executed in the previous
	// 					{
	// 						temp->data.waitingTime++;										 //burst to move the process from ready to running
	// 						para = temp->next; //para is here
	// 						printf("\nCPU is free right now.");
	// 						STSTransferToCPU(temp, cnt);
	// 						tomove = true;

	// 						//para is a temporary variable to store the value of the next
	// 						//process since the current process will be removed from the list and
	// 						//will be part of another list.

	// 						//**Remember we're removing processes from list we're traversing**

	// 						break;
	// 					}
	// 					else
	// 					{
	// 						temp->data.waitingTime++;
	// 						printf("\nProcess P%d is still in the Ready Queue as the CPU is not free.", temp->data.pID);
	// 						break;
	// 					}
	// 				}

	// 			}
	// 			if (t->next != NULL) //IF the next burst is not NULL then the burst after the next burst won't be NULL because IO burst will be the final burst
	// 				t = (t->next)->next; //Next IO Burst
	// 			else
	// 				t = NULL; //IF the next burst is NULL then the burst after the next burst will be NULL too
	// 		}
	// 		if (tomove)
	// 			readyQueue = temp = para; //Next process after removal of the current process
	// 		else
	// 			temp = temp->next; //The Normal Next Process
	// 	}
	// }
	// else
	// 	STSProcess(); //No process to run so execute the STS.
}

void RemoveFromReadyQueue(ReadyQueue *current)
{
	if (current == readyQueue)
		readyQueue = readyQueue->next;

	else
	{
		ReadyQueue *temp = readyQueue->next;
		ReadyQueue *previous = readyQueue;
		while (temp != NULL && temp != current)
		{
			temp = temp->next;
			previous = previous->next;
		}
		previous->next = temp->next;
	}
	free(current);
}

void RemoveFromReadyQueueAndAppendToEnd(ReadyQueue *current)
{
	ReadyQueue *newReadyProcess = (ReadyQueue *)malloc(sizeof(ReadyQueue));
	newReadyProcess->process = current->process;
	newReadyProcess->burstNo = current->burstNo;
	newReadyProcess->timeRemaining = current->timeRemaining;
	newReadyProcess->next = NULL;
	RemoveFromReadyQueue(current);

	if (readyQueue != NULL)
	{
		ReadyQueue *traverse = readyQueue;
		while (traverse->next != NULL)
			traverse = traverse->next;
		traverse->next = newReadyProcess;
	}
	else
		readyQueue = newReadyProcess;
	/*Append*/
}

ReadyQueue* ManageRunning(int cnt)
{
	if (running != NULL && running != STS)
	{
		Process *temp = running;
		ReadyQueue *readyProcess = readyQueue;
		while (readyProcess->process != running)
			readyProcess = readyProcess->next;
		//Process *para;
		Burst *t = temp->data.pBurstList->next; //Atleast 1 CPU Burst exists for each process so we can directly point towards it
		do
		{
			if ((t->starttime != -1) && (t->endtime >= cnt)) //The second after the last sec of the CPU burst should be greater than or
			{												 //equal to current time because of count + 0.99, which I have put in main().
				
				if (t->starttime < cnt && ((t->endtime) > cnt))
				{
					printf("\nProcess P%d is currently executing.", temp->data.pID);
					return NULL; //Return as there is a process already running so no need to check other processes.
				}
				//else if (t->starttime == counter)
				//	printf("\nProcess P%d has just started execution.", temp->data.pID);
				//else if () //Old - else if (t->endtime == counter)
				//	printf("\nProcess P%d is finishing execution.", temp->data.pID);
				else if (t->endtime == cnt) //counter has exceeded the endtime by 1 Old - else if (t->endtime + 1 == counter)
				{
					running = NULL;
					STSProcess();
					printf("\nProcess P%d quantum is over. It will free the CPU now.", temp->data.pID);
					if (readyProcess->timeRemaining == 0)
					{
						printf("\nProcess P%d execution is over. It will free the CPU now.", temp->data.pID);
						RemoveFromReadyQueue(readyProcess);
						STSAddToBlockQueue(temp, counter);
						return NULL;
					}
					else
					{
						
						//STSTransferToReadyQueue(temp, counter);
						return readyProcess;
					}

					//ManageReadyQueue(counter);
					/*I'm calling ManageReady() again, because if a process has completed execution at current time, 
					there may have been be a process at current time waiting at the ReadyQueue which got rejected 
					in ManageReadyQueue() before(remember ManageReadyQueue is called before this method ManageRunning())
					because of the current process which got removed here
					and got admitted to the Block Queue. */
				}
				//else if (t->endtime + 1 == cnt)
				//{
				//Execution is over, now time for IO. Remember a process ALWAYS ENDS WITH A IO BURST
				//}
			}
			if ((t->next)->next != NULL) //IF 4th...6th....8th(ie the 2nd...4th...6th.... CPU burst) burst is not null
				t = (t->next)->next;	 //Alternate Burst
			else
				t = t->next->next; // OR t = NULL;
		} while (t != NULL);
	}
	return NULL;
	//else
	//	printf("\nNo process getting executed currently.");
}

void PrintReadyQueue()
{
	ReadyQueue *temp = readyQueue;
	int count = 0;
	printf("\n\n------The Ready Queue-------");
	while (temp != NULL)
	{
		if (temp->process != running)
		{
			++count;
			printf("\n\t%d. Process P%d", count, temp->process->data.pID);
		}
		temp = temp->next;
	}
	if (count == 0)
		printf("\n\tQueue is empty");
	printf("\n----------------------------");
}

void PrintBlockQueue()
{
	Process *temp = blockQueue;
	int cnt = 0;
	printf("\n\n------The Block Queue-------");
	while (temp != NULL)
	{
		++cnt;
		printf("\n\t%d. Process P%d. ", cnt, temp->data.pID);
		Burst *t = temp->data.pBurstList;
		while (t != NULL)
		{
			if ((t->starttime <= counter) && (counter <= t->endtime))
			{
				printf("(%d - %d)", t->starttime, t->endtime);
				break;
			}
			else
			{
				if (t->next == NULL)
					t = NULL;
				else
					t = t->next->next;
			}
		}
		temp = temp->next;
	}
	if (cnt == 0)
		printf("\n\tQueue is empty");
	printf("\n----------------------------");
}

void PrintCompleted()
{
	Process *temp = Pcompleted;
	int count = 0;
	printf("\n\n------The Completed List-------");
	while (temp != NULL)
	{
		++count;
		printf("\n\t%d. Process P%d", count, temp->data.pID);
		temp = temp->next;
	}
	if (count == 0)
		printf("\nNo Process has been completed by now.");
	printf("\n---------------------------------");
}

void PrintInitial()
{
	Process *temp = PinitialList;
	int count = 0;
	printf("\n\n------Future Arrivals-------");
	while (temp != NULL)
	{
		++count;
		printf("\n\t%d. Process P%d, AT - %d", count, temp->data.pID, temp->data.aTime);
		temp = temp->next;
	}
	if (count == 0)
		printf("\nNo Process to arrive, yet.");
	printf("\n---------------------------------");
}

void PrintRunning()
{
	Process *temp = running;
	printf("\n\n------Under CPU Execution-------");
	if (temp != NULL)
	{
		if (temp->data.pID == -1)
			printf("\n\t>> STS <<", temp->data.pID);
		else
		{
			printf("\n\t>> Process P%d", temp->data.pID);
			Burst *t = temp->data.pBurstList->next;
			while (t != NULL)
			{
				if (t->starttime <= counter && t->endtime >= counter)
				{
					printf(" (%d - %d)", t->starttime, t->endtime);
					break;
				}
				t = t->next->next;
			}
			printf(" <<");
		}
	}
	else
		printf("\nNo process is currently under execution.");
	printf("\n--------------------------------");
}

	

void PrintWaitingTime()
{
	Process *temp = Pcompleted;
	int sumofwaitingtime = 0;
	int sumofblockedtime = 0;
	while (temp != NULL)
	{
		printf("\nP%d - \n", temp->data.pID);
		printf("\nWaiting Time : %d", temp->data.waitingTime);
		printf("\nBlock Waiting Time : %d", temp->data.waitingTimeBlocked);
		sumofblockedtime = sumofblockedtime + temp->data.waitingTimeBlocked;
		sumofwaitingtime = sumofwaitingtime + temp->data.waitingTime;
		temp = temp->next;
		printf("\n-------------------------------------------------------------\n");
	}
	printf("\nTotal No. of processes : %d processes", processcount - 1);
	printf("\nCumulative Waiting time of all Processes : %dms", sumofwaitingtime);
	printf("\nCumulative Block-Q Waiting time of all Processes : %dms", sumofblockedtime);
	printf("\nAverage Waiting Time : %0.2fms", (float)sumofwaitingtime / (processcount - 1));
	printf("\nAverage Block-Q Waiting Time : %0.2fms", (float)sumofblockedtime / (processcount - 1));
	printf("\nTotal Time required to complete all Processes : %dms", counter);
	printf("\nTotal Time CPU Spent executing Processes : %dms", cpuutil);
	char dum = '%';
	printf("\nCPU Utilisation : %0.2f%c", ((float)cpuutil / counter) * 100, dum);
}

//------------------------------------------------------------------------------------------------
int main()
{
	char ch;
	bool stopProcessCreation = false;
	bool skipDirect = false;
	cpuutil = 0;
	PinitialList = CreateElements();
	printLL(PinitialList);
	STS = (Process *)malloc(sizeof(Process)); //I have created a dummy process
	STS->data.pID = -1;
	STS->data.aTime = -1;
	STS->data.nBurst = -1;
	STS->data.pBurstList = NULL;
	STS->next = NULL;
	ReadyQueue *runningProcess = NULL;
	printf("\n-----------------------------------------------------------------------------");
	printf("\nTO STOP EXECUTION PRESS THE \';\' KEY AND THEN PRESS ENTER.");
	printf("\nSTOPPING EXECUTION WON\'T SHOW THE PROCESSES STATISTICS");
	printf("\nTO STOP CREATING MORE PROCESSES PRESS THE \':\' KEY AND THEN PRESS ENTER.");
	printf("\nTO SKIP DIRECTLY TO STATISTICS PRESS \'.\' KEY");
	printf("\nTO CONTINUE JUST PRESS ENTER.");
	printf("\nASSUMPTION: STS takes a unit of time to execute.");
	printf("\n-----------------------------------------------------------------------------");
	for (counter = 0;/*Loops breaks from the body itself so this part is left empty*/; ++counter)
	{
		//STSExecutionComplete = false;
		if (!stopProcessCreation)
			randomGenerationProcess();
		printf("\n\n\n\n================Counter %dms-%0.2fms======================================", counter, (float)counter + 0.99);
		ManageReadyQueue(counter); //Preference for Ready to Running transition
		runningProcess = ManageRunning(counter);
		ManageArrival(counter);
		ManageBlockQueue(counter);
		if (runningProcess)
			RemoveFromReadyQueueAndAppendToEnd(runningProcess);
		//if (STSExecutionComplete)
		//		printf("\nNOTE: STS took a unit of time to execute, so counter = %d.", counter + 1);
		printf("\n-------------------------------------------------------------------------------\n");
		PrintBlockQueue();
		PrintReadyQueue();
		PrintRunning();
		PrintCompleted();
		PrintInitial();
		if (STSExecutionComplete)
			counter--;
		RemoveSTS();
		printf("\n========================================================================\n");
		if (!skipDirect)
			scanf("%c", &ch);
		if (CheckCompletion() || (ch == ';')) //if(CheckCompletion())
			break;
		else if (ch == ':')
			stopProcessCreation = true;
		else if (skipDirect == false && ch == '.')
		{
			skipDirect = true;		
			stopProcessCreation = true;
		}
	}
	if (ch != ';')
	{
		printf("\n========================WAITING TIME==============================\n");
		PrintWaitingTime();
	}
	return 0;
}
