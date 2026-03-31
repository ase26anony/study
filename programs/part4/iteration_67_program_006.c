/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * 
 * Compilation flags for coverage analysis:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
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
void process_sections(int *arr1, int *arr2, int n, int start, int len, int cond) {
    /* Complex base expression 1: conditional operator (lower precedence than []) */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:len])
    {
        /* Complex base expression 2: pointer arithmetic */
        #pragma omp target map(tofrom: (arr1 + start)[0:len])
        for (int i = 0; i < len; i++) {
            (arr1 + start)[i] += 1;
        }
        
        /* Another array section with different base */
        #pragma omp target map(to: arr2[start:len/2])
        for (int i = 0; i < len/2; i++) {
            arr2[start + i] = arr1[start + i] * 2;
        }
    }
    
    /* Use array section in task depend clause */
    #pragma omp task depend(inout: arr1[start:len])
    {
        for (int i = 0; i < len; i++) {
            arr1[start + i] *= 3;
        }
    }
    
    /* Potential type warning: array section in non-OpenMP context */
    /* This may trigger diagnostics during compilation */
    int *ptr = cond ? &arr1[start] : &arr2[start];
    /* The line below might generate a warning about array section usage */
    /* int *section_ptr = &arr1[start:len]; */ /* Invalid in non-OpenMP context */
}

/* Structure with array member for member access base expression */
struct DataContainer {
    int header;
    int values[100];
    int *ptr_array;
};

void process_struct_sections(struct DataContainer *dc1, 
                             struct DataContainer *dc2, 
                             int idx, int len) {
    /* Complex base expression 3: structure member access */
    #pragma omp target data map(tofrom: dc1->values[idx:len])
    {
        #pragma omp target map(to: dc2->values[idx:len])
        for (int i = 0; i < len; i++) {
            dc1->values[idx + i] += dc2->values[idx + i];
        }
    }
    
    /* Complex base expression 4: nested conditional with struct member */
    #pragma omp target enter data map(to: (idx > 0 ? dc1 : dc2)->ptr_array[0:len])
    #pragma omp target exit data map(from: (idx > 0 ? dc1 : dc2)->ptr_array[0:len])
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
    
    /* Dynamic allocation prevents compile-time optimization */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    struct DataContainer dc1, dc2;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = size - i;
    }
    
    dc1.header = 1;
    dc2.header = 2;
    dc1.ptr_array = array1;
    dc2.ptr_array = array2;
    for (int i = 0; i < 100; i++) {
        dc1.values[i] = i * 2;
        dc2.values[i] = i * 3;
    }
    
    /* Process with complex array section expressions */
    process_sections(array1, array2, size, start, length, cond);
    
    /* Process struct-based array sections */
    int struct_idx = get_value(5, 3);  /* Volatile prevents constant folding */
    int struct_len = get_value(10, 2);
    process_struct_sections(&dc1, &dc2, struct_idx, struct_len);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i];
    }
    for (int i = 0; i < 100; i++) {
        checksum += dc1.values[i] + dc2.values[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
