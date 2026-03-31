/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays */
int global_arr[SIZE];
int global_arr2[SIZE];

/* Prevent dead code elimination */
volatile int use_func = 1;

/* Function 1: Map directive with array section */
__attribute__((noinline, used))
void func_map(int *arr, int start, int length) {
    #pragma omp target data map(arr[start:length])
    {
        /* Simple computation inside region */
        for (int i = start; i < start + length; i++) {
            arr[i] = arr[i] * 2;
        }
    }
}

/* Function 2: Depend clause with array section */
__attribute__((noinline, used))
void func_depend(int *arr, int n, int size) {
    #pragma omp task depend(inout: arr[n:size]) shared(arr)
    {
        for (int i = n; i < n + size; i++) {
            arr[i] = arr[i] + i;
        }
    }
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
__attribute__((noinline, used))
void func_update(int *arr, int lower, int len) {
    #pragma omp target update from(arr[lower:len])
    /* Simulate some host computation */
    for (int i = lower; i < lower + len; i++) {
        arr[i] = arr[i] - 1;
    }
}

/* Function 4: Complex array section with ARRAY_REF base */
__attribute__((noinline, used))
void func_complex(int *arr, int idx) {
    /* Create a multi-dimensional access pattern */
    int (*md_arr)[10] = (int(*)[10])arr;
    
    #pragma omp target data map(md_arr[idx][0:5])
    {
        for (int i = 0; i < 5; i++) {
            md_arr[idx][i] = idx * 10 + i;
        }
    }
}

/* Function 5: Multiple array sections in one directive */
__attribute__((noinline, used))
void func_multi_section(int *a, int *b, int start1, int len1, int start2, int len2) {
    #pragma omp target data map(a[start1:len1], b[start2:len2])
    {
        for (int i = 0; i < len1; i++) {
            a[start1 + i] = b[start2 + i] * 3;
        }
    }
}

/* Function 6: Array section with pointer base */
__attribute__((noinline, used))
void func_pointer_section(int *ptr, int offset, int count) {
    int *section_ptr = ptr + offset;
    
    #pragma omp target update to(section_ptr[0:count])
    {
        for (int i = 0; i < count; i++) {
            section_ptr[i] = section_ptr[i] * 2 + 1;
        }
    }
}

int main() {
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    /* Local array */
    int local_arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* Dynamic array */
    int *dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 3;
    }
    
    /* Call functions based on volatile variable to prevent optimization */
    if (use_func > 0) {
        /* Simple array section with constant bounds */
        func_map(global_arr, 10, 100);
        
        /* Array section with variable bounds */
        int n = 5;
        int section_size = 75;
        func_depend(global_arr2, n, section_size);
        
        /* Array section covering entire array */
        func_update(local_arr, 0, SIZE);
        
        /* Complex array section with computed bounds */
        func_complex(dynamic_arr, 3);
        
        /* Multiple sections with arithmetic in bounds */
        func_multi_section(global_arr, global_arr2, 
                          2*CHUNK, SIZE/4, 
                          CHUNK, SIZE/2);
        
        /* Pointer-based section */
        func_pointer_section(dynamic_arr, 25, 50);
    }
    
    /* Compute checksum to prevent elimination and verify execution */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i] + global_arr2[i] + local_arr[i];
        if (i < SIZE) checksum += dynamic_arr[i];  /* dynamic_arr is SIZE elements */
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(dynamic_arr);
    
    return 0;
}
