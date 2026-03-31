/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-omp-array-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-all
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Helper function to introduce variability and prevent constant folding */
static int get_value(volatile int *ptr, int default_val) {
    return ptr ? *ptr : default_val;
}

/* Function that uses array sections in map clauses with complex base expressions */
void process_array_sections(int *arr1, int *arr2, int start, int length, int offset, int cond) {
    volatile int vol_start = start;
    volatile int vol_len = length;
    
    /* Use command-line derived values to prevent constant propagation */
    int lower = get_value(&vol_start, start);
    int upper = get_value(&vol_len, length);
    
    /* TARGET DATA with array section using conditional operator as base */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[lower:upper])
    {
        /* Complex base: pointer arithmetic with offset */
        #pragma omp target map(tofrom: (arr1 + offset)[0:upper - lower])
        for (int i = 0; i < upper - lower; i++) {
            (arr1 + offset)[i] = (cond ? arr1 : arr2)[lower + i] + 1;
        }
        
        /* Another array section with structure-like access simulation */
        struct { int *data; } wrapper = {arr2};
        #pragma omp target map(from: wrapper.data[lower:upper])
        for (int i = 0; i < upper - lower; i++) {
            wrapper.data[lower + i] = i * 2;
        }
    }
    
    /* TASK with depend clause using array section - different context */
    #pragma omp task depend(inout: arr1[start:length]) shared(arr1, start, length)
    {
        for (int i = 0; i < length; i++) {
            arr1[start + i] *= 2;
        }
    }
    
    /* ENTER/EXIT DATA with array sections */
    #pragma omp target enter data map(to: arr2[offset:length])
    
    #pragma omp target map(alloc: arr2[offset:length])
    {
        for (int i = 0; i < length; i++) {
            arr2[offset + i] += 3;
        }
    }
    
    #pragma omp target exit data map(from: arr2[offset:length])
}

/* Function that deliberately creates type warning opportunities */
void problematic_array_section_usage(int *ptr, int n) {
    /* This may trigger diagnostics about array section in non-OpenMP context */
    int (*section_ptr)[n] = (int (*)[n])&ptr[0:n];  /* Cast that might warn */
    
    /* OpenMP usage with complex bounds */
    volatile int vol_n = n;
    int dynamic_len = get_value(&vol_n, n);
    
    #pragma omp target data map(tofrom: ptr[0:dynamic_len])
    {
        #pragma omp target
        for (int i = 0; i < dynamic_len; i++) {
            ptr[i] = i * i;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to create runtime-dependent values preventing constant folding */
    int size = (argc > 1) ? atoi(argv[1]) : N;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    int offset = (argc > 4) ? atoi(argv[4]) : 5;
    int cond = (argc > 5) ? atoi(argv[5]) : 1;
    
    /* Allocate arrays with dynamic size */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Process with array sections - multiple contexts */
    process_array_sections(arr1, arr2, start, length, offset, cond);
    
    /* Another call with different parameters for more coverage */
    problematic_array_section_usage(arr1, size / 2);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Taskwait to ensure task completion */
    #pragma omp taskwait
    
    free(arr1);
    free(arr2);
    
    return 0;
}
