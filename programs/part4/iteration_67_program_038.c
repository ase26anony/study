/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-test.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-all
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Struct with array member to create complex base expressions */
struct Data {
    int *array;
    int len;
};

/* Function that uses array sections in OpenMP directives */
void process_sections(struct Data *d1, struct Data *d2, int start, int length, int cond, int argc, char **argv) {
    volatile int dynamic_start = start; /* volatile prevents constant propagation */
    int dynamic_len = length + use_arg(argc, argv);
    
    /* Complex base expression 1: conditional operator with lower precedence than [] */
    #pragma omp target data map(to: (cond ? d1->array : d2->array)[dynamic_start:dynamic_len])
    {
        /* Complex base expression 2: pointer arithmetic */
        #pragma omp target map(tofrom: (d1->array + dynamic_start)[0:dynamic_len])
        for (int i = 0; i < dynamic_len; i++) {
            (d1->array + dynamic_start)[i] += i;
        }
    }
    
    /* Another usage in different clause for more coverage */
    int *ptr = d2->array;
    #pragma omp target enter data map(to: ptr[dynamic_start:dynamic_len])
    
    /* Use in task depend clause */
    #pragma omp task depend(inout: ptr[dynamic_start:dynamic_len])
    {
        for (int i = 0; i < dynamic_len; i++) {
            ptr[dynamic_start + i] *= 2;
        }
    }
    
    #pragma omp target exit data map(from: ptr[dynamic_start:dynamic_len])
}

/* Function with deliberate type issues to trigger diagnostics */
void problematic_usage(int *arr, int n) {
    /* This may trigger warnings about array section usage outside OpenMP context */
    int *section = &arr[0:n]; /* Array section in non-OpenMP context */
    (void)section;
    
    /* OpenMP usage with complex bounds */
    int volatile v = n; /* volatile prevents constant folding */
    #pragma omp target data map(tofrom: arr[v-2:v+2])
    {
        #pragma omp target
        for (int i = 0; i < n; i++) {
            arr[i] = i;
        }
    }
}

int main(int argc, char **argv) {
    int size = 100;
    if (argc > 2) size = atoi(argv[2]);
    
    /* Allocate and initialize arrays */
    struct Data d1, d2;
    d1.len = size;
    d2.len = size;
    d1.array = (int *)malloc(size * sizeof(int));
    d2.array = (int *)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        d1.array[i] = i;
        d2.array[i] = size - i;
    }
    
    /* Use command-line arguments for dynamic bounds */
    int start = use_arg(argc, argv);
    int length = (argc > 3) ? atoi(argv[3]) : 10;
    int cond = (argc > 4) ? atoi(argv[4]) : 1;
    
    /* Process with complex array section expressions */
    process_sections(&d1, &d2, start, length, cond, argc, argv);
    
    /* Also call function that may trigger diagnostics */
    problematic_usage(d1.array, d1.len);
    
    /* Compute checksum to prevent dead code elimination */
    long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += d1.array[i] + d2.array[i];
    }
    printf("Checksum: %ld\n", sum);
    
    free(d1.array);
    free(d2.array);
    return 0;
}
