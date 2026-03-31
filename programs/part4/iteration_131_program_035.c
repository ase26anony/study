/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent dead code elimination */
volatile int use_global = 1;

/* Non-inlineable functions with different array section patterns */
__attribute__((noinline, used))
void func_map_simple(void) {
    /* Simple array section with constants */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Dummy operation to prevent empty region optimization */
        global_arr[10] = 1;
    }
}

__attribute__((noinline, used))
void func_map_complex(void) {
    int local_arr[SIZE];
    int start = 5;
    int length = CHUNK * 2;
    
    /* Array section with variable expressions */
    #pragma omp target data map(local_arr[start:length])
    {
        local_arr[start] = start;
    }
    
    /* Copy back to global to prevent elimination */
    if (use_global) {
        global_arr[0] = local_arr[start];
    }
}

__attribute__((noinline, used))
void func_depend(void) {
    int n = 20;
    int size = 30;
    
    /* Task with depend clause using array section */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 2;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(void) {
    static int static_arr[SIZE];
    int len = SIZE / 2;
    
    /* Initialize static array */
    for (int i = 0; i < SIZE; i++) {
        static_arr[i] = i * 3;
    }
    
    /* Target update with array section */
    #pragma omp target update from(static_arr[0:len])
    
    /* Use result to prevent elimination */
    if (use_global) {
        global_arr[1] = static_arr[len-1];
    }
}

__attribute__((noinline, used))
void func_multi_sections(void) {
    int multi_arr[SIZE];
    int *ptr_arr = multi_arr;
    int offset = 15;
    
    /* Multiple array sections in different contexts */
    #pragma omp target data map(multi_arr[0:SIZE/4]) \
                            map(ptr_arr[offset:CHUNK])
    {
        multi_arr[0] = 100;
        ptr_arr[offset] = 200;
    }
    
    /* Array section with arithmetic expression */
    int expr_start = 2 * CHUNK;
    int expr_len = CHUNK + 10;
    #pragma omp target update to(multi_arr[expr_start:expr_len])
}

__attribute__((noinline, used))
void func_nested_subscript(void) {
    int matrix[10][SIZE];
    int row = 2;
    
    /* Array section on subscripted base (ARRAY_REF) */
    #pragma omp target data map(matrix[row][10:50])
    {
        matrix[row][10] = 999;
    }
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions with different array section patterns */
    /* Use volatile to prevent constant folding */
    volatile int mode = 0;
    
    if (mode == 0) {
        func_map_simple();
    }
    
    func_map_complex();
    func_depend();
    func_update();
    func_multi_sections();
    func_nested_subscript();
    
    /* Compute checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
