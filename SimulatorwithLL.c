/*
NAME:	 SimulatorwithLL.c
PROGRAM: A C Program using link-list to generate a finite number of Processes having random no. of 
		 interleaved I/O and CPU Burst of random duration using random number
		 generating function.
BY:		 Zaid
COMPATIBLE COMPILER: GCC 4.5.2.
*/


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>


/*Constants make it easy to make changes and get them reflected throughout in the program.
  I don't like to use Macros since they're a cause of various errors(for eg. they're not 
  processed by the compiler)*/
const int BURSTRANGE = 4;
const int ARRIVALRANGE = 4;
const int NOFBURST = 2;

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



/*Global variables. This is the declaration of Ready and Blocked Queue pointers.*/
Process *readyQueue;
Process *blockQueue;
Process *Puncompleted;
Process *Pcompleted;
Process *running;



/*Method Declaration*/
Burst * CreateBursts(Process *);
void ManageBlockQueue();
void AddToBlockQueue(int, Process *);
void ManageTheTime(int, Burst *);
void ManageReadyQueue();
void PrintBlockQueue();
void PrintReadyQueue();
bool CheckCompletion();
void ManageArrival(int);





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
	for (int i = 2; i <= 3; i++) 
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
void ManageArrival(int counter)
{
	while (Puncompleted != NULL)
	{
		Process *temp = Puncompleted;
		if (temp->data.aTime == counter)
		{
			Puncompleted = temp->next;
			printf ("\nProcess P%d has arrived at %d", temp->data.pID, counter);
			AddToBlockQueue(counter, temp);
		}
	}
}


//------------------------------------------------------------------------------------------------
void AddToBlockQueue(int counter, Process *temp)
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
		t = (t->next)->next; //IO bursts are alternating, NOT next to each other
	}
	ManageTheTime(counter, t);
	temp->next = NULL;
}

//------------------------------------------------------------------------------------------------
void ManageTheTime(int counter, Burst *t)
{
	t->starttime = counter;
	t->endtime = counter + t->bTime;
}

//------------------------------------------------------------------------------------------------
void ManageBlockQueue()
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
				t = (t->next)->next;
				if (t.starttime != -1)
				{	
					
					if (t.starttime < counter && t.endtime > counter)
						printf("\nProcess P%d is currently in the Block Queue doing I/O.", temp->data.pID);
					else if (t.starttime == counter)
						printf("\nProcess P%d has just been added to the Block Queue for doing I/O.", temp->data.pID);
					else if (t.endtime == counter)
						printf("\nProcess P%d is finishing up doing I/O in the BlockQueue.", temp->data.pID);
					else if (t.endtime == counter + 1)
					{
						printf("\nProcess P%d will now exit the BlockQueue.", temp->data.pID);
						
						//para is a temporary variable to store the value of the next 
						//process since the current process will be removed from the list and
						//will be part of another list.
						
						//**Remember we're removing processes from list we're traversing**
						para = temp->next;
						if (t->next == NULL)
							TransferToCompletedQueue(temp);
						
						else
						{
							if(isCPUFree())
								TransferToCPU(temp);
							else
								TransferToReadyQueue(temp);
						}
						tomove = true;	//tomove flag set to true
										//Process needs moving to another list
						break; //No point in further checking next I/O burst of that process so break.
					}
				}
			}
			if (tomove)
				temp = para; //Next process
			else
				temp = temp->next;
				
		}
	}		
}


//------------------------------------------------------------------------------------------------
void ManageReadyQueue()
{
	
}

//------------------------------------------------------------------------------------------------
void PrintReadyQueue()
{
	
}


//------------------------------------------------------------------------------------------------
void PrintBlockQueue()
{
	
}


//------------------------------------------------------------------------------------------------
bool CheckCompletion()
{
	if (readyQueue == NULL && blockQueue == NULL && Puncompleted == NULL)
		return true;
	return false;
}







//------------------------------------------------------------------------------------------------
int main()
{
	
	Puncompleted = CreateElements();
	printLL(Puncompleted);
	
	for(int counter = 0;/*Empty*/;++counter)
	{
		ManageArrival(counter);
		ManageBlockQueue(); //Called from the above method
		ManageReadyQueue();
		PrintBlockQueue();
		PrintReadyQueue();
		if(CheckCompletion())
			break;
	}
	
	return 0;
}