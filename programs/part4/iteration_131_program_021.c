/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization and ensure functions are present */
volatile int selector = 1;

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
        local_arr[i] = i;
    }
    
    #pragma omp target data map(local_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Use result to prevent elimination */
    global_arr2[0] += local_arr[start];
}

/* Function 3: Task with depend clause using array section */
__attribute__((noinline, used))
void func_depend(void) {
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    
    if (!dynamic_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    #pragma omp task depend(inout: dynamic_arr[20:30])
    {
        for (int i = 20; i < 50; i++) {
            dynamic_arr[i] += 100;
        }
    }
    
    #pragma omp task depend(inout: dynamic_arr[20:30])
    {
        for (int i = 20; i < 50; i++) {
            dynamic_arr[i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    /* Store result */
    global_arr[0] = dynamic_arr[25];
    
    free(dynamic_arr);
}

/* Function 4: Target update with array section */
__attribute__((noinline, used))
void func_update(void) {
    int update_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        update_arr[i] = SIZE - i;
    }
    
    #pragma omp target update to(update_arr[0:CHUNK])
    
    /* Simulate some device computation */
    #pragma omp target teams distribute parallel for map(tofrom: update_arr[0:CHUNK])
    for (int i = 0; i < CHUNK; i++) {
        update_arr[i] = update_arr[i] * 3 + 1;
    }
    
    #pragma omp target update from(update_arr[CHUNK:SIZE-CHUNK])
    
    /* Use result */
    global_arr2[1] = update_arr[CHUNK/2];
}

/* Function 5: Complex array section with ARRAY_REF as base */
__attribute__((noinline, used))
void func_complex_base(void) {
    int matrix[10][20];
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * 20 + j;
        }
    }
    
    /* Array section on a subscripted array element */
    #pragma omp target data map(matrix[5][2:15])
    {
        for (int j = 2; j < 17; j++) {
            matrix[5][j] += 1000;
        }
    }
    
    global_arr[1] = matrix[5][10];
}

/* Function 6: Multiple array sections in same directive */
__attribute__((noinline, used))
void func_multiple_sections(void) {
    int a[SIZE], b[SIZE], c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    #pragma omp target data map(a[0:50], b[25:75], c[SIZE/4:SIZE/2])
    {
        for (int i = 0; i < 50; i++) a[i]++;
        for (int i = 25; i < 100; i++) b[i]++;
        for (int i = SIZE/4; i < 3*SIZE/4; i++) c[i]++;
    }
    
    global_arr2[2] = a[10] + b[50] + c[SIZE/2];
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i * 3;
        global_arr2[i] = i * 5;
    }
    
    /* Call functions based on volatile selector to ensure all are compiled */
    if (selector > 0) {
        func_map();
        func_map_variable(5, 75);
    }
    
    if (selector > -1) {
        func_depend();
        func_update();
    }
    
    if (selector < 100) {
        func_complex_base();
        func_multiple_sections();
    }
    
    /* Compute checksum to verify execution */
    long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
