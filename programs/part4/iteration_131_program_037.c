/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent optimization */
volatile int trigger = 1;

__attribute__((noinline, used))
void func_map(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Case 1: Simple array section with constants */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Case 2: Array section with variable bounds */
    int start = 20;
    int length = 80;
    #pragma omp target data map(local_arr[start:length])
    {
        for (int i = start; i < start + length; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Store result to global to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

__attribute__((noinline, used))
void func_depend(void) {
    int n = 5;
    int size = 30;
    
    /* Case 3: Array section in depend clause */
    #pragma omp task depend(inout: global_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr[i] = global_arr[i] * 3 + 1;
        }
    }
    
    /* Case 4: Multiple array sections in same directive */
    int m = 40;
    int len = 25;
    #pragma omp task depend(inout: global_arr[n:size]) \
                     depend(inout: global_arr2[m:len])
    {
        for (int i = m; i < m + len; i++) {
            global_arr2[i] = global_arr[i] - 5;
        }
    }
    
    #pragma omp taskwait
}

__attribute__((noinline, used))
void func_update(void) {
    int arr3[SIZE];
    int len = SIZE / 2;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        arr3[i] = i * 3;
    }
    
    /* Case 5: Array section in target update */
    #pragma omp target update to(arr3[0:len])
    
    /* Simulate some device computation */
    #pragma omp target teams distribute parallel for map(tofrom: arr3[0:len])
    for (int i = 0; i < len; i++) {
        arr3[i] = arr3[i] + 100;
    }
    
    #pragma omp target update from(arr3[0:len])
    
    /* Case 6: Complex expression in bounds */
    int offset = 10;
    #pragma omp target update from(arr3[offset:len - offset])
    
    /* Accumulate results */
    for (int i = 0; i < SIZE; i++) {
        global_arr2[i] += arr3[i];
    }
}

__attribute__((noinline, used))
void func_pointer_section(void) {
    int *dynamic_arr = (int *)malloc(SIZE * sizeof(int));
    if (!dynamic_arr) return;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 4;
    }
    
    /* Case 7: Array section on pointer/allocated memory */
    int p = 15;
    int section_size = 60;
    #pragma omp target data map(dynamic_arr[p:section_size])
    {
        for (int i = p; i < p + section_size; i++) {
            dynamic_arr[i] = dynamic_arr[i] / 2;
        }
    }
    
    /* Case 8: Nested array reference as base */
    int matrix[10][20];
    #pragma omp target data map(matrix[2][5:10])
    {
        for (int j = 5; j < 15; j++) {
            matrix[2][j] = j * 10;
        }
    }
    
    free(dynamic_arr);
}

__attribute__((noinline, used))
void func_mixed_sections(void) {
    /* Case 9: Multiple sections with different base types */
    struct {
        int data[SIZE];
        int count;
    } wrapper;
    
    #pragma omp target data map(wrapper.data[0:SIZE])
    {
        for (int i = 0; i < SIZE; i++) {
            wrapper.data[i] = i * 5;
        }
    }
    
    /* Case 10: Arithmetic in bounds */
    int k = 3;
    #pragma omp task depend(inout: global_arr[k*10:CHUNK+k])
    {
        for (int i = k*10; i < k*10 + CHUNK + k; i++) {
            global_arr[i] = global_arr[i] ^ 0xFF;
        }
    }
    
    #pragma omp taskwait
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Call functions based on volatile trigger to prevent dead code elimination */
    if (trigger & 1) func_map();
    if (trigger & 2) func_depend();
    if (trigger & 4) func_update();
    if (trigger & 8) func_pointer_section();
    if (trigger & 16) func_mixed_sections();
    
    /* Compute checksum to ensure code executes */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
