 ////////HEADER///////////
 // typedef enum
 // {
	//  SS_EXCLD = 0,
	//  SS_INCLD = 1,
	//  SS_UNDEF = 2
 // }subselect;

 typedef enum
 {
	 NOTFEASIBLE,
	 FEASIBLE,
	 OVERUSE
 }feasibleflag;
 
 typedef int* ival;              //shared
 typedef int* iweight;   //shared
 //typedef int* Sarray;      //non shared. each thread has one
 
 typedef struct _ssnode _ssnode;
 struct _ssnode
 {
	 int *selectarray;
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
	 float maxVal;
	 feasibleflag flag;
	 int *sack;
 };

 
 typedef struct _ssnodeTSP _ssnodeTSP;
 typedef struct _ssnodeTSP* SSnodeTSP;
 struct _ssnodeTSP
 {
 	int subP[5][5];
 	feasibleflag feas_flag;
 };
 typedef struct _QnodeTSP _QnodeTSP;
 typedef struct _QnodeTSP* QnodeTSP;


 struct _QnodeTSP
 {
 	_ssnodeTSP data;
 	QnodeTSP next;
 };

typedef struct boundReturnTSP boundReturnTSP;
 struct boundReturnTSP
 {
	 float maxVal;
	 feasibleflag flag;
 };
typedef struct intPair intPair;
struct intPair
{
	int num1;
	int num2;
};