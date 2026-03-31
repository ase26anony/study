/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-array-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-optimized
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that takes a pointer to allow array section usage */
void process_section(int *base, int start, int length) {
    /* This may generate diagnostics if array section is used incorrectly */
    #pragma omp target map(tofrom: base[start:length])
    for (int i = 0; i < length; i++) {
        base[start + i] += i;
    }
}

/* Struct with array member to test member access as base expression */
struct WithArray {
    int data[N];
    int *ptr;
};

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent constant propagation */
    volatile int offset = 10;
    int size = use_arg(argc, argv);
    int start = (argc > 2) ? atoi(argv[2]) : 20;
    int length = (argc > 3) ? atoi(argv[3]) : 30;
    
    int arr1[N], arr2[N];
    int *ptr1 = arr1;
    struct WithArray s = {0};
    s.ptr = arr2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        s.data[i] = i * 2;
    }
    
    /* STRATEGY 1: Complex base expressions in OpenMP data mapping */
    
    /* Case 1: Pointer arithmetic as base - may need parentheses in dump */
    #pragma omp target data map(tofrom: (ptr1 + offset)[0:size])
    {
        #pragma omp target map(tofrom: (ptr1 + offset)[0:size])
        for (int i = 0; i < size; i++) {
            ptr1[offset + i] *= 2;
        }
    }
    
    /* Case 2: Conditional operator as base - low precedence triggers op_prio check */
    int cond = (argc > 1);
    #pragma omp target enter data map(to: (cond ? arr1 : arr2)[start:length])
    
    /* Case 3: Structure member access as base */
    #pragma omp target map(tofrom: s.data[5:15])
    for (int i = 0; i < 15; i++) {
        s.data[5 + i] += 1;
    }
    
    #pragma omp target exit data map(from: (cond ? arr1 : arr2)[start:length])
    
    /* STRATEGY 2: Multiple OpenMP clauses with array sections */
    
    /* Using array section in depend clause (OpenMP 4.5+) */
    #pragma omp task depend(inout: arr1[0:10])
    {
        for (int i = 0; i < 10; i++) arr1[i] = 0;
    }
    
    /* Using array section in to/from clauses */
    #pragma omp target map(to: arr2[5:20]) map(from: arr1[5:20])
    {
        for (int i = 0; i < 20; i++) {
            arr1[5 + i] = arr2[5 + i];
        }
    }
    
    /* STRATEGY 3: Potential type checking for diagnostics */
    /* This might generate warnings/errors about array section usage */
    process_section(arr1, 30, 20);
    
    /* STRATEGY 4: Nested complex expressions */
    int *ptr2 = arr2 + 5;
    #pragma omp target data map(to: ((argc > 2) ? ptr1 : ptr2)[0:size])
    {
        /* Mixed base expression with pointer arithmetic and conditional */
        #pragma omp target map(from: ((argc % 2) ? s.ptr : ptr1)[10:15])
        for (int i = 0; i < 15; i++) {
            int *base = (argc % 2) ? s.ptr : ptr1;
            base[10 + i] = i;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i] + s.data[i];
    }
    printf("Checksum: %d\n", sum);
    
    return 0;
}
