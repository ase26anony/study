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

/* Function 1: Array section with map clause on global array */
__attribute__((noinline, used))
void func_map(void) {
    if (!use_omp) return;
    
    /* Map a section of global array with constant bounds */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i * 2;
        }
    }
    
    /* Another map with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(global_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            global_arr[i] += 1;
        }
    }
}

/* Function 2: Array section with depend clause */
__attribute__((noinline, used))
void func_depend(void) {
    if (!use_omp) return;
    
    int local_arr[SIZE];
    int n = 5;
    int size = 30;
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Task with array section dependency */
    #pragma omp task depend(inout: local_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Another task with different section */
    #pragma omp task depend(inout: local_arr[2*n:size/2])
    {
        for (int i = 2*n; i < 2*n + size/2; i++) {
            local_arr[i] += 3;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Array section with update directive */
__attribute__((noinline, used))
void func_update(void) {
    if (!use_omp) return;
    
    static int static_arr[SIZE];
    int len = 40;
    
    /* Initialize static array */
    for (int i = 0; i < SIZE; i++) {
        static_arr[i] = i * 3;
    }
    
    /* Update to device */
    #pragma omp target update to(static_arr[0:len])
    
    /* Simulate device computation */
    #pragma omp target teams distribute parallel for map(tofrom: static_arr[0:len])
    for (int i = 0; i < len; i++) {
        static_arr[i] += 10;
    }
    
    /* Update from device with arithmetic expression in bounds */
    #pragma omp target update from(static_arr[len/2:len - len/2])
}

/* Function 4: Nested array section (ARRAY_REF as base) */
__attribute__((noinline, used))
void func_nested(void) {
    if (!use_omp) return;
    
    int matrix[10][20];
    int row = 2;
    
    /* Array section on a subscripted array element */
    #pragma omp target data map(matrix[row][5:10])
    {
        for (int i = 5; i < 15; i++) {
            matrix[row][i] = row * 100 + i;
        }
    }
}

/* Function 5: Pointer-based array section */
__attribute__((noinline, used))
void func_pointer(void) {
    if (!use_omp) return;
    
    int *dynamic_arr = malloc(SIZE * sizeof(int));
    if (!dynamic_arr) return;
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 4;
    }
    
    /* Array section on pointer */
    int offset = 15;
    int section_size = 25;
    #pragma omp target data map(dynamic_arr[offset:section_size])
    {
        for (int i = offset; i < offset + section_size; i++) {
            dynamic_arr[i] -= 5;
        }
    }
    
    free(dynamic_arr);
}

/* Function 6: Complex bounds expressions */
__attribute__((noinline, used))
void func_complex_bounds(void) {
    if (!use_omp) return;
    
    int complex_arr[SIZE];
    int x = 8;
    int y = 12;
    
    /* Array section with arithmetic in bounds */
    #pragma omp target data map(complex_arr[x + y: x * y])
    {
        for (int i = x + y; i < x + y + x * y && i < SIZE; i++) {
            complex_arr[i] = i % 100;
        }
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 2;
    }
    
    /* Call all functions to ensure they're compiled */
    if (use_omp) {
        func_map();
        func_depend();
        func_update();
        func_nested();
        func_pointer();
        func_complex_bounds();
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
