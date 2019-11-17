#include <stdio.h>
#include <string.h>  
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/uio.h>
#include <assert.h>
#include "sorter.h"
#include "sorter_server.h"
#include "mergesort.c"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include <arpa/inet.h>



pthread_mutex_t lockServerConnection;
pthread_mutex_t lockOldMain;

void *connection_handler(void *);





//Trims whitespace around the word
char* trimWord(char* orgWord){
	//orgWord=original Word and newWord=trimmed word. Strings in C are array of chars
	if(orgWord==NULL){
		return NULL;
	}

	if(orgWord[0]=='\0'){
		return orgWord;
	}
	//checks if word is entirely spaces. "     "


	int isSpace = isEmpty(orgWord);

	if(isSpace==1){
		orgWord="";
	}
	else if (isSpace == 0){

		int i=0;
		int start; //index position of start of trimmed word
		int length=strlen(orgWord); //length of word
		int j=length; //j to start at end
		int stop=length; //index of where the last position of a letter is starting from the back
		start=i; //start to equal i just in case there is no beginninng space.

		while(orgWord[i]==' '){
			i++;//if its a space goes to next position
			start=i; //gets the new position of the string
		}//at the end of the loop, start will be at the index of the first letter position

		while(orgWord[j-1]==' '){
			j=j-1;
			stop=j;
		}

	   int lengthOfSubstr = stop - start; //length of string
	   orgWord=orgWord + start;//orgword starts at beginning;
	   char* newString=strdup(orgWord);
	   newString[lengthOfSubstr]='\0';
	   return newString;
   }

   return orgWord;//trying to do a substring method to get from start to stop.
}

//checks is string is empty
int isEmpty(const char *word) {
  
  while (*word != '\0') {
	if (!isspace((unsigned char)*word))
	  return 0;
	word++;
  }
  return 1;
}

//loops through and counts the number of columns detected in the csv file
int getNumOfColumns(char*tempRow){
	char* currentHeaderValue;
	int columnCounter = 0;
	currentHeaderValue = strtok(tempRow,",");

	while (currentHeaderValue!=NULL) {
		columnCounter++;
		currentHeaderValue = strtok(NULL, ",\n");
	}
	return columnCounter;
}

//custom indexOf Method to search if a character exists in the given string
int indexOf(char* word, char* character){
	char* temp = word;
	while (*temp)
	{
		if (strchr(character, *temp))
			return 1;
		temp++;
	}

	return 0;
}


//Returns 1 if string has letters; else 0
int hasLetters(char* str){
	//check if it has letters or symbols
	if (strlen(str) < 1)
		return 0;

	int hasLets = 0;
	int x = 0;
	for (x = 0; x < strlen(str); x++)
	{
		if((str[x] >= 'A' && str[x] <= 'Z') ||
		   (str[x] >= 'a' && str[x] <= 'z'))
		{
			hasLets = 1;
			break;
		}
	}

	return hasLets;
}

//Returns 1 if string has a . ; else 0
int hasDecimal(char* str){
	if (strlen(str) < 1)
		return 0;
	return indexOf(str, ".");
}

//Returns 1 if string has a number from 0~9
int hasNumbers(char* str){
	if (strlen(str) < 1)
		return 0;

	//check if has numbers and no letters
	int hasNums = 0;
	int x = 0;
	for (x = 0; x < strlen(str); x++)
	{
		if(str[x] >= '0' && str[x] <= '9')
		{
			hasNums = 1;
			break;
		}
	}

	return hasNums;
}

//Determines the type of sort based on the contents of the field
int determineTypeOfSort(struct movieMeta *movie, int numOfRows, int numOfColumns, int indexOfColumnToSort){
	int r= 0;
	int a = 0, b = 0, c = 0;
	//printf("a %d ; b %d; c %d; numOfRows %d; numOfColumns %d; indexToSort %d\n", a, b, c, numOfRows, numOfColumns, indexOfColumnToSort);
	for (r = 0; r < numOfRows; r++)
	{
		if (a == 0)
			a = hasLetters(movie[r].data[indexOfColumnToSort]);

		if (b == 0)
			b = hasDecimal(movie[r].data[indexOfColumnToSort]);

		if (c == 0)
			c = hasNumbers(movie[r].data[indexOfColumnToSort]);
	}

	//printf("a %d ; b %d; c %d\n", a, b, c);
	if (a == 1)
		return 0; // String no matter what it is
	else
	{
		if ((b == 0 && c == 0) || (b == 1 && c == 0))
			return 0; // String is null  || String is ......
		else if (b == 0 && c == 1)
			return 1; // Long
		else if (b == 1 && c == 1)
			return 2; // Double
		}

	return -1;
}



void largeEnqueue(struct movieMeta* movie){
	
	//Create a qNode to hold the array
	struct qNode* node = (struct qNode*) malloc (sizeof(struct qNode));
	
	node->data = movie;

	
	if ((qRear == NULL) && (qFront == NULL)){
		qRear = qFront = node;
		return;
	}

	node->next = NULL;
	qRear->next = node;
	qRear = node;
}

struct movieMeta* largeDequeue2(){
	struct qNode* node = qFront;
	if (qFront == NULL){
		printf("Queue Is Empty...\n");
		return NULL;
	}
	if (qFront == qRear){
		qFront = qRear = NULL;
	}
	else {
		qFront = qFront->next;
	}
	
	return node->data;
}



void printStruct(struct movieMeta* arr, int totalRows, int a){
	
	int index=0;
	for (index = 0; index< totalRows; index++) {
		printf("   [%s]     %d\n", arr[index].data[a], index);
	}
}

int getNumOfStructRows (struct movieMeta* movies) {
	int x = 0;
	while(movies[x].data != NULL)
		x++;
	return x;
}

//JOIN CSVS 
struct movieMeta* joinCSVs(){
	
	struct movieMeta* topHalf;
	struct movieMeta* lowHalf;
	int numRows = 0;
	
	while(gloVars.queueAmt >= 2)
	{
		//printf(">>> Loop starts\n");
		//printf("There are %d CSVs in the queue.... \n", gloVars.queueAmt);
		
		//CSV 1
		topHalf = largeDequeue2();
		//printf(" Seg 1?\n");
		//printf(" queue Amt: %d\n", gloVars.queueAmt);
		gloVars.queueAmt -= 1;
		//printf(" queue Amt: %d\n", gloVars.queueAmt);
		
		if (gloVars.gloTypeOfSort <= -1)
			gloVars.gloTypeOfSort = determineTypeOfSort(topHalf, getNumOfStructRows(topHalf)-1,28, gloVars.gloIndexOfColumnToSort);
		//printf(" Seg 2?\n");
		
		
		//int topHalfrow  = getNumOfStructRows(topHalf);
		//printf("A T1 = %d | T2 = %d ROWS\n", getNumOfStructRows(topHalf), 0);
		int topHalfrow = 0;
		while(topHalf[topHalfrow].data != NULL)
			topHalfrow++;
        //printf(" Seg 3?\n");

		
		//CSV 2
		lowHalf = largeDequeue2();
		gloVars.queueAmt -= 1;
		//int lowHalfrow = getNumOfStructRows(lowHalf);
		int lowHalfrow = 0;
		while(lowHalf[lowHalfrow].data != NULL)
			lowHalfrow++;
      //  printf(" Seg 4?\n");

		//printf("B T1 = %d | T2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf));
		
		numRows = topHalfrow + lowHalfrow;
		//printf("  numRows: %d\n", numRows);
		
		
		
		
		//Increase size of topHalf to incorporate lowHalf
		topHalf = realloc(topHalf, (numRows)*sizeof(struct movieMeta) * sizeof(char*)*(gloVars.gloNumOfColumns));
		//printf("C T1 = %d | T2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf));
		//printf(" Seg 5?\n");
		int r= 0; 
		for (r = topHalfrow; r < (numRows); r++) {
			topHalf[r].data = malloc(sizeof(char*) * (gloVars.gloNumOfColumns) ); //trying to figure out how to make room for string fields
		}
		//printf(" Seg 6?\n");

		//Store data
		int tIndex=0;
		int lIndex = 0; 
		for (tIndex = topHalfrow; tIndex < numRows; tIndex++) {
			topHalf[tIndex].data = lowHalf[lIndex].data;
			lIndex++;
		}
		 
      //  printf(" Seg 7?\n");
		//Call Merge sort on combined pieces
		MergeSort(topHalf, numRows, 28, gloVars.gloIndexOfColumnToSort, gloVars.gloTypeOfSort);
		//printf(" Seg 8?\n");
		
		//pthread_mutex_lock(&lockReNQ);	
			//Enqueue Joined struct
			largeEnqueue(topHalf);
			//printf("ENQUEUED.\n");
			gloVars.queueAmt+=1;
		//pthread_mutex_unlock(&lockReNQ);
		// printf(" Seg 9?\n");
		
		//printf("F TEMP 1 = %d | TEMP 2 = %d \n", getNumOfStructRows(topHalf), getNumOfStructRows(lowHalf)); 
		//printf(">>> Loop Ends\n\n");
		free(lowHalf);
	}
	//Mergesort once again for complete confidence
	//printf("  numRows: %d\n", numRows);
	
	
	//MergeSort(topHalf, numRows, gloVars.gloNumOfColumns, gloVars.gloIndexOfColumnToSort, gloVars.gloTypeOfSort);
	MergeSort(topHalf, numRows, 28, gloVars.gloIndexOfColumnToSort, gloVars.gloTypeOfSort);
	
	
	//printf("FINAL COMBINED UNSORTED\n");
	//printStruct(topHalf, numRows, gloVars.gloIndexOfColumnToSort);
	
	//printf("TOTAL ROWS AFTER JOINING: %d\n", getNumOfStructRows(topHalf));
	//printf("  JoinCSV: Queue Amt: %d\n", gloVars.queueAmt);
	//free(topHalf);
	return topHalf;
} 




void oldmain (char** csvArray, int numOfCsvRows){
	
	
	
	//Part 1: Use Header row to set up values
	//Grabs current row (first row at this point) and creates a temprow for safety
	char currentRow[2000];
	//fgets(currentRow, 1000 , fileInput); //gets current row with all the commas
	//currentRow  = strdup(csvArray[0]);
	//currentRow  = csvArray[0];
	strcpy(currentRow, csvArray[0]);
	char* tempRow = strdup(currentRow);
	int numOfHeadColumns = getNumOfColumns(tempRow); //Allows dynamic columns
	free(tempRow);

	tempRow = strdup(currentRow);
	tempRow[strlen(tempRow)-1]='\0';
	//populating headerNames to eventually find columnToSort
	char *headerNames[numOfHeadColumns];

	char* currentHeaderValue = strtok(tempRow,",");
	int i = 0;
	for (i = 0; i < numOfHeadColumns; i++)
	{
		headerNames[i] = malloc(strlen(currentHeaderValue)+1); // allocates mem for String array
		
		strcpy(headerNames[i],currentHeaderValue); //copies previous value into current position

		currentHeaderValue= strtok(NULL, ",\n"); // goes to next value
	}

	
	if (gloVars.gloNumOfColumns == -1){
		//pthread_mutex_lock(&lockNumCols);
		gloVars.gloNumOfColumns = numOfHeadColumns;
		//pthread_mutex_unlock(&lockNumCols);
	}
  
	free (tempRow);





	int stdnumOfRows = 10;
	char** dataRows = (char**) malloc(sizeof(char*)* stdnumOfRows);
	int x = 0;
	for (x = 0; x < stdnumOfRows; x++){
		dataRows[x] = malloc(sizeof(char)*(500+1));
	}
	
	
	int stdrowNumber = 0; //Counter to keep track of how many rows were made ; dynamic rows
	int indexCsvRow = 0; 
	for (indexCsvRow = 1; indexCsvRow < numOfCsvRows; indexCsvRow++){
		
		tempRow = strdup(csvArray[indexCsvRow]); //duplicate row for safety
		//printf("[%s]\n", tempRow);

		if(stdrowNumber == stdnumOfRows) // if the table is full...
		{
			dataRows = (char**) realloc(dataRows, 2* stdnumOfRows*sizeof(char*)); //realloc overall array with 2x the current size
			stdnumOfRows = stdnumOfRows*2; // update the size

			// and initialize the new rows with the same parameters as before
			int x = 0;
			for (x = stdrowNumber; x < stdnumOfRows; x++ )
				dataRows[x] = malloc(sizeof(char)*(500+1));
		}
		//copy data into array and increase index
		strcpy(dataRows[stdrowNumber],tempRow);
		stdrowNumber++;
		
		
		
	}
	
	int a = 0;
	for(a = 0; a < stdrowNumber; a++)
	{
		//printf("%s\n", dataRows[a]);
	}


	/* Part 3: Store data from ArrayOfStrings into ArrayOfStructs*/

	//Declare size
	int numOfRows = stdrowNumber; //dynamic, but static values work too
	//printf("  OM: Total numOfRows %d\n", numOfRows );
	int numOfColumns = numOfHeadColumns; // dynamic, but static values work too
	struct movieMeta* movie = malloc( numOfRows* sizeof(struct movieMeta) + sizeof(char*)*numOfColumns);
	int r=0;
	for (r = 0; r < numOfRows; r++){
		movie[r].data = malloc(sizeof(char*)*numOfColumns);
	}

	//==== Tokenizing with while loop


	r = 0;
	int currentLineIndex = 0;
	for (currentLineIndex = 0; currentLineIndex < numOfRows; currentLineIndex++) // For each line in dataRows
	{
		tempRow = strdup(dataRows[currentLineIndex]); // duplicate currentLine for safety
		tempRow[strlen(tempRow)-1]='\0';
		char* token;
		int c=0;
		while( (token = strsep(&tempRow,",")) != NULL ) //for each tokenized/seperation with comma....
		{
			movie[r].data[c] = malloc(strlen(token)+1); // malloc space for the incoming string
			//Check if the first character is a ", true? then fix it.

			if (token[0] == ('\"'))
			{
				token = strtok(token, "\""); //remove first quote

				char fix[500] = "";
				strcat(fix, token);
				//continuously append until the token contains a ".
				while(indexOf(token, "\"") != 1)
				{
					//Append the next string
					fix[strlen(fix)] = ','; // add comma to the end of token
					token = strsep(&tempRow, ","); // get next token
					strcat(fix, token); // append
				}

				token = strdup(fix); // Replace current with fixed
				token[strlen(token)-1] = '\0'; // remove remaining quotation mark
				token = trimWord(token); // trim whitespace
			}

			token = trimWord(token);
			movie[r].data[c] = token;
			c++;
		}
		r++;
		free(tempRow);
	}

		largeEnqueue(movie);
		gloVars.queueAmt++;
		
	
	
}



char * str_replace(char *orig, char *rep, char *with) {
    char *result;
    char *ins;
    char *tmp;
    int len_rep;
    int len_with;
    int len_front;
    int count;

    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL;
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}

//Helper method for connectionHandler
void trimBySize(char *str, size_t size){
    assert(size != 0 && str != 0);
    size_t len = strlen(str);
    if (size > len)
        return; 
        
        
    memmove(str, str+size, len - size + 1);
}


void *connection_handler(void *socket_desc){
	  pthread_mutex_lock(&lockServerConnection); 
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];

	
	int columnToSort = -1;
	int linesToRead = 0;
	char* confirmRecieveMsg = "Server: Line Recieved";
	read_size = recv(sock, client_message, 2000, 0);
	write(sock, confirmRecieveMsg, strlen(confirmRecieveMsg));
	//printf("Confirmation line sent\n");
	//printf("\n  [S]<@> Activated: [%s]\n", client_message);
	linesToRead = atoi(str_replace(client_message, "<@>", ""));
	//printf("LINES TO READ: %d\n", linesToRead);
	
	
	
	//char csvArray[linesToRead-1][2000]; 
	int csvArrIndex = 0; 
	
	char **csvArray;

	int numOfCsvRows = 0;
	int csvRowIndex = 0;
	
	//printf("Starting for loop O_O\n");
	int recIndex = 1; 
	for (recIndex = 1; recIndex <=linesToRead ; recIndex++){
		//printf("Loop %d start\n", recIndex);
			read_size = recv(sock , client_message , 2000 , 0);
			//send something back for proper syncronization
			//printf("Sending confirmation text for recIndex: %d\n", recIndex);
			
			write(sock,confirmRecieveMsg , strlen(confirmRecieveMsg));
			//printf("Confirmation line sent\n");
			client_message[read_size] = '\0';
			
			
			char* incomingStr = strdup(client_message);
			//clear the message buffer
			memset(client_message, 0, 2000);
			
			
			if (strstr(incomingStr, ">>>")){
				//- clear >>>
				//- store num in numOfCsvRows
				//- clear >@>
				//- replace @^@ with /n
				//- create char** array
				//- store row in array
				//- counter++
				//printf("\n  [SB%d]>>> Activated: [%s]\n", recIndex,incomingStr);
				
				incomingStr = str_replace(incomingStr, ">>>", "");

				char*dest = strstr(incomingStr, ">@>");
				int pos = dest - incomingStr;
                char valueStr[pos];
				int i = 0;
				for (i = 0; i < pos; i++){
					valueStr[i] = incomingStr[i];
				}
				numOfCsvRows = atoi(valueStr);
				//printf("VALUE IS: [%d]\n", numOfCsvRows);
                //incomingStr = str_replace(incomingStr, valueStr, "AAA");
                trimBySize(incomingStr, sizeof(valueStr));
               // printf("?? %s\n", incomingStr);
				incomingStr = str_replace(incomingStr,">@>", "");
				incomingStr = str_replace(incomingStr, "@^@", "\n");// Temporarily @^@=> "". it SHOULD BE @^@ => \n

				csvArray = (char**) malloc (numOfCsvRows*sizeof(char*))  ;
				for (i = 0; i < numOfCsvRows; i++){
					csvArray[i] = (char*)malloc((2000+1)* sizeof(char));
				}
							
				strcpy(csvArray[csvArrIndex], incomingStr);
				//printf("String Stored %d: [%s]\n",recIndex, csvArray[csvArrIndex]);
				csvArrIndex++;
			}
			else if (strstr(incomingStr, "<<<")){
				//printf("\n  [SC%d]<<< Activated: [%s]\n",recIndex, incomingStr);
				incomingStr = str_replace(incomingStr, "<<<", "");
                incomingStr = str_replace(incomingStr, "@^@", "\n");
                
				strcpy(csvArray[csvArrIndex], incomingStr);
				//printf("String Stored %d: [%s]\n",recIndex, csvArray[csvArrIndex]);
				csvArrIndex = 0;
				
				//Call Method to store in struct
				//reset and free csvArr
				
				pthread_mutex_lock(&lockOldMain);
				oldmain(csvArray, numOfCsvRows);
				free (csvArray);
				pthread_mutex_unlock(&lockOldMain);
			}
			else if (strstr(incomingStr, "@^@")){
				//printf("\n  [SD%d]@^@ Activated: [%s]\n", recIndex,incomingStr);
			    incomingStr = str_replace(incomingStr, "@^@", "\n");
				strcpy(csvArray[csvArrIndex], incomingStr);
				
				//printf("String Stored %d: [%s]\n",recIndex, csvArray[csvArrIndex]);
				csvArrIndex++;
				
			}
	
	}
	   
	    
	    
    // At this point client has sent ALL unsorted csvs,
    //               server stored ALL unsorted csvs in queue
    
    
    //Get sorting index FROM CLIENT
    
    
    int indexOfColumnToSort = -1;
    read_size = recv(sock, client_message, 2000, 0);
	write(sock, confirmRecieveMsg, strlen(confirmRecieveMsg));
	//printf("Confirmation line sent\n");
	indexOfColumnToSort = atoi(str_replace(client_message, ">@>", ""));
	
	gloVars.gloIndexOfColumnToSort = indexOfColumnToSort;
	//printf("We are sorting with column #: %d\n", indexOfColumnToSort);
	
    memset(client_message, 0, 2000);
	
	
	
	

    //printf("hello\n");
    struct movieMeta* largeStruct = joinCSVs();
    //printf("Struct joining complete\n Now sending lines back.\n");
    
    char output_row[2000];
    int i;
    //completeOutput(largeStruct);
	for(i = 0; i < (getNumOfStructRows(largeStruct)); i++){
        sprintf(output_row, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", 
        	largeStruct[i].data[0], largeStruct[i].data[1], largeStruct[i].data[2], largeStruct[i].data[3], 
        	largeStruct[i].data[4], largeStruct[i].data[5], largeStruct[i].data[6], largeStruct[i].data[7], 
        	largeStruct[i].data[8], largeStruct[i].data[9], largeStruct[i].data[10], largeStruct[i].data[11], 
        	largeStruct[i].data[12], largeStruct[i].data[13], largeStruct[i].data[14], largeStruct[i].data[15], 
        	largeStruct[i].data[16], largeStruct[i].data[17], largeStruct[i].data[18], largeStruct[i].data[19], 
        	largeStruct[i].data[20], largeStruct[i].data[21], largeStruct[i].data[22], largeStruct[i].data[23], 
        	largeStruct[i].data[24], largeStruct[i].data[25], largeStruct[i].data[26], largeStruct[i].data[27]);

        //printf("%s\n", output_row);
        write(sock, output_row, 2000);

        read_size = recv(sock, client_message, 2000,0);
        client_message[read_size] = '\0';
        //printf("%s\n", client_message);
        //printf("[%d] ", i);
        //clear recvd
        memset(client_message, 0, 2000);
    }
    //printf("Freeing struct\n");
    free(largeStruct);
    //printf("Emptying queue\n");

	while(gloVars.queueAmt > 0){	
		largeDequeue2();
		gloVars.queueAmt--;
	}
	
    //printf("Resetting globals %d\n", gloVars.queueAmt);
    
    gloVars.gloNumOfColumns = -1;
	gloVars.gloIndexOfColumnToSort = -1;
	gloVars.gloTypeOfSort = -1;
	gloVars.queueAmt = 0;
    
    
    
   
	//printf("Mergesorted Struct sent over successfully!\n");
	
	//close(read_size);
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    pthread_mutex_unlock(&lockServerConnection);      
   pthread_exit(NULL);
    
}


int main(int argc , char *argv[]){
	setbuf(stdout, NULL);
	printf("Received connections from: ", stdout);
	
	int paramIndex = 0, flagp = 0;
	
	char* portStr = "NULL"; //port in string form

	
	while (argv[paramIndex]!= NULL){
		if (strcmp(argv[paramIndex], "-p") == 0){
			portStr = argv[paramIndex+1];
			flagp = 1;
		}
		paramIndex++;
	}

	if (flagp ==0){
		printf("ERROR: -p <PORT> is missing. \n");
		exit(0);
	}
	
	int portValue = atoi(portStr);
	
	
	
	if(pthread_mutex_init(&lockServerConnection, NULL) != 0){
		printf("Error initializing lock1.\n");
	}

	
	
	
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    //puts("Socket created");
     
 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( portValue );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    //puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;

    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
		printf("%d.%d.%d.%d, ",
  			(int)(client.sin_addr.s_addr&0xFF),
  			(int)((client.sin_addr.s_addr&0xFF00)>>8),
  			(int)((client.sin_addr.s_addr&0xFF0000)>>16),
  			(int)((client.sin_addr.s_addr&0xFF000000)>>24));

        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         

    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    
    return 0;
}
 


