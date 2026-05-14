#ifndef SORT_H
#define SORT_H

#include <stddef.h>

typedef int (*CompareFunc)(const void* a, const void* b);

// mergeSort - Ordena um array usando Merge Sort
void mergeSort(void* array, size_t elementCount, size_t elementSize, CompareFunc compare);

#endif 
