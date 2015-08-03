typedef struct {
		int* data;
		int size;
		int maxsize;
} DynIntArr;

void arr_init(DynIntArr* arr, int s);
void resize(DynIntArr* arr, int newsize);
void append(DynIntArr* arr, int n);
void arr_free(DynIntArr* arr);
