/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-test.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-optimized
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_value(int base, int increment) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + increment;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_array_sections(int *arr1, int *arr2, int n, int offset, int len, int cond) {
    /* Complex base expression 1: conditional operator (lower precedence than array section) */
    int *base_ptr = cond ? arr1 : arr2;
    
    /* OpenMP target data region with array section having complex base */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[offset:len]) \
                            map(from: (base_ptr + offset)[0:len/2])
    {
        /* Nested target region with different array section */
        #pragma omp target map(alloc: (arr1 + offset)[0:len]) \
                           map(tofrom: (arr2)[offset:len/2])
        {
            for (int i = 0; i < len/2; i++) {
                arr2[offset + i] = arr1[offset + i] * 2;
            }
        }
        
        /* Another complex base: pointer arithmetic */
        #pragma omp target map(tofrom: (arr1 + get_value(offset, 1))[0:len/3])
        {
            for (int i = 0; i < len/3; i++) {
                arr1[offset + 1 + i] += i;
            }
        }
    }
    
    /* Task with depend clause using array section */
    #pragma omp task depend(inout: arr1[offset:len/2])
    {
        for (int i = 0; i < len/2; i++) {
            arr1[offset + i] = arr1[offset + i] * 3;
        }
    }
    
    #pragma omp taskwait
}

/* Structure with array member to create member access base expression */
struct DataContainer {
    int *member_array;
    int size;
};

void process_struct_section(struct DataContainer *container, int start, int length) {
    /* Array section with structure member access as base */
    #pragma omp target data map(tofrom: container->member_array[start:length])
    {
        #pragma omp target
        {
            for (int i = 0; i < length; i++) {
                container->member_array[start + i] += start + i;
            }
        }
    }
}

/* Function that might provoke type-related diagnostics */
void problematic_section_usage(int *arr, int n) {
    /* This could trigger warnings about array section in non-OpenMP context */
    int *section_ptr = &arr[2:4]; /* Invalid in plain C, but OpenMP parser handles it */
    
    /* Using array section in is_device_ptr clause with pointer arithmetic */
    int *ptr = arr + 5;
    #pragma omp target data map(to: arr[0:n]) is_device_ptr(ptr[0:3])
    {
        /* Empty but creates the tree nodes */
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int offset = (argc > 2) ? atoi(argv[2]) : 10;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    int cond = (argc > 4) ? atoi(argv[4]) : 1;
    
    if (size <= 0) size = 100;
    if (offset < 0) offset = 10;
    if (length <= 0) length = 20;
    if (offset + length > size) length = size - offset;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Process with complex array sections */
    process_array_sections(arr1, arr2, size, offset, length, cond);
    
    /* Process with struct member array section */
    struct DataContainer container;
    container.member_array = arr1;
    container.size = size;
    process_struct_section(&container, offset/2, length/2);
    
    /* Try to trigger diagnostics with problematic usage */
    if (argc > 5) {
        problematic_section_usage(arr1, size);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
