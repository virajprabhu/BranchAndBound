//MPI for knapsack
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 60     //3 per no.so this can take 20nos in an input line 
#define ASIZE 5 //no of elems in input array
//create structs
// 2 datatype.s
// sendtoslave type
// bestSol value, curSolvector, cur problem(fractional index), [dest rank slave]
// float bestSol, Sarray solnvector, problem: {fracIndex, maxVL, feas flag}
// 
// sendtomaster type
// bestSol val, curSol vecotr, tag of status/request, [master rank]
// float bestSol, Sarray solnvector, 

//
//bound calc on dataArr[j] elem which is passed to the worker
//flag is feasible?
//		maxval > curBest? ==> [update]
//else if
//	curBest == 0? ==> branch on frac index.
//else if
//	curBest !=0 && maxval > curBest? ==> branch on frac index.
//else
//	discard.

 ////////HEADER///////////
 typedef enum
 {
         SS_EXCLD,
         SS_INCLD,
         SS_UNDEF
 }subselect;
 
 typedef enum
 {
         NOTFEASIBLE,
         FEASIBLE
 }feasibleflag;
 
 typedef enum 
 {
 		IDLE,
 		BUSY
 }workerstatus;

 typedef enum 
 {
 	BRANCH_TAG,
 	UPDATE_TAG,
 	PBM_TAG,
 	DONE_TAG
 } statustags;

 typedef int* ival;              //shared
 typedef int* iweight;   //shared
 typedef subselect* Sarray;      //non shared. each thread has one
 
 typedef struct _ssnode _ssnode;
 struct _ssnode
 {
    Sarray selectarray;
    feasibleflag feas_flag;
 };
 
 typedef struct _ssnode* SSnode;
 
 typedef struct _Qnode _Qnode;
 typedef struct _Qnode* Qnode;
 struct _Qnode
 {
         _ssnode data;
         Qnode next;
 };
 
 
 typedef struct boundReturn boundReturn;
 struct boundReturn
 {
         int fractionalIndex;
         feasibleflag flag;
         float maxVal;         
 };

typedef struct _inpdat inpPack;
struct _inpdat
{
	ival ValArray;
	iweight WtArray;
	int WtCap;
};

typedef struct _wrkdata wrkdata;
struct _wrkdata
{
	_ssnode soln;
	float bestVal;
	inpPack idata;
};



typedef struct _wrkrecv wrkrec;
struct _wrkrecv
{
	int* arr;
	int val;	
};

// typedef struct _slvrdata slvrecvdata;
// struct _slvrdata
// {
// 	float curval;
// 	int noSlaves;
// 	int* slvIds; //array of allocated slave ids
// };
///////HEADER ENDS///////

	ival ValArray;
	iweight WtArray;
	int asize;
	int WtCap;


/////FIFO Queue OPS///////
Qnode initQueue()
{
        Qnode qn = malloc(sizeof(_Qnode));
        qn->data.selectarray = NULL;
        qn->next = NULL;
        return  qn;
}

int isEmpty(Qnode head)
{
        if(head->data.selectarray == NULL)
                return 1;
        else
                return 0;
}
 
Qnode inserttoQueue(Qnode head, SSnode inpdata)
{
        if(head->data.selectarray == NULL)
        {
                head->data.selectarray = inpdata->selectarray;
                head->data.feas_flag = inpdata->feas_flag;
                head->next = NULL;
                // qsize++;
                return head;
        }
        else
        {
                Qnode qn = head;
                while(qn->next != NULL)
                {
                        qn = qn->next;
                }
                qn->next = malloc(sizeof(_Qnode));
                qn=qn->next;
                qn->data.selectarray = inpdata->selectarray;
                qn->data.feas_flag = inpdata->feas_flag;
                qn->next = NULL;
                // qsize++;
                return head;      
        }
        return NULL; //failed to insert
}
 
// int getQueueSize()
// {
//         return QueueSize;
// }

Qnode removefromQueue(Qnode head)
{
        if(head->next == NULL)
        {
                // qsize--;
                return head;
        }
        else
        {
                Qnode qn = head;
                head = head->next;
                // qsize--;
                return qn;
        }
}
 	
///////////////QUEUE OPS END///////////////


int getWeightforSoln(Sarray solnvector, int index)
{
        int wt = 0;
        int k;
        for(k = asize - 1; k>=0; k--)
        {
                if(solnvector[k] == SS_INCLD)
                        wt +=  WtArray[k];      //Add all the definitely included items first
        }
               
        for(k = asize-1; k>=index; k--)
        {
                if(solnvector[k] == SS_UNDEF)
                        wt += WtArray[k];       //Add all the undefined items down to the current index
        }
        return wt;
}
 
int getValueforSoln(Sarray solnvector, int index)
{

  	int val = 0;
    int k;
    for(k = asize - 1; k>=0; k--)
    {
            if(solnvector[k] == SS_INCLD)
                    val +=  ValArray[k];    //Add all the definitely included items first
    }
           
    for(k = asize-1; k>=index; k--)
    {
            if(solnvector[k] == SS_UNDEF)
            val += ValArray[k];                             //Add all the undefined items down to the current index
    }
    return val;
}
 
boundReturn getBound(SSnode node)
{
    int i = asize; //index starts from end
    float frac;
    float temp;
    float bound = getWeightforSoln(node->selectarray, i);
    while(i>=0)
    {
        //printf("INSIDE WHILE OF GET BOUND\n");
        if(bound == WtCap)
        {
                //we are done
            float maxVal = getValueforSoln(node->selectarray, i);
            //printf("Calculated maxVal!\n");
            boundReturn ret;
            ret.fractionalIndex = i;
            //printf("INSIDE EQUAL:\n FLAG:%d\tVALUE UPPER BOUND:%f\n", ret.flag, ret.maxVal);
            if(frac == -0.1f)
            {
                    ret.flag = FEASIBLE;
                    ret.maxVal = maxVal;
            }
            else
            {
                    ret.maxVal = maxVal + temp - ValArray[i];
                    ret.flag = NOTFEASIBLE;
            }
            temp = 0;
                return ret;
        }
        else if(bound > WtCap)
        {
            //printf("INSIDE GREATER \n");
            //need to exclude some. which?
            frac = 1 - (bound - WtCap) / WtArray[i];
           
            temp = frac * ValArray[i];
            bound = WtCap;
            //try excluding the current index package
            continue;
        }
        else
        {      
            //printf("INSIDE LESS\n");
            --i;
            //node->selectarray[i] = SS_INCLD;
        }
        bound = getWeightforSoln(node->selectarray, i);
        //printf("WEIGHT CALCULATED IS %f\n", bound);
        frac = -0.1f; //set as default value.
    }
    // return 0;
}
 

void IDLE2WORKING(int* barr, int index)
{
	barr[index] = BUSY;
}

void WORKING2IDLE(int* barr, int index)
{
	barr[index] = IDLE;
}
/////////////////////////

int main( argc, argv )
int argc;
char **argv;
{
	int rank, nProcs;
	MPI_Comm new_comm;

	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_split( MPI_COMM_WORLD, rank == 0, 0, &new_comm );
	if (rank == 0) //color. true for master. false for slave
		master_io(MPI_COMM_WORLD, new_comm);
	else
		slave_io(MPI_COMM_WORLD, new_comm);

	MPI_Finalize();
	return 0;
}

/* This is the master */
int master_io( master_comm, comm )
MPI_Comm comm;
{
	int i,j, nProcs; 
	MPI_Status status;
	int *busyArr;
	int idleCount =0;
	Qnode qn;
	SSnode n1,n2;
	int QueueSize =0;

	//for input data. shifted to header part.
	// ival ValArray;
	// iweight WtArray;
	// int asize;
	// int WtCap;

	MPI_Comm_size( master_comm, &nProcs ); //may use comm_world
	busyArr = malloc(sizeof(int)*nProcs); 

	busyArr[0] = BUSY; //1==busy
	for(i=1;i<nProcs ;i++)
	{
		busyArr[i] = IDLE;	//0==idle
	}
	idleCount = nProcs -1;

	//////////////////

//	void getData()
//	{
       
        char* tok;
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
       
       inpPack sackdata;
       sackdata.WtArray = WtArray;
       sackdata.ValArray = ValArray;
       sackdata.WtCap = WtCap;
//	} 		//getData part done
	
	
	MPI_Datatype INTARR; //Sarray
	MPI_Datatype SNODE; //_ssnode
	MPI_Datatype WRKRDATA; //wrkdata
	// MPI_Datatype WRKRRECV;  == SNODE
	MPI_Datatype INPDATA; //inpPack
	// MPI_Datatype SLVRDATA; //

	MPI_Type_contiguous(ASIZE, MPI_INT, &INTARR);
	MPI_Type_commit(&INTARR);
	
	int solnsize;
	MPI_Type_size(INTARR, &solnsize);
	int B[2] = {1,1};
	MPI_Aint D[2] = {0,solnsize};
	MPI_Datatype T[2] = {INTARR,MPI_INT};
	MPI_Type_create_struct(1,B,D,T, &SNODE);
	MPI_Type_commit(&SNODE);

	int _B[2] = {2,1};
	MPI_Aint _D[2] = {0,2*solnsize};
	MPI_Datatype _T[2] = {INTARR,MPI_INT};
	MPI_Type_create_struct(1,_B,_D,_T,&INPDATA);
	MPI_Type_commit(&INPDATA);

	int snodesize;
	MPI_Type_size(SNODE,&snodesize);
	int inpsize;
	MPI_Type_size(INPDATA,&inpsize);
	int __B[3] = {1,1,1};
	MPI_Aint __D[3] = {0,snodesize,snodesize+inpsize};
	MPI_Datatype __T[3] = {SNODE,MPI_FLOAT,INPDATA};
	MPI_Type_create_struct(1,__B,__D,__T, &WRKRDATA);
	MPI_Type_commit(&WRKRDATA);



	// MPI_Type_create_struct(1,{1,1},{0,4},{MPI_FLOAT,MPI_INT},&SLVRDATA);
	// MPI_Type_commit(&SLVRDATA);

	//send init subprob to first idle slave
	//auxSp = sp.initSubproblem();	
	//init subproblem
	
        Sarray initArray = calloc(asize,sizeof(subselect));
        for(i=0;i<asize;i++)
        {
            initArray[i] = SS_UNDEF;
        }
       	
       	qn = initQueue();
       	// qn->data = calloc(1,sizeof(_ssnode));
        qn->data.selectarray = initArray;
        qn->data.feas_flag = NOTFEASIBLE;
        QueueSize++;
      
    // slvrecvdata* srdata; 
    // slvrecvdata* ssdata;  
	wrkdata* initSprob;
	wrkdata* recvData;
	wrkrec* slvrecd;

	_ssnode* BestSolnVector;	//master copy
	float curBestVal = 0.0f;//master copy

	printf("starting master while.. QS:%d\n", QueueSize);
	while(QueueSize != 0)
    {
            //printf("entered while\n");
            int nn = QueueSize;
            int nt = (nn > nProcs-1)?(nProcs-1):nn;
          	printf("nn: %d, nthread: %d\n", nn,nt);
            //_ssnode dataArr[nt];
            for (j = 1; j < nt; j++)
            {       
                    // #pragma omp critical
                Qnode dnode = removefromQueue(qn);
                --QueueSize;

                //dataArr[j] = dnode->data; 
                wrkdata* temppack = malloc(sizeof(struct _wrkdata));
                temppack->soln.selectarray = dnode->data.selectarray;
                temppack->soln.feas_flag = dnode->data.feas_flag;
                temppack->bestVal = curBestVal;
                temppack->idata = sackdata;
                MPI_Send(&temppack,1,WRKRDATA, j, PBM_TAG, MPI_COMM_WORLD);
                idleCount--;
				IDLE2WORKING(busyArr, j); 		//mark this slave (#1) as working

            }

            int probeflag;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probeflag, &status);
            printf("starting  master probe.. val:%d QS : %d\n",probeflag, QueueSize);
			while(probeflag) /*flag == true if any msg cxan be received */
			{

				if(status.MPI_TAG == UPDATE_TAG)
				{
					MPI_Recv(recvData, 1, WRKRDATA, MPI_ANY_SOURCE,UPDATE_TAG,MPI_COMM_WORLD, &status);
					BestSolnVector->selectarray = recvData->soln.selectarray;
					BestSolnVector->feas_flag = recvData->soln.feas_flag;
					curBestVal = recvData->bestVal;
				}
				if(status.MPI_TAG == BRANCH_TAG)
				{
					MPI_Recv(slvrecd, 1, SNODE, MPI_ANY_SOURCE,BRANCH_TAG,MPI_COMM_WORLD, &status);
					// boundReturn ret = getBound(slvrecd)
					n1 = calloc(1,sizeof(_ssnode));
                    n2 = calloc(1,sizeof(_ssnode));
                   
                    Sarray n1sa = (Sarray)slvrecd->arr;
                    n1sa[slvrecd->val] = SS_INCLD;
                    n1->selectarray = n1sa;
                    n1->feas_flag = NOTFEASIBLE;
                   
                    Sarray n2sa = (Sarray)slvrecd->arr;
                    n2sa[slvrecd->val] = SS_EXCLD;
                    n2->selectarray = n2sa;
                    n2->feas_flag = NOTFEASIBLE;
                   
                    inserttoQueue(qn, n1); QueueSize++;
                    inserttoQueue(qn, n2); QueueSize++;
				}
				if(status.MPI_TAG == DONE_TAG)
				{
					break;
				}
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probeflag, &status);
			}

            // omp_set_num_threads(nt);
    }
    return 0;
 }          

/* This is the slave */
int slave_io( master_comm, comm )
MPI_Comm comm;
{
	int  rank;
	wrkdata* slvwrkdata;
	wrkdata* pbmsenddata;
	MPI_Status status;
	// slvrecvdata* srdata;
	// _ssnode auxSol;
	wrkrec* brdata;
	// boundReturn ret;
	


	MPI_Datatype INTARR; //Sarray
	MPI_Datatype SNODE; //_ssnode
	MPI_Datatype WRKRDATA; //wrkdata
	// MPI_Datatype WRKRRECV;  == SNODE
	MPI_Datatype INPDATA; //inpPack
	// MPI_Datatype SLVRDATA; //

	MPI_Type_contiguous(ASIZE, MPI_INT, &INTARR);
	MPI_Type_commit(&INTARR);
	
	int solnsize;
	MPI_Type_size(INTARR, &solnsize);
	int B[2] = {1,1};
	MPI_Aint D[2] = {0,solnsize};
	MPI_Datatype T[2] = {INTARR,MPI_INT};
	MPI_Type_create_struct(1,B,D,T, &SNODE);
	MPI_Type_commit(&SNODE);

	int _B[2] = {2,1};
	MPI_Aint _D[2] = {0,2*solnsize};
	MPI_Datatype _T[2] = {INTARR,MPI_INT};
	MPI_Type_create_struct(1,_B,_D,_T,&INPDATA);
	MPI_Type_commit(&INPDATA);

	int snodesize;
	MPI_Type_size(SNODE,&snodesize);
	int inpsize;
	MPI_Type_size(INPDATA,&inpsize);
	int __B[3] = {1,1,1};
	MPI_Aint __D[3] = {0,snodesize,snodesize+inpsize};
	MPI_Datatype __T[3] = {SNODE,MPI_FLOAT,INPDATA};
	MPI_Type_create_struct(1,__B,__D,__T, &WRKRDATA);
	MPI_Type_commit(&WRKRDATA);

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	int probeflag;
	while(1)
	{
		MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probeflag, &status);
		printf("starting while of slave no %d, probeflag %d\n",rank,probeflag);
		while (probeflag) 
		{
			if(status.MPI_TAG == PBM_TAG)
			{	
				MPI_Recv(slvwrkdata,1,WRKRDATA,0,PBM_TAG,MPI_COMM_WORLD,&status);

				boundReturn ret = getBound(&slvwrkdata->soln);
		        // printf("Entered the for loop for the %d time. Bound calculated: %f \n", j, ret.maxVal);
		        if(ret.flag == FEASIBLE)
		        {      
		            printf("Inside feasible check\n");
		            if(ret.maxVal > slvwrkdata->bestVal)
		            {
		        		// printf("Updating Current Best to %f!\n", curBest);
		                pbmsenddata = malloc(sizeof(struct _wrkdata));
		                pbmsenddata->soln = slvwrkdata->soln;
		                pbmsenddata->bestVal = ret.maxVal;
		                pbmsenddata->idata = slvwrkdata->idata;
		                // curBest = ret.maxVal;
		                MPI_Send(&pbmsenddata,1,WRKRDATA,0,UPDATE_TAG,MPI_COMM_WORLD);
		            }
		        }
		        else if(slvwrkdata->bestVal == 0)
		        {
		            //Branch on fractional index
		        	printf("Branching with NO feasible solution yet!\n");
		        	brdata = malloc(sizeof(struct _wrkrecv));
		        	brdata->val = ret.fractionalIndex;
		        	brdata->arr = (int*)slvwrkdata->soln.selectarray;
		        	MPI_Send(&brdata,1,SNODE,0,BRANCH_TAG,MPI_COMM_WORLD);
		        }
		        else if(slvwrkdata->bestVal != 0 && ret.maxVal > slvwrkdata->bestVal)
		        {
		            //Branch at fractional index
		            printf("Branching WITH a current feasible solution!\n");
		            brdata = malloc(sizeof(struct _wrkdata));
		        	brdata->val = ret.fractionalIndex;
		        	brdata->arr = (int*)slvwrkdata->soln.selectarray;
		        	MPI_Send(&brdata,1,SNODE,0,BRANCH_TAG,MPI_COMM_WORLD);
		        }
		        else //Discard
		        {
		            printf("Inside discard else block!\n");
		            //...here goes nothing..                              
		        }      
				//recv(source, flag);
				MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &probeflag, &status);
			} 
		}
		
	}
	MPI_Barrier(comm);
	return 0;
}