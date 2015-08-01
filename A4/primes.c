#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dynarr.h"


int is_happy(int num) {
	DynIntArr* arr = malloc(sizeof(DynIntArr));
	arr_init(arr, 10);
	
	while(num != 1) {						// Loop until found happy or sad
		int new_num = 0;
		while (num / 10 != 0) {				// While >1 digit
			new_num += pow(num % 10, 2);	// num % 10 = least sig. digit
			num = num / 10;					// Chop off last digit
		}
		new_num += pow(num, 2);
		num = new_num;
		
		int i;								// Check if already hit this #
		for (i=0; i<arr->size; i++)
		{
			if (num == arr->data[i])
			{
				printf("is sad!\n");
				return 0;
			}
		}
		append(arr, num);
	}
	printf("is happy!\n");
	arr_free(arr);
	free(arr);
	
	return 1;
}

int main() {
	is_happy(998);
	return 0;
}














