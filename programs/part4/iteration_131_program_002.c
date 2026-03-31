/* omp_array_section_coverage.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Array section with map clause on global array */
NOINLINE_USED
void func_map(void) {
    /* Map a section of global array */
    #pragma omp target data map(global_arr[10:100])
    {
        /* Simple operation inside mapped region */
        #pragma omp target teams distribute parallel for
        for (int i = 10; i < 110; i++) {
            global_arr[i] = i * 2;
        }
    }
}

/* Function 2: Array section with depend clause */
NOINLINE_USED
void func_depend(int n, int size) {
    /* Task with array section dependency */
    #pragma omp task depend(inout: global_arr2[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr2[i] += i;
        }
    }
    
    /* Ensure task is executed */
    #pragma omp taskwait
}

/* Function 3: Array section with update directive */
NOINLINE_USED
void func_update(int len) {
    /* Update from device to host */
    #pragma omp target update from(global_arr[0:len])
    
    /* Update to device from host with different bounds */
    #pragma omp target update to(global_arr2[len/2:len/2])
}

/* Function 4: Array section on pointer with complex expression */
NOINLINE_USED
void func_pointer_section(void) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Array section with pointer base and variable bounds */
    int *ptr = local_arr;
    int start = 5;
    int length = SIZE - 10;
    
    #pragma omp target data map(ptr[start:length])
    {
        #pragma omp target teams distribute parallel for
        for (int i = start; i < start + length; i++) {
            ptr[i] *= 3;
        }
    }
}

/* Function 5: Nested array section (array of pointers) */
NOINLINE_USED
void func_nested_section(void) {
    int *arr_of_ptrs[10];
    
    /* Allocate and initialize */
    for (int i = 0; i < 10; i++) {
        arr_of_ptrs[i] = (int*)malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) {
            arr_of_ptrs[i][j] = i * 100 + j;
        }
    }
    
    /* Array section on pointer element */
    #pragma omp target data map(arr_of_ptrs[2][20:30])
    {
        #pragma omp target teams distribute parallel for
        for (int j = 20; j < 50; j++) {
            arr_of_ptrs[2][j] += 1000;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(arr_of_ptrs[i]);
    }
}

/* Function 6: Multiple array sections in same directive */
NOINLINE_USED
void func_multiple_sections(void) {
    int a[SIZE], b[SIZE], c[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = 0;
    }
    
    /* Multiple array sections in map clause */
    #pragma omp target data map(a[0:50], b[25:75], c[10:90])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < SIZE; i++) {
            if (i < 50) c[i] = a[i] + b[i];
        }
    }
}

int main(void) {
    volatile int mode = 1; /* Prevent constant folding */
    
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = i * 3;
    }
    
    /* Allocate and initialize dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* Enable OpenMP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Call different functions based on mode (ensures all are compiled) */
            if (mode & 1) func_map();
            if (mode & 2) func_depend(30, 40);
            if (mode & 4) func_update(60);
            if (mode & 8) func_pointer_section();
            if (mode & 16) func_nested_section();
            if (mode & 32) func_multiple_sections();
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
        if (i < SIZE && dynamic_arr) checksum += dynamic_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(dynamic_arr);
    
    return 0;
}
