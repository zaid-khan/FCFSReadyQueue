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
static int waitingTimeBlocked = 0;
static int waitingArrivalTime = 0;
static int counter;
bool STSExecutionComplete;
bool hasSTSExecuted = false;
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
Process *PinitialList = NULL;
Process *Pcompleted = NULL;
Process *running = NULL;
Process *STS = NULL;


/*Method Declaration*/
Burst * CreateBursts(Process *);
void ManageBlockQueue(int);
void ManageReadyQueue(int);
void ManageRunning(int);
void PrintBlockQueue();
void PrintReadyQueue();
void PrintRunning();
void PrintCompleted();
void ManageArrival(int);




//Methods which have declared and defined together.
bool CheckCompletion()
{ 
	if (readyQueue == NULL && blockQueue == NULL && running == NULL && PinitialList == NULL)
		return true;
	return false;
}



////////////////////////*********IMPORTANT******************/////////////////////////////////////
////////////////////////*********IMPORTANT******************/////////////////////////////////////
////////////////////////*********IMPORTANT******************/////////////////////////////////////
/*The below defined methods are all methods which perform the different functionality of a STS.*/
//---------------------------------------------------------------------------
bool STSisCPUFree() 
{
	if (running == NULL)
		return true;
	else
		return false;
}

//------------------------------------------------------------------------------------------------
void STSManageTheTime(int cnt, Burst *t)
{
	t->starttime = cnt;
	t->endtime = cnt + t->bTime;
}


//-----------------------------------------------------------------------------------------------*
void STSTransferToCPU(Process *temp, int cnt)
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
	STSManageTheTime(cnt, t);
	printf("\nProcess P%d will now start its execution.", temp->data.pID);
}




//------------------------------------------------------------------------------------------------*
void STSTransferToReadyQueue(Process *temp, int cnt)
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
	printf ("\nProcess P%d is finished.", temp->data.pID);	
}


//------------------------------------------------------------------------------------------------*
void STSAddToBlockQueue(Process *temp, int cnt)
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
	STSManageTheTime(cnt, t);
	temp->next = NULL;
	printf("\nProcess P%d has been added to the block Queue.", temp->data.pID);
}


//------------------------------------------------------------------------------------------------*
Process * STSRemoveFromBlockQueue(Process *toreturn)
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



//-------------------------------------------------------------------------------------------
bool STSProcess()
{
	if (!STSExecutionComplete)
	{
		if (STSisCPUFree())
		{
			running = STS;
			STSExecutionComplete = true;
			counter++;      //global DON'T CHANGE
			printf ("\nSTS is under execution.");
			return true;
		}
		else
			return false;
	}
	return true;
}


//-------------------------------------------------------------------------------------------
void RemoveSTS()
{
	if (running == STS)
	{
		printf ("\nSTS will now stop execution.");
		running = NULL;
		hasSTSExecuted = true; //Previously, it has executed.
		STSExecutionComplete = false; //Completion status from the previous counter is removed.
	}
	else
	{
		hasSTSExecuted = false;
	}
}


//***************************END OF STS METHODS******************************************
//*********************//////////////////////////////////////////////////////////////////



/*Method Definition*/
//Process related to simple creation and print of processes.
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

//End of those processes
//-----------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
void ManageArrival(int cnt)
{
	if (PinitialList != NULL)
	{
		Process *temp = PinitialList;
		while (temp != NULL)
		{
				//if (temp->data.aTime == counter)
				//{ 
					//Print about the newly arrived process.
					if (STSProcess())
					{
						if (temp->data.aTime + 1 <= counter)
						{
						
							printf ("\nIn process of adding the newly arrived Process P%d", temp->data.pID);
							PinitialList = temp->next; //We're removing the current process from the initial list i.e. PinitialList, so the PinitialList 
						//head will point at the next element in the list
						//printf ("\nProcess P%d has been added to the block queue at %dms.", temp->data.pID, cnt);
						//Calling STS
							STSAddToBlockQueue(temp, counter); //ACTUAL INCREMENTED COUNTER
							temp = PinitialList; //temp will now point at the next element in the list as the current element has been removed
						//break; //Assuming two processes won't arrive at the same time, i.e. their arrival time differs.
						}
						else
						{
							printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, cnt);
							temp = temp->next;
						}
						
					}
					
						         //If NOT Ready then print conf. that process
						           //won't be added in the next burst. 
					else                         //If CPU is not free and occupied by some process.
					{
						if (temp->data.aTime == counter)
							printf ("\nProcess %d has arrived but won't be added since STS is not running", temp->data.pID);
	
						else
							printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, cnt);
	
						temp = temp->next;
						//temp->data.aTime++;
					}
                    // if (temp->data.aTime + 1 == counter)
                    // {
					// 	printf ("\nProcess P%d has arrived at %dms.", temp->data.pID, temp->data.aTime);
						

						
					// }
					// else

                    
                    //Check whether the STS Process is 
                    //ready or not if Ready, then print
                    //confirmation that the newly arrived process will be added in 
                    //the next burst.
                    
					
					//return;
				//}
				
				//if (temp->data.aTime + 1 == counter)
				//{
	//				if (hasSTSExecuted && temp->data.aTime < cnt) //STS has executed before and arrival time 
	//				{
						
	//				}
	//				else //STS Process can't be executed
	//				{	
						//printf ("\nError. STS has NOT been executed before.");
	//					return; //RETURN DIRECTLY, DON'T WAIT.
	//				}
				//}
		}
			// else if ((temp->data.aTime == counter) && !STSProcess())
			// {
			// 	printf("\nProcess P%d has not yet arrived at %dms", temp->data.pID, cnt);
			// 	printf("\nBut STS is not Free to add it to the Block Queue.");
			// 	temp = temp->next;
			// }
			// else
			// {
				
			// }
	}
	
}





//------------------------------------------------------------------------------------------------
void ManageBlockQueue(int cnt)
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
				if ((t->starttime != -1) && (t->endtime + 1 >= counter)) //Counter should be less than or equal to the second after end time
				{	
					
					if (t->starttime < cnt && ((t->endtime) > counter)) //Old - t->endtime > counter
						printf("\nProcess P%d is currently in the Block Queue doing I/O.", temp->data.pID);
					//else if (t->starttime == counter)
					//	printf("\nProcess P%d has just been added to the Block Queue for doing I/O.", temp->data.pID);
					//else if (t->endtime == counter) //Old - "else if (t->endtime == counter)"
					//	printf("\nProcess P%d is finishing up doing I/O in the BlockQueue.", temp->data.pID);
					else if (t->endtime + 1 == counter)  // + 1cnt is different from counter.
					{
						if (STSProcess())
						{
							// if ((t->endtime + 1 == cnt)) //STS Process has now surely been executed so + 1
							// {											 //So the counter has been incremented	
								para = temp->next;

								if (t->next == NULL)
									STSTransferToCompletedQueue(STSRemoveFromBlockQueue(temp), counter);

								else
									STSTransferToReadyQueue(STSRemoveFromBlockQueue(temp), counter);
										/*I have disabled direct transition from blocked to running instead I'll add
										to the ready queue for that second(counter) and if the CPU is free at that second/coun.
										it will get transitioned from there in ManageReadyQueue() */
								
								tomove = true;	//tomove flag set to true
								//Process needs moving to another list
								break; //No point in further checking next I/O burst of that process so break.
								//t->endtime++;
								//waitingTimeBlocked++;
								//printf ("\nCPU is busy at this moment as STS is running.");
								//printf ("\n--Process P%d will now be transferred FROM the Block Queue.", temp->data.pID);
								
							// }
						}
						
						
						//else if (STSProcess() && (t->endtime + 1 == cnt))
						//{
						
						//	printf("\nProcess P%d has exited the BlockQueue.", temp->data.pID);
						
							//para is a temporary variable to store the value of the next 
							//process since the current process will be removed from the list and
							//will be part of another list.
							
							//**Remember we're removing processes from list we're traversing**
							
							
							
							
							
						//}
						
					}
					else if(!STSProcess() && (t->endtime == counter))
					{
						// if ((t->endtime == cnt)) //STS Process is executing in current time.
						// {											 //So the counter has been incremented	
							//t->endtime++;
							waitingTimeBlocked++;
							//printf ("\nCPU is busy at this moment as STS is running.");
							t->endtime++;
							//waitingTimeBlocked++;
							//printf ("\nCPU is busy at this moment so STS can't run.");
							printf ("\n--Process P%d will now stay in the Block Queue.", temp->data.pID);
							printf ("\n---Endtime extended to %d.", t->endtime);
							break;
							//return; DON'T RETURN AS THE ENDTIME OF OTHER PROCESSES IN BLOCKED QUEUE NEED TO BE EXTENDED
							//printf ("\n--Process P%d will now be transferred FROM the Block Queue.", temp->data.pID);
							
						// }
						
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
void ManageReadyQueue(int cnt)
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
				if ((t->starttime != -1) && (t->endtime < cnt)) //IOBurst has been completed of that process ......odd bursts
				{												//At least 1 second(for STS) should pass 
																//since that completion. Therefore the greater sign for counter.
					if ((t->next != NULL) && ((t->next)->starttime == -1)) //There "exists" a CPU Burst after the 
					{												       //IO burst which has NOT been started
						if (!hasSTSExecuted) //If the STS has NOT been executed previously 
						{									 //so STSProcess() will execute or return true is still executing
							waitingTime++;
							printf("\nProcess P%d is still in the Ready Queue as the CPU is not free.", temp->data.pID);
							break;
						}
						else if(STSisCPUFree() && hasSTSExecuted)    //CPU must be free as well as STS has been executed in the previous
						{											 //burst to move the process from ready to running
							para = temp->next; //para is here
							printf("\nCPU is free right now.");
							STSTransferToCPU(temp, cnt);
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
	else
		STSProcess(); //No process to run so execute the STS.
}



//------------------------------------------------------------------------------------------------
void ManageRunning(int cnt)
{
	if(running != NULL && running != STS)
	{
		Process *temp = running;
		//Process *para;  
		Burst *t = temp->data.pBurstList->next; //Atleast 1 CPU Burst exists for each process so we can directly point towards it
		do
		{
			if ((t->starttime != -1) && (t->endtime >= cnt)) //The second after the last sec of the CPU burst should be greater than or
			{													     //equal to current time because of count + 0.99, which I have put in main().
				if (t->starttime < cnt && ((t->endtime) > cnt))
				{
					printf("\nProcess P%d is currently executing.", temp->data.pID);
					return; //Return as there is a process already running so no need to check other processes.
				}
				//else if (t->starttime == counter)
				//	printf("\nProcess P%d has just started execution.", temp->data.pID);
				//else if () //Old - else if (t->endtime == counter)
				//	printf("\nProcess P%d is finishing execution.", temp->data.pID);
				else if (t->endtime == cnt) //counter has exceeded the endtime by 1 Old - else if (t->endtime + 1 == counter)
				{
					printf("\nProcess P%d execution is over. It will free the CPU now.", temp->data.pID);
					running = NULL;
					STSProcess();
					STSAddToBlockQueue(temp, counter);
					return;
					
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
	int cnt = 0;
	printf ("\n\n------The Block Queue-------");
	while (temp != NULL)
	{
		++cnt;
		printf ("\n\t%d. Process P%d. ", cnt, temp->data.pID);
		Burst *t = temp->data.pBurstList;
		while (t != NULL)
		{
			if ((t->starttime <= counter) && (counter <= t->endtime))
			{
				printf ("(%d - %d)", t->starttime, t->endtime);
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
	if(cnt == 0)
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
	{
		if (temp->data.pID == -1)
			printf ("\n\t>> STS <<", temp->data.pID);
		else
		{
			printf ("\n\t>> Process P%d", temp->data.pID);
			Burst *t = temp->data.pBurstList->next;
			while (t != NULL)
			{
				if (t->starttime <= counter && t->endtime >= counter)
				{
					printf (" (%d - %d)", t->starttime, t->endtime);
					break;
				}
				t = t->next->next;
			}
			printf (" <<");
		}
		
	}
	else
		printf ("\nNo process is currently under execution.");
	printf ("\n--------------------------------");
}




//------------------------------------------------------------------------------------------------
int main()
{
	char ch;

    PinitialList = CreateElements();
	printLL(PinitialList);
	STS = (Process *)malloc(sizeof(Process)); //I have created a dummy process
	STS->data.pID = -1;
	STS->data.aTime = -1;
	STS->data.nBurst = -1;
	STS->data.pBurstList = NULL;
	STS->next = NULL;
	
	
	
	printf("\n-----------------------------------------------------------------------------");
	printf("\nTO STOP EXECUTION PRESS THE ; KEY AND THEN PRESS ENTER.");
	printf("\nTO CONTINUE JUST PRESS ENTER.");
	printf("\nASSUMPTION: STS takes a unit of time to execute.");
	printf("\n-----------------------------------------------------------------------------");
	for(counter = 0;/*Loops breaks from the body itself so this part is left empty*/;++counter)
	{   
		//STSExecutionComplete = false;
		
		
		printf("\n\n\n\n================Counter %dms-%0.2fms======================================", counter, (float)counter + 0.99);
		ManageReadyQueue(counter); //Preference for Ready to Running transition
		ManageRunning(counter);
		ManageArrival(counter);
		ManageBlockQueue(counter); 
		//if (STSExecutionComplete)
		//		printf("\nNOTE: STS took a unit of time to execute, so counter = %d.", counter + 1);
		printf("\n-------------------------------------------------------------------------------\n");
		PrintBlockQueue();
		PrintReadyQueue();
		PrintRunning();
		PrintCompleted();
		if (STSExecutionComplete)
			counter--;
		RemoveSTS();
		printf("\n========================================================================\n");
		scanf("%c", &ch);
		if(CheckCompletion() || (ch == ';'))   //if(CheckCompletion()) 
			break;
	}
	return 0;
}
