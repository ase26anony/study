/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * The array sections are used in various OpenMP clauses with non-constant bounds
 * and base expressions involving operators of varying precedence.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 5;
}

/* Function that returns different arrays based on a condition */
static int *select_array(int cond, int *a, int *b) {
    return cond ? a : b;
}

/* Function expecting a pointer (not an array section) - may cause diagnostics */
static void expect_pointer(int *p) {
    if (p) *p = 0;
}

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent constant propagation */
    volatile int base_size = 100;
    int size = base_size;
    int offset = use_arg(argc, argv);
    int length = (offset > 0) ? offset : 10;
    
    /* Allocate arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *ptr = arr1;
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* Structure with array member */
    struct {
        int *member_array;
        int count;
    } struct_var = {arr1, size};
    
    /* 
     * STRATEGY 1: Array sections with complex base expressions in map clauses
     * These will create OMP_ARRAY_SECTION nodes during OpenMP lowering.
     */
    
    /* Base expression: conditional operator (lower precedence than array section) */
    #pragma omp target data map(tofrom: (argc > 2 ? arr1 : arr2)[offset:length])
    {
        /* Inside target data region */
        #pragma omp target map(alloc: (ptr + offset)[0:length])
        {
            for (int i = 0; i < length; i++) {
                /* Complex base: pointer arithmetic */
                (ptr + offset)[i] += 1;
            }
        }
    }
    
    /* 
     * STRATEGY 2: Array section with structure member access as base
     * This tests another operator precedence scenario.
     */
    int start = offset % 20;
    int len = length % 20;
    
    #pragma omp target enter data map(to: struct_var.member_array[start:len])
    
    #pragma omp target map(alloc: struct_var.member_array[start:len])
    {
        for (int i = 0; i < len; i++) {
            struct_var.member_array[start + i] *= 2;
        }
    }
    
    #pragma omp target exit data map(from: struct_var.member_array[start:len])
    
    /*
     * STRATEGY 3: Array sections in task depend clauses
     * Creates additional OMP_ARRAY_SECTION nodes.
     */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr1[0:length])
            {
                for (int i = 0; i < length; i++) arr1[i] = i * 3;
            }
            
            #pragma omp task depend(in: arr1[0:length])
            {
                for (int i = 0; i < length; i++) arr1[i] += 1;
            }
        }
    }
    
    /*
     * STRATEGY 4: Potential diagnostic triggers
     * Passing array section to function expecting pointer (without proper OpenMP context).
     * This may generate warnings/errors that invoke the pretty-printer.
     */
    /* Uncomment to potentially trigger diagnostics:
    expect_pointer(arr1[offset:length]);
    */
    
    /* 
     * STRATEGY 5: Multiple clauses with different array sections
     * Increases variety of OMP_ARRAY_SECTION contexts.
     */
    int chunk = length / 2;
    #pragma omp target data map(to: arr2[0:chunk]) map(from: arr2[chunk:chunk])
    {
        #pragma omp target
        {
            for (int i = 0; i < chunk; i++) {
                arr2[i] = arr2[i + chunk];
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
