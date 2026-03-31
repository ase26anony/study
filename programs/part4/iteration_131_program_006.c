/* omp_array_sections.c
 * Generates OpenMP array sections to exercise OMP_ARRAY_SECTION
 * pretty-printing in GCC's tree-pretty-print.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP directives */
int global_arr[SIZE];
int global_arr2[SIZE];
int global_arr3[SIZE];

/* Volatile variable to prevent optimization */
volatile int use_all = 1;

/* Function declarations with attributes to prevent inlining/elimination */
void __attribute__((noinline, used)) func_map_simple(void);
void __attribute__((noinline, used)) func_map_complex(void);
void __attribute__((noinline, used)) func_depend_variable(void);
void __attribute__((noinline, used)) func_update_expr(void);
void __attribute__((noinline, used)) func_nested_sections(void);
void __attribute__((noinline, used)) func_multiple_sections(void);

/* Helper to initialize arrays */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
        global_arr3[i] = i * 3;
    }
}

/* Function 1: Simple array section with constant bounds */
void __attribute__((noinline, used)) func_map_simple(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 10;
    }
    
    /* OMP_ARRAY_SECTION with constant bounds on global array */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Do some work */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += 1;
        }
    }
    
    /* OMP_ARRAY_SECTION with constant bounds on local array */
    #pragma omp target data map(local_arr[5:50])
    {
        for (int i = 5; i < 55; i++) {
            local_arr[i] += 2;
        }
    }
}

/* Function 2: Array section with variable bounds */
void __attribute__((noinline, used)) func_map_complex(void) {
    int start = 20;
    int length = 75;
    
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp target data map(global_arr2[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr2[i] *= 2;
        }
    }
}

/* Function 3: Array section in depend clause */
void __attribute__((noinline, used)) func_depend_variable(void) {
    int n = 30;
    int size = 40;
    
    /* OMP_ARRAY_SECTION in depend clause */
    #pragma omp task depend(inout: global_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr[i] -= 5;
        }
    }
    
    /* Ensure task completes */
    #pragma omp taskwait
}

/* Function 4: Array section with expression bounds */
void __attribute__((noinline, used)) func_update_expr(void) {
    int len = SIZE / 2;
    
    /* OMP_ARRAY_SECTION with expression bounds */
    #pragma omp target update from(global_arr3[0:len])
    
    /* Another with arithmetic expression */
    int offset = 10;
    #pragma omp target update to(global_arr3[offset*2:len/2])
}

/* Function 5: Nested array sections (array of pointers) */
void __attribute__((noinline, used)) func_nested_sections(void) {
    int *ptr_arr[10];
    
    /* Allocate and initialize pointer array */
    for (int i = 0; i < 10; i++) {
        ptr_arr[i] = (int*)malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) {
            ptr_arr[i][j] = i * 100 + j;
        }
    }
    
    /* OMP_ARRAY_SECTION on dereferenced pointer */
    #pragma omp target data map(ptr_arr[3][15:25])
    {
        for (int j = 15; j < 40; j++) {
            ptr_arr[3][j] += 100;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(ptr_arr[i]);
    }
}

/* Function 6: Multiple array sections in same directive */
void __attribute__((noinline, used)) func_multiple_sections(void) {
    /* Multiple OMP_ARRAY_SECTION nodes in same directive */
    #pragma omp target data \
        map(global_arr[0:50]) \
        map(global_arr2[25:75]) \
        map(global_arr3[50:100])
    {
        /* Combined operation */
        for (int i = 0; i < 50; i++) {
            global_arr[i] += global_arr2[i+25] + global_arr3[i+50];
        }
    }
}

/* Main function that calls all test functions */
int main(void) {
    init_arrays();
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_all) {
        func_map_simple();
        func_map_complex();
        func_depend_variable();
        func_update_expr();
        func_nested_sections();
        func_multiple_sections();
    }
    
    /* Compute checksum to ensure arrays are used */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + global_arr3[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
