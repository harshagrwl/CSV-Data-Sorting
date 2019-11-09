#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Merge(struct movieMeta *arr,struct movieMeta *L,int leftCount,struct movieMeta *R,int rightCount, int indexOfColumnToSort, int typeOfSort){
	
	int leftSubIndex, rightSubIndex, mergedIndex;
	// leftSubIndex -  index of left aubarray (L)
	// rightSubIndex - index of right sub-raay (R)
	// mergedIndex -   index of merged subarray (arr)
	leftSubIndex = 0; rightSubIndex = 0; mergedIndex =0;
	
	/**
	 * strcmp ( s1, s2)
	 *
	 * x > 0 ; s1 > s2
	 * x < 0 ; s1 < s2
	 * x = 0 ; s1 = s2
	 * */
	while((leftSubIndex < leftCount) && (rightSubIndex < rightCount)){
	
		int comparison = -1;
		 /*
		 * For lower case and upper case versions of the same letter, the upper case should come first. 
		 * You should be careful to ensure that aardvark comes before Zebra though! Open up a dictionary, and that should show the order that words appear in.
		 * */
		switch(typeOfSort)
		{
			case 0:{
				 // String
				comparison = 0;
				
				char* leftStr = L[leftSubIndex].data[indexOfColumnToSort];
				char* rightStr = R[rightSubIndex].data[indexOfColumnToSort];
					
				if (strcasecmp(leftStr, rightStr) < 0)
					comparison = 1;
				break;
			}
			case 1:{ // long
				char *ptr1;
				char *ptr2;
				long leftValue = strtol(L[leftSubIndex].data[indexOfColumnToSort], &ptr1, 10);
				long rightValue = strtol(R[rightSubIndex].data[indexOfColumnToSort], &ptr2, 10);
				comparison = 0;
				if (leftValue < rightValue)
					comparison = 1;
				
				break;
			}
			case 2:{ // double
				comparison = 0;
				double leftValue = atof(L[leftSubIndex].data[indexOfColumnToSort]);
				double rightValue = atof(R[rightSubIndex].data[indexOfColumnToSort]);
				
				if (leftValue < rightValue)
					comparison = 1;
				break;
			}
		}
		//if(L[leftSubIndex].data[indexOfColumnToSort]  < R[rightSubIndex].data[indexOfColumnToSort]) // Original 
		if(comparison == 1)

		{
			arr[mergedIndex++] = L[leftSubIndex++];
		}
		else{
			arr[mergedIndex++] = R[rightSubIndex++];
		}
	}
	while(leftSubIndex < leftCount)
	{
		arr[mergedIndex++] = L[leftSubIndex++];
	}
	while(rightSubIndex < rightCount)
	{
		arr[mergedIndex++] = R[rightSubIndex++];
	}
}


void MergeSort(struct movieMeta *movie, int totalRows,   int numOfColumns, int indexOfColumnToSort, int typeOfSort){
	// base Case. If the array has less than two element, do nothing.
	if(totalRows < 2)
		return;
		
	int mid;
	mid = totalRows/2;  // find the mid index.

	// create left and right subarrays
	struct movieMeta *L = malloc(mid* sizeof(struct movieMeta) + sizeof(char*)*numOfColumns);
	struct movieMeta *R = malloc((totalRows - mid) * sizeof(struct movieMeta) + sizeof(char*)*numOfColumns);
	//initialize
	int i = 0;
	for(i = 0; i < mid; i++)
		L[i].data = malloc(sizeof(char*) * numOfColumns);
	for(i = 0; i < (totalRows-mid); i++)
		R[i].data = malloc(sizeof(char*) * numOfColumns);


	//Storing data in sub arrays
	for(i = 0; i<mid ;i++){
		int x = 0;
		for(x = 0; x < numOfColumns; x++)
			L[i].data[x] = movie[i].data[x];
	}

	for(i = mid ; i<totalRows ;i++){
		int x = 0;
		for (x = 0; x < numOfColumns; x++)
			R[i-mid].data[x] = movie[i].data[x];
	}

	MergeSort(L,    mid ,      numOfColumns, indexOfColumnToSort, typeOfSort);  // sorting the left subarray
	MergeSort(R,totalRows-mid, numOfColumns, indexOfColumnToSort, typeOfSort);  // sorting the right subarray

	Merge(movie,L,mid,R,totalRows-mid,       indexOfColumnToSort, typeOfSort);  // Merging L and R into movie as sorted list.
	

	free(L);
	free(R);
}
