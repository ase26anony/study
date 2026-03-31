/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent dead code elimination */
volatile int use_omp = 1;

__attribute__((noinline, used))
void func_map(void) {
    /* Map array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
    
    /* Map array section with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(global_arr2[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr2[i] *= 2;
        }
    }
}

__attribute__((noinline, used))
void func_depend(int *arr, int n, int size) {
    /* Task with array section dependency */
    #pragma omp task depend(inout: arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            arr[i] = arr[i] * 3 + 1;
        }
    }
    
    /* Another task with different section */
    #pragma omp task depend(inout: arr[0:CHUNK])
    {
        for (int i = 0; i < CHUNK; i++) {
            arr[i] -= 5;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(int *arr, int len) {
    /* Target update with array section */
    #pragma omp target update from(arr[0:len])
    
    /* Update with arithmetic in bounds */
    #pragma omp target update to(arr[len/2:len/4])
}

__attribute__((noinline, used))
void func_complex_expr(int *arr) {
    /* Array section with more complex subscript expression */
    int base = 5;
    #pragma omp target data map(arr[base*2:base+10])
    {
        for (int i = base*2; i < base*2 + base + 10; i++) {
            arr[i] = i % 7;
        }
    }
}

__attribute__((noinline, used))
void func_multi_sections(int *a, int *b, int *c) {
    /* Multiple array sections in same directive */
    #pragma omp target data map(a[0:50], b[10:30], c[5:25])
    {
        for (int i = 0; i < 50; i++) a[i] += 1;
        for (int i = 10; i < 40; i++) b[i] += 2;
        for (int i = 5; i < 30; i++) c[i] += 3;
    }
}

__attribute__((noinline, used))
void func_array_ref_base(int (*matrix)[10]) {
    /* Base is an ARRAY_REF, testing op_prio() logic */
    #pragma omp target data map(matrix[5][2:8])
    {
        for (int j = 2; j < 10; j++) {
            matrix[5][j] = j * 10;
        }
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
    }
    
    int local_arr[SIZE];
    int local_arr2[SIZE];
    int local_arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 3;
        local_arr2[i] = i * 4;
        local_arr3[i] = i * 5;
    }
    
    int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute OpenMP functions based on volatile variable
       to ensure all code paths are compiled */
    if (use_omp) {
        #pragma omp parallel
        {
            #pragma omp single
            {
                func_map();
                
                #pragma omp task
                func_depend(local_arr, 30, 40);
                
                #pragma omp task
                func_update(local_arr2, 60);
                
                #pragma omp task
                func_complex_expr(local_arr3);
                
                #pragma omp task
                func_multi_sections(local_arr, local_arr2, local_arr3);
                
                #pragma omp task
                func_array_ref_base(matrix);
            }
        }
    }
    
    /* Compute checksum to prevent optimization and verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + 
                   local_arr[i] + local_arr2[i] + local_arr3[i];
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += matrix[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
