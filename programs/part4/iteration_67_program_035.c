/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compilation options for coverage:
 *   1. -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower
 *   2. -O0 -fopenmp -Wall -Werror=openmp-mapping -c
 *   3. -O2 -fopenmp -foffload=disable -fdump-tree-optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int offset) {
    volatile int v = base; /* volatile prevents optimization */
    return v + offset;
}

/* Function that uses array sections in a map clause */
void process_section(float* arr, int start, int length, int n) {
    /* Complex base expression 1: conditional operator (low precedence) */
    float* alt_arr = arr + n/2;
    #pragma omp target data map(to: (n > 10 ? arr : alt_arr)[start:length])
    {
        /* Inside target data region - launch a kernel */
        #pragma omp target map(alloc: (arr + start)[0:length])
        {
            for (int i = 0; i < length; i++) {
                (arr + start)[i] += 1.0f;
            }
        }
    }
}

/* Another function with different array section usage */
void task_with_depend(float* ptr1, float* ptr2, int offset, int count) {
    /* Complex base expression 2: pointer arithmetic */
    #pragma omp task depend(inout: (ptr1 + offset)[0:count]) \
                     depend(in: (ptr2)[offset:count])
    {
        for (int i = 0; i < count; i++) {
            (ptr1 + offset)[i] = (ptr2)[offset + i] * 2.0f;
        }
    }
}

/* Function that might provoke type checking diagnostics */
void problematic_section(int* int_arr, float* float_arr, int len) {
    /* This may trigger warnings about type mismatches */
    #pragma omp target data map(to: int_arr[0:len]) map(from: float_arr[0:len])
    {
        /* Mixed types in array sections - potential diagnostic source */
        #pragma omp target
        for (int i = 0; i < len; i++) {
            float_arr[i] = (float)int_arr[i];
        }
    }
}

int main(int argc, char* argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int base_size = argc > 1 ? atoi(argv[1]) : 100;
    int section_start = argc > 2 ? atoi(argv[2]) : 10;
    int section_length = argc > 3 ? atoi(argv[3]) : 20;
    
    /* Dynamic allocation prevents compile-time knowledge of bounds */
    float* array1 = (float*)malloc(base_size * sizeof(float));
    float* array2 = (float*)malloc(base_size * sizeof(float));
    int* int_array = (int*)malloc(base_size * sizeof(int));
    
    if (!array1 || !array2 || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < base_size; i++) {
        array1[i] = (float)i;
        array2[i] = (float)(i * 2);
        int_array[i] = i * 3;
    }
    
    /* Strategy 1: Complex base with conditional operator */
    printf("Processing with conditional base expression...\n");
    process_section(array1, 
                   get_bound(section_start, 0),  /* non-constant bound */
                   get_bound(section_length, 1), /* non-constant bound */
                   base_size);
    
    /* Strategy 2: Complex base with pointer arithmetic in task depend */
    printf("Creating tasks with pointer arithmetic bases...\n");
    #pragma omp parallel
    #pragma omp single
    {
        task_with_depend(array1, array2, 
                        get_bound(5, section_start),  /* non-constant */
                        get_bound(8, section_length)); /* non-constant */
        
        /* Wait for task completion */
        #pragma omp taskwait
    }
    
    /* Strategy 3: Potential type checking diagnostics */
    printf("Processing with mixed types...\n");
    problematic_section(int_array, array2, 
                       get_bound(15, section_length)); /* non-constant */
    
    /* Strategy 4: Structure member access as base expression */
    struct Data {
        float* values;
        int count;
    } data;
    
    data.values = array1;
    data.count = base_size;
    
    /* Complex base: structure member access */
    #pragma omp target data map(tofrom: data.values[section_start:section_length])
    {
        #pragma omp target
        for (int i = 0; i < section_length; i++) {
            data.values[section_start + i] *= 2.0f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < base_size; i++) {
        checksum += array1[i] + array2[i] + int_array[i];
    }
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(int_array);
    
    return 0;
}
