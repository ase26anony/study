/* omp_array_section_test.c
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_test.c
 * Or: gcc -O2 -fopenmp -fdump-tree-gimple -c omp_array_section_test.c
 * Or: gcc -O2 -fopenmp -fdump-tree-omplower -c omp_array_section_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're accessible */
int global_arr[SIZE];
int global_arr2[SIZE];
int *global_ptr;

/* Prevent dead code elimination */
volatile int use_all = 1;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Simple operation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* OMP_ARRAY_SECTION with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(local_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            local_arr[i] += 3;
        }
    }
    
    /* Store result to global to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

/* Function 2: Depend directive with array section */
__attribute__((noinline, used))
void func_depend(void) {
    int n = 5;
    int size = 30;
    
    /* OMP_ARRAY_SECTION with expression in bounds */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] = i * 2;
        }
    }
    
    /* Another with arithmetic expression */
    int offset = 10;
    #pragma omp task depend(inout: global_arr2[2*offset:size/2])
    {
        for (int i = 2*offset; i < 2*offset + size/2; i++) {
            global_arr2[i] += 5;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(void) {
    int dynamic_arr[SIZE];
    int len = SIZE/2;
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* OMP_ARRAY_SECTION with full range */
    #pragma omp target update to(dynamic_arr[0:len])
    {
        /* Simulate some target computation */
        for (int i = 0; i < len; i++) {
            dynamic_arr[i] -= 10;
        }
    }
    
    /* OMP_ARRAY_SECTION with pointer base */
    int *ptr = &dynamic_arr[SIZE/4];
    #pragma omp target update from(ptr[0:len/2])
    
    /* Store to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += dynamic_arr[i];
    }
}

/* Function 4: Complex array section with ARRAY_REF base */
__attribute__((noinline, used))
void func_complex_base(void) {
    int matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 20 + j;
        }
    }
    
    /* OMP_ARRAY_SECTION on an array element (ARRAY_REF as base) */
    int row = 3;
    #pragma omp target data map(matrix[row][5:10])
    {
        for (int j = 5; j < 15; j++) {
            matrix[row][j] *= 2;
        }
    }
    
    /* Store to global */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            global_arr[(i*20 + j) % SIZE] += matrix[i][j];
        }
    }
}

/* Function 5: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int arr1[SIZE], arr2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
    }
    
    /* Multiple OMP_ARRAY_SECTION nodes in one directive */
    #pragma omp target data map(arr1[0:50], arr2[25:75])
    {
        for (int i = 0; i < 50; i++) arr1[i] += 1;
        for (int i = 25; i < 100; i++) arr2[i] -= 1;
    }
    
    /* Store to global */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += arr1[i] + arr2[i];
    }
}

int main(void) {
    /* Initialize globals */
    global_ptr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = 0;
        global_arr2[i] = 0;
        if (global_ptr) global_ptr[i] = i * 4;
    }
    
    /* Call all functions based on volatile variable 
     * to prevent constant folding and dead code elimination */
    if (use_all) {
        func_map();
        func_depend();
        func_update();
        func_complex_base();
        func_multiple_sections();
    }
    
    /* Compute checksum to ensure code executes */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (global_ptr) checksum += global_ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    if (global_ptr) free(global_ptr);
    return 0;
}
