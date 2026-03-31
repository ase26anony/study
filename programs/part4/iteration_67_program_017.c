/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions to exercise the op_prio comparison and parenthesization
 * logic, and variable bounds to prevent constant folding.
 *
 * Compilation options for coverage:
 *   -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower
 *   -O0 -fopenmp -Wall -Werror=openmp-mapping -c
 *   -O2 -fopenmp -foffload=disable -fdump-tree-optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant propagation */
static int get_value(int base, int increment) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + increment;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_array_sections(int *arr1, int *arr2, int n, int offset, int len, int cond) {
    /* Complex base expression 1: conditional operator as base */
    /* This should trigger op_prio check for parenthesization */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[offset:len])
    {
        #pragma omp target map(alloc: (cond ? arr1 : arr2)[offset:len])
        {
            for (int i = 0; i < len; i++) {
                (cond ? arr1 : arr2)[offset + i] += i;
            }
        }
    }

    /* Complex base expression 2: pointer arithmetic as base */
    /* ptr + offset has lower precedence than array section */
    int *ptr = arr1;
    #pragma omp target enter data map(to: (ptr + offset)[0:len])
    
    #pragma omp target map(from: (ptr + offset)[0:len])
    {
        for (int i = 0; i < len; i++) {
            (ptr + offset)[i] *= 2;
        }
    }
    
    #pragma omp target exit data map(from: (ptr + offset)[0:len])

    /* Array section with structure member access */
    struct {
        int *data;
        int size;
    } vec = {arr2, n};
    
    /* vec.data has lower precedence than array section */
    int start = get_value(0, 1);
    int length = get_value(len, -1);
    #pragma omp target data map(tofrom: vec.data[start:length])
    {
        #pragma omp target
        {
            for (int i = 0; i < length; i++) {
                vec.data[start + i] += 3;
            }
        }
    }

    /* Use array section in task depend clause */
    int *dep_arr = arr1;
    #pragma omp task depend(inout: dep_arr[offset:len])
    {
        for (int i = 0; i < len; i++) {
            dep_arr[offset + i] += 5;
        }
    }
    
    #pragma omp taskwait
}

/* Function that might provoke type checking diagnostics */
void problematic_usage(int *arr, int n) {
    /* This usage might trigger warnings about array sections 
     * outside of OpenMP data clauses */
    int *section = &arr[2:4];  /* This is invalid C but parsed as array section */
    (void)section; /* Suppress unused warning */
    
    /* Array section in is_device_ptr clause with pointer arithmetic */
    int *dev_ptr;
    #pragma omp target data map(to: arr[0:n]) use_device_ptr(arr)
    {
        dev_ptr = arr;
    }
    
    /* Using array section in is_device_ptr would be invalid but
     * helps create the tree node for pretty-printing */
    #pragma omp target is_device_ptr((dev_ptr + 2)[0:n-2])
    {
        for (int i = 0; i < n-2; i++) {
            (dev_ptr + 2)[i] += 1;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int base_size = 100;
    if (argc > 1) {
        base_size = atoi(argv[1]);
        if (base_size < 20) base_size = 20;
    }
    
    int offset = get_value(5, 0);
    int length = get_value(10, 0);
    int cond = get_value(1, 0);
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(base_size * sizeof(int));
    int *arr2 = (int *)malloc(base_size * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < base_size; i++) {
        arr1[i] = i;
        arr2[i] = base_size - i;
    }
    
    /* Process with various array section patterns */
    process_array_sections(arr1, arr2, base_size, offset, length, cond);
    
    /* Try problematic usage to potentially trigger diagnostics */
    problematic_usage(arr1, base_size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < base_size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
