#include <stdlib.h>

#include "dynarr.h"

void arr_init(DynIntArr* arr, int s) {
	arr->size = 0;
	arr->maxsize = s;
	arr->data = malloc(sizeof(int) * arr->maxsize);
}

void resize(DynIntArr* arr, int newsize) {
	int* new_data = malloc(sizeof(int) * newsize);
	int i;
	for (i = 0; i<arr->size; i++) 
	{
		new_data[i] = arr->data[i];
	}
	free(arr->data);
	arr->data = new_data;
}

void append(DynIntArr* arr, int n) {
	arr->size++;
	if (arr->size < arr->maxsize) {
		arr->data[arr->size-1] = n;
	}
	else {
		resize(arr, arr->maxsize*2);
		arr->data[arr->size-1] = n;
	}
}

void arr_free(DynIntArr* arr) {
	free(arr->data);
}