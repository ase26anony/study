/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
__attribute__((noinline, used))
void func_map(void) {
    /* Use array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
    
    /* Another array section on same array with different bounds */
    #pragma omp target data map(global_arr[CHUNK*2:CHUNK])
    {
        for (int i = CHUNK*2; i < CHUNK*3; i++) {
            global_arr[i] *= 2;
        }
    }
}

__attribute__((noinline, used))
void func_depend(int n, int size) {
    /* Array section with variable bounds */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 3;
        }
    }
    
    /* Wait for task completion */
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(int len) {
    /* Array section on dynamic array */
    #pragma omp target update from(dynamic_arr[0:len])
    
    /* Another update with expression in bounds */
    int start = len/2;
    #pragma omp target update to(dynamic_arr[start:len-start])
}

__attribute__((noinline, used))
void func_complex_expr(void) {
    /* Array section with arithmetic expression in bounds */
    int offset = 5;
    #pragma omp target data map(global_arr[offset*2:CHUNK-offset])
    {
        for (int i = offset*2; i < offset*2 + (CHUNK-offset); i++) {
            global_arr[i] += global_arr2[i];
        }
    }
}

__attribute__((noinline, used))
void func_multi_sections(void) {
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[0:50]) \
        map(global_arr2[50:50]) \
        map(dynamic_arr[100:50])
    {
        for (int i = 0; i < 50; i++) {
            global_arr[i] = global_arr2[i+50] + dynamic_arr[i+100];
        }
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Call functions based on volatile variable to ensure all are compiled */
    if (mode > 0) {
        func_map();
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            if (mode > 0) {
                func_depend(20, 30);
            }
        }
    }
    
    if (mode > 0) {
        func_update(100);
    }
    
    if (mode > 0) {
        func_complex_expr();
    }
    
    if (mode > 0) {
        func_multi_sections();
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < 100) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(dynamic_arr);
    return 0;
}
