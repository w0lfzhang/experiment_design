#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct 
{
	int *p;
	int start;
	int end;
}arr;

void DataSwap(int* data1, int* data2)  
{  
    int temp = *data1;  
    *data1 = *data2;  
    *data2 = temp;  
}  

void SelectionSort(int *pDataArray, int start, int end)  
{  
	int i, index, j;
    for (i = start; i < end ; i++)      
    {  
        index = i;  
        for (j = i + 1; j <= end; j++)       
            if (pDataArray[j] < pDataArray[index])  
                index = j;  
  
        if (index != i)      
            DataSwap(&pDataArray[index], &pDataArray[i]);  
    }  
}  

void merge(int *arr,int *temp,int start,int mid,int end){
	int i = start, j = mid + 1,k = start;
	while(i != mid + 1 && j != end +1 )
	{
		if(arr[i] > arr[j])
			temp[k++] = arr[j++];
		else
			temp[k++] = arr[i++];
	}
	while(i != mid + 1)
	{
		temp[k++] = arr[i++];
	}
	while(j != end + 1)
	{
		temp[k++] = arr[j++];
	}
	for(i = start; i <= end; i++)
	{
		arr[i] = temp[i];
	}
}
void *selectfunc(void *arg)
{
	arr *array = (arr *)arg;
	SelectionSort(array->p, array->start, array->end);
	return NULL;
}

int main()
{
	pthread_t tid;
	int array[10] = {10, 45, 4, 32, 54, 35, 24, 78, 23, 1};
	int temp[10];
	int n = 10;
	arr test = {array, 0, n/2 - 1};
	pthread_create(&tid, NULL, selectfunc, &test);
	SelectionSort(array, n/2, n - 1);
	pthread_join(tid, NULL);

	merge(array, temp, 0, n/2 - 1, n - 1);
	int i;
	printf("after sorting:\n");
	for(i = 0; i < 10; i++)
	{
		printf("%d  ", array[i]);
	}
	getchar();
}
