#include "sort.h"
#include <stdlib.h>
#include <string.h>

// merge - Combina dois subarrays ordenados
static void merge(void* array, size_t left, size_t mid, size_t right, size_t elementSize, CompareFunc compare) {
    void* leftTemp = malloc((mid - left) * elementSize);
    void* rightTemp = malloc((right - mid) * elementSize);
    
    if (leftTemp == NULL || rightTemp == NULL) {
        free(leftTemp);
        free(rightTemp);
        return;
    }
    
    memcpy(leftTemp, (char*)array + left * elementSize, (mid - left) * elementSize);
    memcpy(rightTemp, (char*)array + mid * elementSize, (right - mid) * elementSize);
    
    size_t i = 0; 
    size_t j = 0;      
    size_t k = left;   
    
    while (i < (mid - left) && j < (right - mid)) {
        void* leftElem = (char*)leftTemp + i * elementSize;
        void* rightElem = (char*)rightTemp + j * elementSize;
        
        if (compare(leftElem, rightElem) >= 0) {
            memcpy((char*)array + k * elementSize, leftElem, elementSize);
            i++;
        } else {
            memcpy((char*)array + k * elementSize, rightElem, elementSize);
            j++;
        }
        k++;
    }
    
    while (i < (mid - left)) {
        memcpy((char*)array + k * elementSize, (char*)leftTemp + i * elementSize, elementSize);
        i++;
        k++;
    }
    
    while (j < (right - mid)) {
        memcpy((char*)array + k * elementSize, (char*)rightTemp + j * elementSize, elementSize);
        j++;
        k++;
    }
    
    free(leftTemp);
    free(rightTemp);
}


// mergeSortInternal - Implementação recursiva de Merge Sort
static void mergeSortInternal(void* array, size_t left, size_t right, size_t elementSize, CompareFunc compare) {
    if (left + 1 >= right) {
        return;
    }
    
    size_t mid = left + (right - left) / 2;

    mergeSortInternal(array, left, mid, elementSize, compare);
    mergeSortInternal(array, mid, right, elementSize, compare);
    merge(array, left, mid, right, elementSize, compare);
}

// mergeSort - Ordena um array usando Merge Sort
void mergeSort(void* array, size_t elementCount, size_t elementSize, CompareFunc compare) {
    if (array == NULL || elementCount <= 1 || elementSize == 0 || compare == NULL) {
        return;
    }
    
    mergeSortInternal(array, 0, elementCount, elementSize, compare);
}
