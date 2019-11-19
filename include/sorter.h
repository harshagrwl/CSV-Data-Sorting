
typedef struct movieMeta {

	char** data;
	
}movie;


int indexOf(char* word, char* character);
int hasLetter(char*str);
int hasDecimal(char* str);
int hasNumbers(char* str);
int determineTypeOfSort(struct movieMeta *movie, int numOfRows, int numOfColumns, int indexOfColumnToSort);
int isEmpty(const char *word);
void Merge(struct movieMeta *arr,struct movieMeta *L,int leftCount,struct movieMeta *R,int rightCount, int indexOfColumnToSort, int typeOfSort);
void MergeSort(struct movieMeta *movie, int totalRows,   int numOfColumns, int indexOfColumnToSort, int typeOfSort);
char* trimWord(char* orgWord);
int getNumOfColumns(char*tempRow);
int getSortingColumnIndex(char* headerNames[], int numOfHeadColumns, char* columnToSort);
int isAtEndOfTable(int currentLine, int totalRows);
void confirmationPrint(struct movieMeta* movie, int numOfRows, int numOfColumns);
int main (int argc, char *argv[]);
