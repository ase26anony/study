/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int use_func = 1;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Another array section on global array with variable bounds */
    int start = 5;
    int length = CHUNK;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] = local_arr[i];
        }
    }
}

/* Function 2: Depend directive with array section */
__attribute__((noinline, used))
void func_depend(void) {
    int n = 20;
    int size = 75;
    
    /* OMP_ARRAY_SECTION with variable expressions */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Array section with arithmetic expression */
    #pragma omp task depend(inout: global_arr2[2*n:size/2])
    {
        for (int i = 2*n; i < 2*n + size/2; i++) {
            global_arr2[i] += 1;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(void) {
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    if (!dynamic_arr) return;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 4;
    }
    
    int len = SIZE / 2;
    /* OMP_ARRAY_SECTION on pointer/array */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Nested array section: base is itself an ARRAY_REF */
    int matrix[100][100];
    #pragma omp target update to(matrix[10][20:30])
    
    free(dynamic_arr);
}

/* Function 4: Combined construct with multiple array sections */
__attribute__((noinline, used))
void func_combined(void) {
    int arr1[SIZE], arr2[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
    }
    
    /* Multiple array sections in same directive */
    #pragma omp target data map(arr1[0:50], arr2[25:75])
    {
        for (int i = 0; i < 50; i++) arr1[i] *= 2;
        for (int i = 25; i < 100; i++) arr2[i] += 3;
    }
    
    /* Array section with complex base (subscript expression) */
    int *ptr_arr = arr1;
    #pragma omp target update from(ptr_arr[CHUNK*2:CHUNK])
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
    }
    
    /* Call functions conditionally to prevent dead code elimination */
    if (use_func & 1) func_map();
    if (use_func & 2) func_depend();
    if (use_func & 4) func_update();
    if (use_func & 8) func_combined();
    
    /* Compute checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
