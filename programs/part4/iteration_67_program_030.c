/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-array-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-optimized
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int argc, char **argv) {
    volatile int v = base; /* volatile to prevent optimization */
    if (argc > 1) v += atoi(argv[1]) % 10;
    return v;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_with_array_sections(int *arr1, int *arr2, int n, int offset, int cond, int argc, char **argv) {
    int i, j;
    
    /* Use command-line arguments to make bounds non-constant */
    i = get_bound(offset, argc, argv);
    j = get_bound(n, argc, argv);
    
    /* STRATEGY 1: Complex base expression with conditional operator
       This should trigger op_prio checks for parentheses in pretty-printing */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[i:j])
    {
        /* STRATEGY 2: Complex base with pointer arithmetic */
        #pragma omp target map(tofrom: (arr1 + offset)[0:j/2])
        for (int k = 0; k < j/2; k++) {
            (arr1 + offset)[k] += 1;
        }
        
        /* STRATEGY 3: Another array section with different base */
        #pragma omp target map(tofrom: arr2[i:j/3])
        for (int k = 0; k < j/3; k++) {
            arr2[i + k] *= 2;
        }
    }
    
    /* STRATEGY 4: Array section in task depend clause
       This creates additional OMP_ARRAY_SECTION nodes */
    #pragma omp task depend(inout: arr1[offset:5])
    {
        for (int k = 0; k < 5; k++) {
            arr1[offset + k] = k;
        }
    }
    
    /* STRATEGY 5: Deliberate type warning/error potential
       Passing array section to function expecting pointer (without OpenMP context)
       This may trigger diagnostics that pretty-print the array section */
    /* Uncomment to potentially trigger warning:
    extern void expect_pointer(int *);
    expect_pointer(arr1[i:j]);  // This is invalid C but may be parsed as array section
    */
}

/* Structure with array member for member access base expression */
struct WithArray {
    int data[100];
    int *ptr;
};

void process_struct_sections(struct WithArray *s, int len, int argc, char **argv) {
    int start = get_bound(0, argc, argv);
    int count = get_bound(len/2, argc, argv);
    
    /* STRATEGY 6: Array section with structure member access as base */
    #pragma omp target data map(tofrom: s->data[start:count])
    {
        #pragma omp target
        for (int i = 0; i < count; i++) {
            s->data[start + i] = i * 2;
        }
    }
    
    /* STRATEGY 7: Nested complex base expression */
    #pragma omp target enter data map(to: (s->ptr + start)[0:count])
    #pragma omp target exit data map(from: (s->ptr + start)[0:count])
}

int main(int argc, char **argv) {
    const int N = 100;
    int *array1, *array2;
    struct WithArray s;
    int offset = 10;
    int cond = (argc > 2) ? atoi(argv[2]) % 2 : 1;
    
    /* Dynamic allocation prevents constant folding */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    s.ptr = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !s.ptr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        s.data[i] = i * 3;
        s.ptr[i] = i * 4;
    }
    
    /* Process with various array section patterns */
    process_with_array_sections(array1, array2, N, offset, cond, argc, argv);
    process_struct_sections(&s, N, argc, argv);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i] + s.data[i] + s.ptr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(s.ptr);
    
    return 0;
}
