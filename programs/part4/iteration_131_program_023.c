/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int use_omp = 1;

/* Function 1: Map array section with constant bounds */
__attribute__((noinline, used))
void func_map(void) {
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] += i;
        }
    }
}

/* Function 2: Map array section with variable bounds */
__attribute__((noinline, used))
void func_map_variable(int start, int length) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    #pragma omp target data map(local_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Use result to prevent elimination */
    global_arr[0] += local_arr[start];
}

/* Function 3: Task with depend clause using array section */
__attribute__((noinline, used))
void func_depend(void) {
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    #pragma omp task depend(inout: dynamic_arr[20:CHUNK]) shared(dynamic_arr)
    {
        for (int i = 20; i < 20 + CHUNK; i++) {
            dynamic_arr[i] += 1;
        }
    }
    
    #pragma omp task depend(inout: dynamic_arr[20:CHUNK]) shared(dynamic_arr)
    {
        for (int i = 20; i < 20 + CHUNK; i++) {
            dynamic_arr[i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    /* Store result */
    global_arr2[0] = dynamic_arr[20];
    free(dynamic_arr);
}

/* Function 4: Target update with array section */
__attribute__((noinline, used))
void func_update(int len) {
    int arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr3[i] = i * 4;
    }
    
    #pragma omp target update to(arr3[0:len])
    
    /* Simulate device computation */
    #pragma omp target map(tofrom: arr3[0:len])
    {
        for (int i = 0; i < len; i++) {
            arr3[i] += 5;
        }
    }
    
    #pragma omp target update from(arr3[0:len])
    
    global_arr2[1] = arr3[0];
}

/* Function 5: Complex array section expression */
__attribute__((noinline, used))
void func_complex_expr(int n) {
    int arr4[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr4[i] = i;
    }
    
    /* Array section with arithmetic in bounds */
    #pragma omp target data map(arr4[2*n: SIZE/4])
    {
        for (int i = 2*n; i < 2*n + SIZE/4; i++) {
            arr4[i] = arr4[i] * arr4[i - 1];  /* Create dependency */
        }
    }
    
    global_arr2[2] = arr4[2*n];
}

/* Function 6: Nested array reference as base */
__attribute__((noinline, used))
void func_nested_base(int idx) {
    int matrix[10][SIZE];
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Base is matrix[idx], which is itself an array reference */
    #pragma omp target data map(matrix[idx][10:50])
    {
        for (int j = 10; j < 60; j++) {
            matrix[idx][j] += matrix[idx][j-1];
        }
    }
    
    global_arr2[3] = matrix[idx][10];
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = 0;
    }
    
    /* Call functions based on volatile variable to prevent dead code elimination */
    if (use_omp) {
        func_map();
        func_map_variable(5, 30);
        func_depend();
        func_update(40);
        func_complex_expr(3);
        func_nested_base(2);
    }
    
    /* Compute checksum to verify execution */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
