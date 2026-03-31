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

/* Function 1: Map with array section on global array */
__attribute__((noinline, used))
void func_map(void) {
    /* Array section with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
    
    /* Another section with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] *= 2;
        }
    }
}

/* Function 2: Task depend with array section on local array */
__attribute__((noinline, used))
void func_depend(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Array section in depend clause with expression */
    int n = 5;
    #pragma omp task depend(inout: local_arr[n:CHUNK])
    {
        for (int i = n; i < n + CHUNK; i++) {
            local_arr[i] += 100;
        }
    }
    
    /* Another depend with arithmetic expression */
    #pragma omp task depend(inout: local_arr[2*n:SIZE/4])
    {
        for (int i = 2*n; i < 2*n + SIZE/4; i++) {
            local_arr[i] *= 3;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Target update with array section on pointer array */
__attribute__((noinline, used))
void func_update(void) {
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    if (!dynamic_arr) return;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Array section in update directive */
    int len = SIZE - 10;
    #pragma omp target update to(dynamic_arr[0:len])
    
    /* Simulate some device computation */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < len; i++) {
        dynamic_arr[i] += 1;
    }
    
    /* Update back with different section */
    #pragma omp target update from(dynamic_arr[5:len-5])
    
    free(dynamic_arr);
}

/* Function 4: Complex array section on ARRAY_REF base */
__attribute__((noinline, used))
void func_complex_section(void) {
    int matrix[10][SIZE];
    
    /* Array section on multi-dimensional array subscript */
    #pragma omp target data map(matrix[2][10:80])
    {
        for (int i = 10; i < 90; i++) {
            matrix[2][i] = i * i;
        }
    }
    
    /* Nested array sections */
    int idx = 3;
    #pragma omp task depend(inout: matrix[idx][20:60])
    {
        for (int i = 20; i < 80; i++) {
            matrix[idx][i] += matrix[2][i];
        }
    }
    #pragma omp taskwait
}

/* Function 5: Mixed directives with array sections */
__attribute__((noinline, used))
void func_mixed(void) {
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[0:50]) \
        map(global_arr2[25:75])
    {
        for (int i = 0; i < 50; i++) {
            global_arr[i] = global_arr2[i+25];
        }
    }
    
    /* Array section with size expression */
    int chunk_size = CHUNK * 2;
    #pragma omp target update to(global_arr[SIZE/4:chunk_size])
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_func & 1) func_map();
    if (use_func & 2) func_depend();
    if (use_func & 4) func_update();
    if (use_func & 8) func_complex_section();
    if (use_func & 16) func_mixed();
    
    /* Compute checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
