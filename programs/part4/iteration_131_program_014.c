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
        /* Simple computation inside region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Copy to global to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] += local_arr[i];
    }
}

__attribute__((noinline, used))
void func_depend(int n, int size) {
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr[i] += i;
        }
    }
    
    /* Nested array section with arithmetic expression */
    int offset = 2 * n;
    #pragma omp task depend(inout: global_arr2[offset:size/2])
    {
        for (int i = offset; i < offset + size/2; i++) {
            global_arr2[i] = i * 3;
        }
    }
}

__attribute__((noinline, used))
void func_update(int *ptr, int len) {
    /* OMP_ARRAY_SECTION on pointer with full range */
    #pragma omp target update from(ptr[0:len])
    {
        /* Dummy operation to keep the region */
        for (int i = 0; i < len; i++) {
            ptr[i] = ptr[i] % 100;
        }
    }
    
    /* Another section with different base */
    int *alias = ptr + 5;
    #pragma omp target update to(alias[10:len-15])
    {
        for (int i = 10; i < len - 5; i++) {
            alias[i] += 1;
        }
    }
}

__attribute__((noinline, used))
void func_mixed(void) {
    int matrix[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Array section on multi-dimensional array */
    #pragma omp target data map(matrix[10][20:30])
    {
        for (int j = 20; j < 50; j++) {
            matrix[10][j] = 0;
        }
    }
    
    /* Multiple array sections in same directive */
    #pragma omp target data \
        map(global_arr[0:CHUNK]) \
        map(global_arr2[CHUNK:CHUNK*2])
    {
        for (int i = 0; i < CHUNK; i++) {
            global_arr[i] += global_arr2[i + CHUNK];
        }
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i * 2;
        global_arr2[i] = i * 3;
    }
    
    /* Force all functions to be compiled and called */
    if (use_func) {
        func_map();
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                func_depend(5, 25);
                func_depend(30, 40);
            }
        }
        
        int *dynamic_arr = malloc(SIZE * sizeof(int));
        for (int i = 0; i < SIZE; i++) {
            dynamic_arr[i] = i * 4;
        }
        
        func_update(dynamic_arr, SIZE);
        func_mixed();
        
        free(dynamic_arr);
    }
    
    /* Compute checksum to prevent elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
