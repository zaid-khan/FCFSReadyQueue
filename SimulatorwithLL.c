/*
NAME:	 SimulatorwithLL.c
PROGRAM: A C Program using link-list to generate a finite number of Processes having random no. of 
		 interleaved I/O and CPU Burst of random duration using random number
		 generating function.
BY:		 Zaid
COMPATIBLE COMPILER: GCC 4.9.2.
*/


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>


/*Constants make it easy to make changes and get them reflected throughout in the program.
  I don't like to use Macros since they're a cause of various errors(for eg. they're not 
  processed by the compiler)*/
const int BURSTRANGE = 3;
const int ARRIVALRANGE = 4;
const int NOFBURST = 2;
const int NOOFPROCESSES = 4;
static int waitingTime = 0;
/*This structure is for each individual bursts. Each burst consists of its time, it's start time,
 it's end time(both have default values of -1 assigned to them in the program below) and the 
 pointer to the next burst since each process can have 'n'(variable) number of bursts. */
typedef struct Burst
{
	int bTime;
	int starttime, endtime;
	struct Burst *next;
}Burst;


/*This structure is for each individual Process Data. Each Process Data consists of its PID, Arrival Time,
  Number of bursts and a pointer to its first burst which is of the type above. */
typedef struct ProcessData 
{
	int pID;
	int aTime;
	int nBurst;
	Burst *pBurstList;
}ProcessData;


/*This structure is for each individual Process. Each Process consists of its Data 
  and a pointer to the next Process. */
typedef struct node
{
	ProcessData data;
	struct node *next;
}Process;



/*Global variables. This is the declaration of Ready, Running, Completed, Uncompleted and Blocked Queue pointers.*/
Process *readyQueue = NULL;
Process *blockQueue = NULL;
Process *Puncompleted = NULL;
Process *Pcompleted = NULL;
Process *running = NULL;



/*Method Declaration*/
Burst * CreateBursts(Process *);
void ManageBlockQueue(int);
Process * RemoveFromBlockQueue(Process *);
void ManageReadyQueue(int);
void ManageRunning(int);
void PrintBlockQueue();
void PrintReadyQueue();
void PrintRunning();
void PrintCompleted();
void ManageArrival(int);






//Method Declaration + Definiton for small methods
bool CheckCompletion()
{
	if (readyQueue == NULL && blockQueue == NULL && running == NULL && Puncompleted == NULL)
		return true;
	return false;
}


//---------------------------------------------------------------------------
bool isCPUFree() 
{
	if (running == NULL)
		return true;
	else
		return false;
}

//------------------------------------------------------------------------------------------------
void ManageTheTime(int counter, Burst *t)
{
	t->starttime = counter;
	t->endtime = counter + t->bTime;
}


//---------------------------------------------------------------------------
void TransferToCPU(Process *temp, int counter)
{
	running = temp;
	//printf ("\nProcess P%d has started running", temp->data.pID); 
	Burst *t = temp->data.pBurstList->next; //The 1st burst is always the IO burst so we start from 2nd burst
	while(t->starttime != -1)
	{
		//Since we're jumping from 2nd to 4th we need to check whether the 
		//4th burst exists or not otherwise we'll access a random location
		//**Remember the number of bursts are always odd in numbers**
		//**Remember the first and last bursts are always IO bursts**
		if ((t->next)->next != NULL) 
			t = (t->next)->next; //CPU bursts are alternating, NOT next to each other
	}
	temp->next = NULL;
	ManageTheTime(counter, t);
	printf("\nProcess P%d will now start its execution.", temp->data.pID);
}




//------------------------------------------------------------------------------------------------
void TransferToReadyQueue(Process *temp, int counter)
{
	Process *traverse;
	if (readyQueue == NULL)
		readyQueue = temp;
	else
	{
		traverse = readyQueue;
		while (traverse->next != NULL)
		{
			traverse = traverse->next;	
		}
		traverse->next = temp;
	}
	temp->next = NULL;	
	printf ("\nProcess P%d has been added to the Ready Queue.", temp->data.pID);
}


//------------------------------------------------------------------------------------------------
void TransferToCompletedQueue(Process *temp, int counter)
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
	printf ("\nProcess P%d is finished.", temp->data.pID);	
}








/*Method Definition*/
//------------------------------------------------------------------------------------------------
Process * CreateElements()
{
	//srand(time(NULL));
	srand(18121995);
	Process *firstProcess, *P;
	int arrivalTime = 0;
	

	firstProcess = (Process *)malloc(sizeof(Process)); //Memory allocation
	 
	firstProcess->data.pID = 1;
	firstProcess->data.aTime = arrivalTime;
	firstProcess->data.nBurst = rand() % ARRIVALRANGE + 1;
	

	firstProcess->data.pBurstList = CreateBursts(firstProcess);
	firstProcess->next = NULL;

	P = firstProcess; 

	//Since we have already created the first process i is from 2 to n.(here in this case, 3)
	for (int i = 2; i <= NOOFPROCESSES; i++) 
	{
		
		P->next = (Process *)malloc(sizeof(Process));
		P = P->next;
		P->data.pID = i;
		arrivalTime += rand() % ARRIVALRANGE + 1; //Cumulative
		P->data.aTime = arrivalTime;
		P->data.nBurst = rand() % NOFBURST + 1;
		//Call to CreateBursts with the Process being passed as a pointer and the 
		//first burst address getting returned.
		P->data.pBurstList = CreateBursts(P); 
		P->next = NULL;
	}
	return firstProcess;
}


//------------------------------------------------------------------------------------------------
Burst * CreateBursts(Process *P)
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
			temp->starttime = -1;
			temp->endtime = -1;
			temp->next = NULL;
		}	
		return firstBurst;
}

//------------------------------------------------------------------------------------------------
void printLL(Process *head)
{
	while (head != NULL)
	{
		printf("PID : %d\n", head->data.pID);
		printf("Arrival Time : %d\n", head->data.aTime);
		printf("No. of Burst : %d\n", head->data.nBurst);
		Burst *h = head->data.pBurstList;
		
		printf("Bursts : ");
		while(h != NULL)
		{
			printf("%d ", h->bTime);
			//printf("%s  ", h->isDone ? " True" : " False");
			h = h->next;
		}
		printf("\n--------------------------------------------\n");
		head = head->next;
	}
}



//------------------------------------------------------------------------------------------------
void AddToBlockQueue(Process *temp, int counter)
{
	Burst *t;
	if(blockQueue == NULL)
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
	while(t->starttime != -1)
	{
		if (t->next != NULL) //For the 4th, 6th...bursts whether they're NULL or not.
			t = (t->next)->next; //IO bursts are alternating, NOT next to each other
	}
	ManageTheTime(counter, t);
	temp->next = NULL;
	printf("\nProcess P%d has been added to the block Queue.", temp->data.pID);
}


//------------------------------------------------------------------------------------------------
Process * RemoveFromBlockQueue(Process *toreturn)
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




//------------------------------------------------------------------------------------------------
void ManageArrival(int counter)
{
	if (Puncompleted != NULL)
	{
		Process *temp = Puncompleted;
		while (temp != NULL)
		{
			if (temp->data.aTime == counter)
			{
				Puncompleted = temp->next; //We're removing the current process from the initial list i.e. Puncompleted, so the Puncompleted 
				//head will point at the next element in the list
				printf ("\nProcess P%d has arrived at %dms", temp->data.pID, counter);
				AddToBlockQueue(temp, counter);
				temp = Puncompleted; //temp will now point at the next element in the list as the current element has been removed
				//break; //Assuming two processes won't arrive at the same time, i.e. their arrival time differs.
			}
			else
			{
				printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, counter);
				temp = temp->next;
			}
		}
	}
}





//------------------------------------------------------------------------------------------------
void ManageBlockQueue(int counter)
{
	if(blockQueue != NULL)
	{
		bool tomove = false;
		Process *temp = blockQueue;
		Process *para;  
		while (temp != NULL)
		{
			Burst *t = temp->data.pBurstList;
			while (t != NULL)
			{
				tomove = false; //Initially tomove set to false
				if ((t->starttime != -1) && (t->endtime >= counter)) //Old t->endtime + 1 >= counter
				{	
					
					if (t->starttime < counter && ((t->endtime - 1) > counter)) //Old - t->endtime > counter
						printf("\nProcess P%d is currently in the Block Queue doing I/O.", temp->data.pID);
					//else if (t->starttime == counter)
					//	printf("\nProcess P%d has just been added to the Block Queue for doing I/O.", temp->data.pID);
					else if (t->endtime - 1 == counter) //Old - "else if (t->endtime == counter)"
						printf("\nProcess P%d is finishing up doing I/O in the BlockQueue.", temp->data.pID);
					else if (t->endtime == counter) //counter has exceeded the endtime by 1. Old - "else if (t->endtime + 1 == counter)"
					{
						printf("\nProcess P%d will now exit the BlockQueue.", temp->data.pID);
						
						//para is a temporary variable to store the value of the next 
						//process since the current process will be removed from the list and
						//will be part of another list.
						
						//**Remember we're removing processes from list we're traversing**
						para = temp->next;
						if (t->next == NULL)
							TransferToCompletedQueue(RemoveFromBlockQueue(temp), counter);
						
						else
						{
							TransferToReadyQueue(RemoveFromBlockQueue(temp), counter);
							/*I have disabled direct transition from blocked to running instead I'll add
						    to the ready queue for that second(counter) and if the CPU is free at that second/coun.
							it will get transitioned from there in ManageReadyQueue() */
						}
						tomove = true;	//tomove flag set to true
										//Process needs moving to another list
						break; //No point in further checking next I/O burst of that process so break.
					}
				}
				if (t->next != NULL) //IF the next burst is not NULL then the burst after the next burst won't be NULL because IO burst will be the final burst
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






//------------------------------------------------------------------------------------------------
void ManageReadyQueue(int counter)
{
	if (readyQueue != NULL)
	{
		bool tomove = false;
		Process *temp = readyQueue;
		Process *para;  
		while (temp != NULL)
		{
			Burst *t = temp->data.pBurstList; //1st burst which is IO burst
			while (t != NULL)
			{
				tomove = false; //Initially tomove set to false
				if ((t->starttime != -1) && (t->endtime <= counter)) //IOBurst has been completed of that process ......odd bursts
				{													 //Old - t->endtime < counter
					if ((t->next != NULL) && ((t->next)->starttime == -1)) //There "exists" a CPU Burst after the 
					{												       //IO burst which has NOT been started
						if(isCPUFree())
						{
							para = temp->next; //para is here
							printf("\nCPU is free right now.");
							TransferToCPU(temp, counter);
							tomove = true;
							 
							//para is a temporary variable to store the value of the next 
							//process since the current process will be removed from the list and
							//will be part of another list.
						
							//**Remember we're removing processes from list we're traversing**
							
							break;
						}
						else
						{
							waitingTime++;
							printf("\nProcess P%d is still in the Ready Queue as the CPU is not free.", temp->data.pID);
							break;
						}
					}
					
				}
				if (t->next != NULL) //IF the next burst is not NULL then the burst after the next burst won't be NULL because IO burst will be the final burst
					t = (t->next)->next; //Next IO Burst
				else
					t = NULL; //IF the next burst is NULL then the burst after the next burst will be NULL too
			}
			if (tomove)
				readyQueue = temp = para; //Next process after removal of the current process
			else
				temp = temp->next; //The Normal Next Process		
		}
	}		
	//else
	//	printf("\nReady Queue is empty");
}



//------------------------------------------------------------------------------------------------
void ManageRunning(int counter)
{
	if(running != NULL)
	{
		Process *temp = running;
		//Process *para;  
		Burst *t = temp->data.pBurstList->next; //Atleast 1 CPU Burst exists for each process so we can directly point towards it
		do
		{
			if ((t->starttime != -1) && (t->endtime >= counter)) //The last second of the CPU burst should be greater or equal to the
			{													  // current time.	   
				if (t->starttime < counter && ((t->endtime - 1) > counter))
					printf("\nProcess P%d is currently executing.", temp->data.pID);
				//else if (t->starttime == counter)
				//	printf("\nProcess P%d has just started execution.", temp->data.pID);
				else if (t->endtime - 1 == counter) //Old - else if (t->endtime == counter)
					printf("\nProcess P%d is finishing execution.", temp->data.pID);
				else if (t->endtime == counter) //counter has exceeded the endtime by 1 Old - else if (t->endtime + 1 == counter)
				{
					printf("\nProcess P%d execution is over. It will free the CPU now.", temp->data.pID);
					running = NULL;
					AddToBlockQueue(temp, counter);
					ManageReadyQueue(counter); 
					/*I'm calling ManageReady() again, because if a process has completed execution at current time, 
					there may have been be a process at current time waiting at the ReadyQueue which got rejected 
					in ManageReadyQueue() before(remember ManageReadyQueue is called before this method ManageRunning())
					because of the current process which got removed here
					and got admitted to the Block Queue. */
					break; //Execution is over, now time for IO. Remember a process ALWAYS ENDS WITH A IO BURST
				}
			}
			if ((t->next)->next != NULL) //IF 4th...6th....8th(ie the 2nd...4th...6th.... CPU burst) burst is not null 
				t = (t->next)->next; //Alternate Burst
			else
				t = t->next->next; // OR t = NULL;
		}while (t != NULL);
	}
	//else
	//	printf("\nNo process getting executed currently.");
}



//------------------------------------------------------------------------------------------------
void PrintReadyQueue()
{
	Process *temp = readyQueue;
	int count = 0;
	printf ("\n\n------The Ready Queue-------");
	while (temp != NULL)
	{
		++count;
		printf ("\n\t%d. Process P%d", count, temp->data.pID);
		temp = temp->next;
	}
	if(count == 0)
		printf("\n\tQueue is empty");
	printf ("\n----------------------------");
}


//------------------------------------------------------------------------------------------------
void PrintBlockQueue()
{
	Process *temp = blockQueue;
	int count = 0;
	printf ("\n\n------The Block Queue-------");
	while (temp != NULL)
	{
		++count;
		printf ("\n\t%d. Process P%d", count, temp->data.pID);
		temp = temp->next;
	}
	if(count == 0)
		printf("\n\tQueue is empty");
	printf ("\n----------------------------");
}



//------------------------------------------------------------------------------------------------
void PrintCompleted()
{
	Process *temp = Pcompleted;
	int count = 0;
	printf ("\n\n------The Completed List-------");
	while (temp != NULL)
	{
		++count;
		printf ("\n\t%d. Process P%d", count, temp->data.pID);
		temp = temp->next;
	}
	if(count == 0)
		printf("\nNo Process has been completed by now.");
	printf ("\n---------------------------------");
}


//------------------------------------------------------------------------------------------------
void PrintRunning()
{
	Process *temp = running;
	printf ("\n\n------Under CPU Execution-------");
	if (temp != NULL)
		printf ("\n\t>Process P%d<", temp->data.pID);
	else
		printf ("\nNo process is currently under execution.");
	printf ("\n--------------------------------");
}




//------------------------------------------------------------------------------------------------
int main()
{
	char ch;
	Puncompleted = CreateElements();
	printLL(Puncompleted);
	
	
	
	printf("\n-----------------------------------------------------------------------------");
	printf("\nTO STOP EXECUTION PRESS THE ; KEY AND THEN PRESS ENTER.");
	printf("\nTO CONTINUE JUST PRESS ENTER.");
	printf("\nASSUMPTION: STS takes a negligible amount of time to execute.");
	printf("\n-----------------------------------------------------------------------------");
	for(int counter = 0;/*Loops breaks from the body itself so this part is left empty*/;++counter)
	{   
		printf("\n\n\n\n================Counter %dms-%0.2fms======================================", counter, (float)counter + 0.99);
		
		ManageArrival(counter);
		ManageBlockQueue(counter); 
		ManageReadyQueue(counter);
		ManageRunning(counter);
		printf("\n-------------------------------------------------------------------------------\n");
		PrintBlockQueue();
		PrintReadyQueue();
		PrintRunning();
		PrintCompleted();
		printf("\n========================================================================\n");
		scanf("%c", &ch);
		if(CheckCompletion() || (ch == ';'))
			break;
	}
	
	return 0;
}
