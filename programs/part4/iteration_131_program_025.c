/* omp_array_section_test.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays to ensure they're visible to OpenMP directives */
int global_arr[SIZE];
int global_arr2[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure functions are compiled */
#define NOINLINE_USED __attribute__((noinline, used))

/* Function 1: Map directive with array section */
void NOINLINE_USED func_map(int start, int length) {
    /* Use array section in map clause */
    #pragma omp target data map(global_arr[start:length])
    {
        /* Simple computation inside region */
        for (int i = start; i < start + length; i++) {
            global_arr[i] *= 2;
        }
    }
}

/* Function 2: Depend directive with array section */
void NOINLINE_USED func_depend(int idx, int len) {
    /* Create task with array section dependency */
    #pragma omp task depend(inout: global_arr2[idx:len])
    {
        for (int i = idx; i < idx + len; i++) {
            global_arr2[i] += i;
        }
    }
    #pragma omp taskwait
}

/* Function 3: Update directive with array section */
void NOINLINE_USED func_update(int offset, int count) {
    /* Use array section in update clause */
    #pragma omp target update from(global_arr[offset:count])
    /* This might not do much without actual offload, but creates the node */
}

/* Function 4: Complex array section with ARRAY_REF base */
void NOINLINE_USED func_complex(int n) {
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* Array section on a subscripted expression */
    #pragma omp target data map(local_arr[n*2:CHUNK])
    {
        for (int i = n*2; i < n*2 + CHUNK; i++) {
            local_arr[i] = local_arr[i] * 3 + 1;
        }
    }
}

/* Function 5: Dynamic array with pointer arithmetic */
void NOINLINE_USED func_dynamic(int base_idx) {
    /* Array section on dynamically allocated memory */
    #pragma omp target data map(dynamic_arr[base_idx:SIZE/2])
    {
        for (int i = base_idx; i < base_idx + SIZE/2; i++) {
            dynamic_arr[i] = i % 100;
        }
    }
}

/* Function 6: Multiple array sections in same directive */
void NOINLINE_USED func_multiple(void) {
    /* Map multiple array sections */
    #pragma omp target data \
        map(global_arr[0:CHUNK]) \
        map(global_arr2[CHUNK:CHUNK*2]) \
        map(dynamic_arr[SIZE/4:SIZE/4])
    {
        /* Combined computation */
        for (int i = 0; i < CHUNK; i++) {
            global_arr[i] = global_arr2[i + CHUNK] + dynamic_arr[i + SIZE/4];
        }
    }
}

int main(void) {
    volatile int mode = 0; /* Prevent constant folding */
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_arr2[i] = SIZE - i;
    }
    
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenMP parallel region to ensure directives are processed */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Call different functions based on volatile variable
               This ensures all functions are compiled */
            if (mode == 0) {
                func_map(10, 100);
            } else if (mode == 1) {
                func_depend(5, 75);
            } else {
                func_update(0, 50);
            }
            
            /* Always call the other functions */
            func_complex(3);
            func_dynamic(25);
            func_multiple();
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += global_arr[i] + global_arr2[i];
        if (i < SIZE) sum += dynamic_arr[i]; /* Conditional to avoid simple opt */
    }
    
    printf("Checksum: %d\n", sum);
    
    free(dynamic_arr);
    return 0;
}
