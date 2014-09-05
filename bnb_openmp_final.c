    //OpenMP for BnB kanpsack
     
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#define MAXTHREAD 1000
#include "bnb_openmp_dataDef.h"
#define BUFSIZE 5000              //3 per no.so this can take 20nos


int QueueSize = 0;
double curBest;
ival ValArray;
iweight WtArray;
int WtCap;
int asize;

int QueueSizeTSP = 0;
double curBestTSP = 10000;

	int adjMatrix[5][5] = {{0, 1, 1, 1, 1},
						 {1, 0, 1, 1, 1},
						 {1, 1, 0, 1, 1},
						 {1, 1, 1, 0, 1},
						 {1, 1, 1, 1, 0}};

	int costMatrix[5][5] = {{0, 3, 4, 2, 7},
						 {3, 0, 4, 6, 3},
						 {4, 4, 0, 5, 8},
						 {2, 6, 5, 0, 6},
						 {7, 3, 8, 6, 0}};

/*TODO: Deletion scheme without garbage collection can cause memory issues for large datasets*/
/////FIFO Queue///////
///////////////////
Qnode qn;
QnodeTSP q;

/*KNAPSACK HELPER FUNCTIONS*/
void initQueue()
{
	qn = calloc(1, sizeof(_Qnode));
	qn->data.selectarray = NULL;
	qn->next = NULL;
	//return  qn;
}

int isEmpty(Qnode head)
{
	if(head->data.selectarray == NULL)
		return 1;
	else
		return 0;
}

int inserttoQueue(Qnode head, SSnode inpdata)
{
	if(QueueSize == 0)
	{
		int k;
		for(k = 0; k<asize; k++)
		{
			head->data.selectarray[k] = inpdata->selectarray[k];
		}
		head->data.feas_flag = inpdata->feas_flag;
		head->next = NULL;
		QueueSize++;
		return 1;
	}
	else
	{
		Qnode temp = head;
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = calloc(1, sizeof(_Qnode));
		temp = temp->next;
		temp->data.selectarray = calloc(asize, sizeof(int));
		//int tempArr1[asize];// = calloc(5, sizeof(subselect));
		int k;
		for(k = 0; k<asize; k++)
		{
			temp->data.selectarray[k] = inpdata->selectarray[k];
		}
		temp->data.feas_flag = inpdata->feas_flag;
		temp->next = NULL;
		QueueSize++;
		return 1;      
	}
	return 0; //failed to insert
}


int getQueueSize()
{
	return QueueSize;
}

Qnode removefromQueue()
{
	if(QueueSize == 0)
	{
		printf("#############################ERROR: Queue is empty!##############################");
		exit(0);
	}
	if(qn->next == NULL)
	{
		//printf("In here\n");
		QueueSize--;
		//qn->data.selectarray = NULL;
		return qn;
	}
	else
	{
		//printf("Inside else\n");
		Qnode temp = qn;
		//temp->data = head->data;
		qn = qn->next;
		temp->next = NULL;
		QueueSize--;
		return temp;
	}
}
void printQueue(Qnode q)
{
	//Prints the contents of the queue
	//Qnode temp = calloc(1,sizeof(_Qnode));
	Qnode temp = q;
	printf("\nPrinting the queue\n\n");
	if(QueueSize == 0)
	{
		printf("Queue is empty!\n");
		return;
	}
	else
	{
		while(temp != NULL)
		{
			int k;
			printf("Node selectarray:\t");
			for(k = 0; k<asize; k++)
			{
				printf("%d\t", temp->data.selectarray[k]);
			}
			printf("\tFeasible flag:%d\n", temp->data.feas_flag);
			temp = temp->next;
		}
	}
}
//////////////////////////////
///
///     READ INPUT DATA
///
////////////////////////////

void getDataTSP()
{
	char* tok;
	int i;
	FILE* f = fopen("tspinp.txt","r");
	
}

void getDataKS()
{
	
	char* tok;
	int i;
	FILE* f = fopen("queueinp.txt","r");
	
	char* str = malloc(sizeof(char)*BUFSIZE);
	//get size of array
	fgets(str,BUFSIZE,f);
	asize = atoi(str);
	
	//get max wt
	str = malloc(sizeof(char)*BUFSIZE);
	fgets(str, BUFSIZE, f);
	WtCap = atoi(str);
	
	
	//get Values
	str = malloc(sizeof(char)*BUFSIZE);
	fgets(str, BUFSIZE, f);
	ValArray = malloc(sizeof(int)*asize);
	i=0;
	char* buf= str;
	while((tok = strsep(&buf,",")) != NULL)
	{      
		if(i<asize)
			ValArray[i++] = atoi(tok);
		else
			//printf("\n\x1b[31mMismatch. More values entered than capacity.\x1b[0m\n");
			printf("\nMismatch. More values entered than capacity.\n");
	}
	
	//get Weights
	str = malloc(sizeof(char)*BUFSIZE);
	fgets(str, BUFSIZE, f);
	WtArray = malloc(sizeof(int)*asize);
	i=0;
	buf= str;
	while( (tok = strsep(&buf,",")) != NULL)
	{      
		if(i<asize)
			WtArray[i++] = atoi(tok);
		else
			printf("\nMismatch. More wts entered than capacity.\n");
	}
	
	
	printf("size %d\n", asize);
	printf("max wt %d\n", WtCap);
	for(i=0;i<asize;++i)
	{
		printf("%d ", ValArray[i]);
	}
	printf("\n");
	
	for(i=0;i<asize;++i)
	{
		printf("%d ", WtArray[i]);
	}
	printf("\n");
	
}
//////////////////////////////
///
/// MAIN OPERATIONS
///
////////////////////////

int getWeightforSoln(int* solnvector, int index)
{
	int wt = 0;
	int k;
	for(k = asize - 1; k>=0; k--)
	{
		if(solnvector[k] == 1)
			wt +=  WtArray[k];	//Add all the definitely included items first
	}
	if(wt > WtCap)
	{
		return -1;
	}	
	for(k = asize-1; k>=index; k--)
	{
		if(solnvector[k] == 2)
			wt += WtArray[k];				//Add all the undefined items down to the current index
	}
	return wt;
}

int getValueforSoln(int* solnvector, int index)
{
	int val = 0;
	int k;
	for(k = asize - 1; k>=0; k--)
	{
		if(solnvector[k] == 1)
			val +=  ValArray[k];	//Add all the definitely included items first
	}
		
	for(k = asize-1; k>=index; k--)
	{
		if(solnvector[k] == 2)
		val += ValArray[k];				//Add all the undefined items down to the current index
	}
	return val;
}

boundReturn getBound(SSnode node)
{
	int i = asize; //index starts from end
	double frac;
	double temp;
	double bound = getWeightforSoln(node->selectarray, i);
	boundReturn ret;
	ret.sack = calloc(asize, sizeof(int));
	while(i>=0)
	{
		//printf("BOUND: %f\n", bound);
		if(bound == WtCap)
		{
			//we are done
			// printf("INSIDE EQUAL\n");

			double maxVal = getValueforSoln(node->selectarray, i);
			//printf("Calculated maxVal!\n");
			
			ret.fractionalIndex = i;
			//printf("INSIDE EQUAL:\n FLAG:%d\tVALUE UPPER BOUND:%f\n", ret.flag, ret.maxVal);
			if(frac == -0.1f)
			{
				ret.flag = FEASIBLE;
				ret.maxVal = maxVal;
				int k;
				for(k = asize - 1; k>=0; k--)
				{
					if(node->selectarray[k] == 1)
						ret.sack[k] = 1;
				}
				for(k = asize-1; k >= i; k--)
				{
					if(node->selectarray[k] == 2)
						ret.sack[k] = 1;
				}
			}
			else
			{
				ret.maxVal = maxVal + temp - ValArray[i];
				ret.flag = FEASIBLE;
			}
			temp = 0;
			return ret;
		}
		else if(bound > WtCap)
		{
			// printf("INSIDE GREATER \n");
			//need to exclude some. which?
			frac = 1 - (bound - WtCap) / WtArray[i];
			temp = frac * ValArray[i];
			node->selectarray[i] = 0;
			bound = WtCap;
			//try excluding the current index package
			continue;
		}
		else
		{      
			// printf("INSIDE LESS\n");
			--i;
			//node->selectarray[i] = SS_INCLD;
		}
		bound = getWeightforSoln(node->selectarray, i);
		if(bound == -1)
		{
				ret.maxVal = -1;
				ret.flag = OVERUSE;

				return ret;
		}
		//printf("WEIGHT CALCULATED IS %f\n", bound);
		frac = -0.1f;
	}
}
/*END OF KNAPSACK HELPER FUNCTIONS*/

/*TSP HELPER FUNCTIONS*/
intPair find2SmallestEdges(int subP[][5], int index)
{
	int i;
	int min1 = 10000, min2 = 10000;
	intPair ret;
	for(i = 0; i<5; i++)
	{
		if((subP[i][index] == 1) && costMatrix[index][i] < min1)
		{
			min1 = costMatrix[index][i];
		}
	}
	for(i = 0; i<5; i++)
	{
		if((subP[i][index] == 1) && (costMatrix[index][i] < min2) && (costMatrix[index][i] != min1))
		{
			min2 = costMatrix[index][i];
		}
	}
	if(min1 == 10000)
	{
		for(i = 0; i<5; i++)
		{
			if(costMatrix[index][i] < min1)
			{
				min1 = costMatrix[index][i];
			}
		}
		for(i = 0; i<5; i++)
		{
			if((costMatrix[index][i] < min2) && (costMatrix[index][i] != min1))
			{
				min2 = costMatrix[index][i];
			}
		}
	}
	else if(min2 == 10000)
	{
		for(i = 0; i<5; i++)
		{
			if((subP[i][index] != 1) && (costMatrix[index][i] < min2))
			{
				min2 = costMatrix[index][i];
			}
		}
	}

	ret.num1 = min1;
	ret.num2 = min2;
	return ret;
}
boundReturnTSP getBoundTSP(_ssnodeTSP node)
{
	/*Finds the lower bound for the TSP path as half of the sum of two edges of least cost per vertex*/
	double bound = 0;
	boundReturnTSP ret;
	int a, b, y, count;
	ret.flag = FEASIBLE;
	for(a = 0; a<5; a++)
	{
		for(b = 0; b<5; b++)
		{
			if(node.subP[a][b] == 2)
			{
				ret.flag = NOTFEASIBLE;
			}
		}
	}
	for(y = 0; y<5; y++)
	{		
			intPair pair = find2SmallestEdges(node.subP, y);
			bound += (costMatrix[y][pair.num1] + costMatrix[y][pair.num2]);
	}					
	ret.maxVal = bound/2;
	return ret;
}
void initQueueTSP()
{
	q = calloc(1, sizeof(_QnodeTSP));
	q->next = NULL;
	//return  qn;
}


int inserttoQueueTSP(QnodeTSP head, SSnodeTSP inpdata)
{
	if(QueueSizeTSP == 0)
	{
		int k, l;
		for(k = 0; k<5; k++)
		{
			for(l = 0; l<5; l++)
			{
				head->data.subP[k][l] = inpdata->subP[k][l];
			}
		}
		head->data.feas_flag = inpdata->feas_flag;
		head->next = NULL;
		QueueSizeTSP++;
		return 1;
	}
	else
	{
		QnodeTSP temp = head;
		while(temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = calloc(1, sizeof(_QnodeTSP));
		temp = temp->next;
		//q->data.selectarray = calloc(asize, sizeof(int));
		//int tempArr1[asize];// = calloc(5, sizeof(subselect));
		int k, l;
		for(k = 0; k<5; k++)
		{
			for(l = 0; l<5; l++)
			{
				temp->data.subP[k][l] = inpdata->subP[k][l];
			}
		}
		temp->data.feas_flag = inpdata->feas_flag;
		temp->next = NULL;
		QueueSizeTSP++;
		return 1;      
	}
	return 0; //failed to insert
}

QnodeTSP removefromQueueTSP()
{
	if(QueueSizeTSP == 0)
	{
		printf("#############################ERROR: Queue is empty!##############################");
		exit(0);
	}
	if(q->next == NULL)
	{
		//printf("In here\n");
		QueueSizeTSP--;
		//qn->data.selectarray = NULL;
		return q;
	}
	else
	{
		//printf("Inside else\n");
		QnodeTSP temp = q;
		//temp->data = head->data;
		q = q->next;
		temp->next = NULL;
		QueueSizeTSP--;
		return temp;
	}
}
/*END OF TSP HELPER FUNCTIONS*/
int main(int argc, char* argv[])
{
	//get input data
	int choice;
	printf("Enter choice of problem:\n1: 0/1 Knapsack\n2:Travelling Salesman Problem\n");
	scanf("%d", &choice);
	if(choice == 1)
	{
	printf("*********0/1 Knapsack problem***********\n");
	getDataKS();
	int initArray[asize];
	int i;
	for(i=0;i<asize;i++)
	{
		initArray[i] = 2;
	}
	
	initQueue();
	
	qn->data.selectarray = initArray;
	qn->data.feas_flag = NOTFEASIBLE;
	QueueSize++;
	int *bestSack = calloc(asize, sizeof(int));
	curBest = 0.0f;
	while(QueueSize > 0)
	{
		//printQueue(qn);
		//printf("entered while\n");
		int nn = getQueueSize();
		int nt = (nn > MAXTHREAD)?MAXTHREAD:nn;
		
		int j;
		_ssnode dataArr[nt];
		for (j = 0; j < nt; j++)
		{
			Qnode dnode = removefromQueue();
			int p;
			dataArr[j].selectarray = calloc(asize, sizeof(int));
			for(p = 0; p<asize; p++)
			{
				dataArr[j].selectarray[p] = dnode->data.selectarray[p];
			}
			dataArr[j].feas_flag = dnode->data.feas_flag;
		}
		omp_set_num_threads(nt);
		
		#pragma omp private(ret) parallel for
		for(j = 0; j < nt; j++)
		{
			int k;
			boundReturn ret = getBound(&dataArr[j]);
			if(ret.maxVal == -1) continue;
			printf("Entered the for loop for the %d time. Bound calculated: %f \n", j, ret.maxVal);
			if(ret.flag == FEASIBLE)
			{      
				//printf("Inside feasible check\n");
				#pragma omp critical
				{
					if(ret.maxVal > curBest)
					{
						
						curBest = ret.maxVal;
						printf("Updating Current Best to %f!\n", curBest);
						int k;
						for(k = 0; k<asize; k++)
						{
							bestSack[k] = ret.sack[k];
						}
					}
				}
			}
			else if(curBest == 0)
			{
				//Branch on fractional index
				#pragma omp critical
				{
					printf("Branching with NO feasible solution yet!\n");
					SSnode n1 = calloc(1,sizeof(_ssnode));
					SSnode n2 = calloc(1,sizeof(_ssnode));
					n1->selectarray = calloc(asize, sizeof(int));
					n2->selectarray = calloc(asize, sizeof(int));
					int k;

					for(k = 0; k<asize; k++)
					{
						n1->selectarray[k] = dataArr[j].selectarray[k];
						n2->selectarray[k] = dataArr[j].selectarray[k];
					}
				
					n1->selectarray[ret.fractionalIndex] = 1;
					n1->feas_flag = NOTFEASIBLE;
					inserttoQueue(qn, n1);
					//printQueue(qn);

					n2->selectarray[ret.fractionalIndex] = 0;
					n2->feas_flag = NOTFEASIBLE;
					inserttoQueue(qn, n2);
					//printQueue(qn);

				}
				
			}
			else if(curBest != 0 && ret.maxVal > curBest)
			{
				//Branch at fractional index
				#pragma omp critical
				{      
					printf("Branching WITH a current feasible solution!\n");
					SSnode n1 = calloc(1, sizeof(_ssnode));
					SSnode n2 = calloc(1, sizeof(_ssnode));
					
					int tempArr1[asize];// = calloc(5, sizeof(subselect));
					int tempArr2[asize];
					int k;
					for(k = 0; k<asize; k++)
					{
						tempArr1[k] = dataArr[j].selectarray[k];
						tempArr2[k] = dataArr[j].selectarray[k];
					}
					
					n1->selectarray = tempArr1;
					n1->selectarray[ret.fractionalIndex] = 1;
					n1->feas_flag = NOTFEASIBLE;
					inserttoQueue(qn, n1);
					//printQueue(qn);
					
					n2->selectarray = tempArr2;
					n2->selectarray[ret.fractionalIndex] = 0;
					n2->feas_flag = NOTFEASIBLE;
					inserttoQueue(qn, n2);
					//printQueue(qn);;
				}
			}
			else //Discard
			{
				//printf("Inside discard else block!\n");
				//...here goes nothing..                               
			} 
			//printQueue(qn);
		}
		
	}	
	printf("Maximum value of knapsack:%f\nItems in the knapsack:", curBest);
	int k;
	for(k = 0; k<asize; k++)
	{
		if(bestSack[k] == 1)
			printf("%d ", k+1);
	}
	printf("\n");
	}
	/*TSP BEGINS HERE*/
	else if(choice == 2)
	{

	printf("*********TSP Problem***********\n");
	initQueueTSP();
	int e, f;
	for(e = 0; e<5; e++)
	{	int f;
		for(f = 0; f<5; f++)
		{
			q->data.subP[e][f] = 2;
		}
	}
	q->data.feas_flag = NOTFEASIBLE;
	QueueSizeTSP++;
	while(QueueSizeTSP > 0)
	{
		//printQueue(qn);
		printf("Entered while\n");
		int nn = QueueSizeTSP;
		int nt = (nn > MAXTHREAD)?MAXTHREAD:nn;
		
		int j;
		_ssnodeTSP dataArr[nt];
		for (j = 0; j < nt; j++)
		{
			QnodeTSP dnode = removefromQueueTSP();
			int p;
			int y;
			for(y = 0; y<5; y++)
			{	int z;
				for(z = 0; z<5; z++)
				{
					dataArr[j].subP[y][z] = dnode->data.subP[y][z];
				}
			}
			dataArr[j].feas_flag = dnode->data.feas_flag;
		}
		omp_set_num_threads(nt);
		#pragma omp private(retTSP) parallel for
		for(j = 0; j < nt; j++)
		{
			printf("Finding bound for the node:\n");
			int y;
					for(y = 0; y<5; y++)
					{	int z;
						for(z = 0; z<5; z++)
						{
							printf("%d ", dataArr[j].subP[y][z]);
						}
						printf("\n");
					}
			boundReturnTSP retTSP = getBoundTSP(dataArr[j]);
			printf("Entered the for loop for the %d time. Bound calculated: %f \n", j, retTSP.maxVal);
			if(retTSP.flag == FEASIBLE)
			{      
				printf("Inside feasible check\n");
				#pragma omp critical
				{
					if(retTSP.maxVal < curBestTSP)
					{
						printf("Updating Current Best to %f!\n", curBest);
						curBestTSP = retTSP.maxVal;
					}
				}
			}
			else if(curBestTSP == 10000)
			{
				//Branch on fractional index
				#pragma omp critical
				{
					printf("Branching with NO feasible solution yet!\n");
					SSnodeTSP n1 = calloc(1,sizeof(_ssnodeTSP));
					SSnodeTSP n2 = calloc(1,sizeof(_ssnodeTSP));
					int k;
					int y;
					for(y = 0; y<5; y++)
					{	int z;
						for(z = 0; z<5; z++)
						{
							(n1->subP[y][z]) = (dataArr[j].subP[y][z]);
							(n2->subP[y][z]) = (dataArr[j].subP[y][z]);
						}
					}
					int flag = 0;
					for(y = 0; y<5; y++)
					{	int z;
						for(z = 0; z<5; z++)
						{
							if(dataArr[j].subP[y][z] == 2)
							{
									n1->subP[y][z] = 1;
									n2->subP[y][z] = 0;
									flag = 1;
									//printf("n1->subP[%d][%d]:%d", y, z, n1->subP[y][z]);
									
									break;
							}
						}
						if(flag == 1) break;
					}
					printf("n1->subP[4][1]:%d", n1->subP[4][1]);
					flag = 0;
					n1->feas_flag = NOTFEASIBLE;
					inserttoQueueTSP(q, n1);

					//n2->subP[][] = 0;	//DEAL
					n2->feas_flag = NOTFEASIBLE;
					inserttoQueueTSP(q, n2);
					//printQueue(qn);
				}	
			}
			else if(curBestTSP != 10000 && retTSP.maxVal < curBestTSP)
			{
				//Branch at fractional index
				printf("Branching WITH a current feasible solution!\n");
				#pragma omp critical
				{
					SSnodeTSP n1 = calloc(1,sizeof(_ssnodeTSP));
					SSnodeTSP n2 = calloc(1,sizeof(_ssnodeTSP));
					int k;
					int y;
					for(y = 0; y<5; y++)
					{	int z;
						for(z = 0; z<5; z++)
						{
							n1->subP[y][z] = dataArr[j].subP[y][z];
							n2->subP[y][z] = dataArr[j].subP[y][z];
						}
					}
					int flag = 0;
					for(y = 0; y<5; y++)
					{	int z;
						for(z = 0; z<5; z++)
						{
							if(dataArr[j].subP[y][z] == 2)
							{
									n1->subP[y][z] = 1;
									n2->subP[y][z] = 0;
									flag = 1;
									//printf("n1->subP[%d][%d]:%d", y, z, n1->subP[y][z]);
									break;
							}
						}
						if(flag == 1) break;
					}

					flag = 0;
					n1->feas_flag = NOTFEASIBLE;
					inserttoQueueTSP(q, n1);

					//n2->subP[][] = 0;	//DEAL
					n2->feas_flag = NOTFEASIBLE;
					inserttoQueueTSP(q, n2);
					//printQueue(qn);

				}
			}
			else //Discard
			{
				printf("Inside discard else block!\n");
				//...here goes nothing..                               
			} 
			//printQueue(qn);
		}

	}
	printf("Optimum cost of TSP path is %f\n", curBestTSP);
	}
	else 
		printf("Invalid choice\n");

	printf("*************END***************\n");
	return 0;
}
