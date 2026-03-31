/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compilation options for coverage:
 *   1. Debug dumps: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower test-omp-array-section.c
 *   2. Diagnostics: gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   3. Optimization: gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int get_value(int base, int offset) {
    volatile int v = base;
    return v + offset;
}

/* Function that uses array sections in multiple contexts */
void process_sections(int *arr1, int *arr2, int start, int length, int n, int cond) {
    /* Complex base expression 1: conditional operator (lower precedence than []) */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:length])
    {
        /* Complex base expression 2: pointer arithmetic */
        #pragma omp target map(tofrom: (arr1 + start)[0:length])
        {
            for (int i = 0; i < length; i++) {
                (arr1 + start)[i] += 1;
            }
        }
    }
    
    /* Another usage with different clause */
    #pragma omp target enter data map(to: arr2[start:length])
    
    /* Use in task depend clause */
    #pragma omp task depend(inout: arr1[start:length])
    {
        for (int i = 0; i < length; i++) {
            arr1[start + i] *= 2;
        }
    }
    
    #pragma omp taskwait
    #pragma omp target exit data map(from: arr2[start:length])
}

/* Function with deliberate type issues to provoke diagnostics */
void problematic_usage(int *ptr, int n) {
    int local_arr[100];
    
    /* This may trigger warnings about array section in non-OpenMP context */
    /* when compiled with -Werror=openmp-mapping */
    #pragma omp target data map(tofrom: ptr[0:n])
    {
        /* Mixing array sections with incompatible base */
        int *dyn_ptr = (int *)malloc(n * sizeof(int));
        if (dyn_ptr) {
            /* Complex base: function call result */
            #pragma omp target map(tofrom: (dyn_ptr ? dyn_ptr : ptr)[0:n])
            {
                for (int i = 0; i < n; i++) {
                    (dyn_ptr ? dyn_ptr : ptr)[i] = i;
                }
            }
            free(dyn_ptr);
        }
    }
}

/* Structure with array member for member access base expression */
struct DataContainer {
    int header;
    int values[100];
    int *ptr;
};

void process_struct(struct DataContainer *dc, int start, int len) {
    /* Complex base: structure member access */
    #pragma omp target data map(tofrom: dc->values[start:len])
    {
        /* Nested complex base: pointer from structure member */
        #pragma omp target map(tofrom: (dc->ptr + start)[0:len])
        {
            for (int i = 0; i < len; i++) {
                dc->values[start + i] += dc->ptr[start + i];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    int cond = (argc > 4) ? atoi(argv[4]) : 1;
    
    if (size < start + length) {
        fprintf(stderr, "Error: insufficient array size\n");
        return 1;
    }
    
    /* Dynamic allocation prevents static analysis */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    struct DataContainer dc;
    dc.ptr = (int *)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !dc.ptr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        dc.ptr[i] = i * 2;
        dc.values[i] = i * 3;
    }
    
    /* Process with complex array section bases */
    process_sections(arr1, arr2, start, length, size, cond);
    
    /* Process with problematic usage (may trigger diagnostics) */
    problematic_usage(arr1, get_value(5, 5));
    
    /* Process structure with member array sections */
    process_struct(&dc, start, length);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + dc.values[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dc.ptr);
    
    return 0;
}
