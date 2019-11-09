struct qNode{
	struct movieMeta* data;
	struct qNode* next;
};
struct qNode* qFront = NULL;
struct qNode* qRear  = NULL;

typedef struct{
	
	int gloNumOfColumns;
	int gloIndexOfColumnToSort;
	int gloTypeOfSort;
	int queueAmt;

}GLOBAL;
GLOBAL gloVars={
	.gloNumOfColumns = -1,
	.gloIndexOfColumnToSort = -1,
	.gloTypeOfSort = -1,
	.queueAmt = 0, 
};
