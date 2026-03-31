/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions and variable bounds in multiple OpenMP contexts.
 *
 * Compilation flags to trigger pretty-printing:
 *   -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower
 *   -O0 -fopenmp -Wall -Werror=openmp-mapping -c
 *   -O2 -fopenmp -foffload=disable -fdump-tree-optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int get_value(int x, int def) {
    volatile int v = x; /* volatile to prevent optimization */
    return v > 0 ? v : def;
}

/* Function that uses array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int n, int offset, int len, int cond) {
    /* Complex base expression 1: conditional operator as base */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[offset:len])
    {
        /* Complex base expression 2: pointer arithmetic as base */
        #pragma omp target map(tofrom: (arr1 + offset)[0:len])
        for (int i = 0; i < len; i++) {
            (arr1 + offset)[i] += i;
        }
    }

    /* Another usage in a different clause type */
    int *ptr = cond ? arr1 : arr2;
    #pragma omp target enter data map(to: ptr[offset:len])
    #pragma omp target exit data map(from: ptr[offset:len])
}

/* Structure with array member for member access base expression */
struct Data {
    int header;
    int values[100];
    int *ptr;
};

void process_struct_sections(struct Data *d1, struct Data *d2, 
                             int start, int count, int cond) {
    /* Complex base: structure member access with conditional */
    int *base = (cond ? d1 : d2)->values;
    #pragma omp target data map(tofrom: base[start:count])
    {
        #pragma omp target
        for (int i = 0; i < count; i++) {
            base[start + i] *= 2;
        }
    }

    /* Another complex base: pointer member with offset */
    #pragma omp target map(to: (d1->ptr + start)[0:count])
    {
        for (int i = 0; i < count; i++) {
            d1->ptr[start + i] = i;
        }
    }
}

/* Function that might trigger diagnostics with array sections */
void problematic_usage(int *arr, int n) {
    /* This could trigger warnings about array section usage
     * when not in proper OpenMP context */
    int *section = &arr[10];  /* Not an array section, but might be confused */
    
    /* Actual array section in depend clause for task */
    #pragma omp task depend(inout: arr[0:n])
    {
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    /* Array section in is_device_ptr clause */
    int *dev_ptr;
    #pragma omp target data map(to: arr[0:n])
    {
        #pragma omp target is_device_ptr(arr)
        {
            /* Access device pointer */
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int size = get_value(argc, 100);
    int offset = get_value(atoi(argv[0]), 10);
    int length = get_value(size / 2, 20);
    int cond = argc > 2;

    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    struct Data d1, d2;
    d1.ptr = (int *)malloc(size * sizeof(int));
    d2.ptr = (int *)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        d1.values[i % 100] = i;
        d2.values[i % 100] = size - i;
        d1.ptr[i] = 0;
        d2.ptr[i] = 0;
    }

    /* Use array sections with complex base expressions */
    process_sections(arr1, arr2, size, offset, length, cond);
    
    /* Use array sections with structure member bases */
    process_struct_sections(&d1, &d2, offset % 50, length % 50, cond);
    
    /* Potentially problematic usage that might trigger diagnostics */
    problematic_usage(arr1, length);

    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + d1.values[i % 100] + d2.values[i % 100];
    }
    
    printf("Checksum: %lld\n", checksum);

    /* Cleanup */
    free(arr1);
    free(arr2);
    free(d1.ptr);
    free(d2.ptr);

    return 0;
}
