/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-array-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-optimized
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_array_sections(int n, int m, int *arr1, int *arr2, int *ptr, int offset, int len) {
    /* 1. Array section with pointer arithmetic base (triggers op_prio check) */
    #pragma omp target data map(tofrom: (ptr + offset)[0:len])
    {
        #pragma omp target map(alloc: (ptr + offset)[0:len/2])
        for (int i = 0; i < len/2; i++) {
            (ptr + offset)[i] = i * 2;
        }
    }

    /* 2. Array section with conditional operator base (complex precedence) */
    int *base_ptr = (n > m) ? arr1 : arr2;
    #pragma omp target enter data map(to: base_ptr[0:n])
    #pragma omp target map(alloc: base_ptr[0:n])
    for (int i = 0; i < n; i++) {
        base_ptr[i] += i;
    }
    #pragma omp target exit data map(from: base_ptr[0:n])

    /* 3. Array section in depend clause for task */
    int *section_start = arr1 + 2;
    #pragma omp task depend(inout: section_start[0:5])
    {
        for (int i = 0; i < 5; i++) {
            section_start[i] *= 2;
        }
    }
    #pragma omp taskwait
}

/* Function with deliberate type warning potential */
void problematic_section(int *arr, int start, int length) {
    /* This may trigger diagnostics about array section usage */
    volatile int *volatile_ptr = arr; /* volatile to prevent optimization */
    #pragma omp target data map(tofrom: (int *)volatile_ptr[start:length])
    {
        /* Empty but still creates the array section node */
    }
}

int main(int argc, char **argv) {
    int size = use_arg(argc, argv);
    int offset = (argc > 2) ? atoi(argv[2]) : 2;
    int section_len = (argc > 3) ? atoi(argv[3]) : 10;

    /* Dynamic allocation prevents constant folding */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    int *dyn_ptr = (int *)malloc((size + offset) * sizeof(int));

    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = size - i;
        dyn_ptr[i] = 0;
    }

    /* Process with complex array section bases */
    process_array_sections(size, size/2, array1, array2, dyn_ptr, offset, section_len);

    /* Another usage with struct member access (simulated via pointer offset) */
    struct { int *data; int count; } wrapper;
    wrapper.data = array1;
    wrapper.count = size;
    #pragma omp target data map(tofrom: wrapper.data[offset:section_len])
    {
        #pragma omp target
        for (int i = 0; i < section_len; i++) {
            wrapper.data[offset + i] += 3;
        }
    }

    /* Call function that might trigger diagnostics */
    problematic_section(array2, 1, 5);

    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i];
    }
    printf("Checksum: %d\n", checksum);

    free(array1);
    free(array2);
    free(dyn_ptr);
    return 0;
}
