/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * The array sections are used in multiple OpenMP contexts with variable bounds
 * and operator precedence scenarios to ensure the uncovered code block is executed.
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

/* Function expecting a pointer (may cause type warnings with array sections) */
static void expects_pointer(int *p) {
    if (p) *p = 0;
}

int main(int argc, char **argv) {
    /* Use volatile and argc to prevent constant propagation */
    volatile int base_size = 100;
    int size = base_size;
    int n = use_arg(argc, argv);
    int m = (argc > 2) ? atoi(argv[2]) : 10;
    
    if (n <= 0) n = 5;
    if (m <= 0) m = 10;
    if (n + m > size) size = n + m;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *ptr = arr1;
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* STRATEGY 1: Complex base expressions in OpenMP target data region */
    /* Base expression with pointer arithmetic (lower precedence than [:] ) */
    #pragma omp target data map(tofrom: (ptr + n)[0:m])
    {
        /* Inside the data region, run a target kernel */
        #pragma omp target map(tofrom: (ptr + n)[0:m])
        {
            for (int i = 0; i < m; i++) {
                (ptr + n)[i] += 1;
            }
        }
    }
    
    /* STRATEGY 2: Conditional operator as base (triggers op_prio parentheses logic) */
    int cond = (argc > 3);
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[n:m])
    {
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[n:m])
        {
            for (int i = 0; i < m; i++) {
                (cond ? arr1 : arr2)[n + i] *= 2;
            }
        }
    }
    
    /* STRATEGY 3: Structure member array section */
    struct S {
        int member_arr[200];
        int *member_ptr;
    } s;
    
    s.member_ptr = arr2;
    for (int i = 0; i < size; i++) s.member_arr[i] = i * 3;
    
    /* Array section on structure member array */
    #pragma omp target data map(tofrom: s.member_arr[n:m])
    {
        #pragma omp target map(tofrom: s.member_arr[n:m])
        {
            for (int i = 0; i < m; i++) {
                s.member_arr[n + i] -= 1;
            }
        }
    }
    
    /* STRATEGY 4: Multiple OpenMP clauses with array sections */
    /* Use in 'to', 'from', and 'alloc' clauses */
    int *arr3 = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) arr3[i] = i * 2;
    
    #pragma omp target data map(to: arr3[n:m]) map(from: arr3[m:n]) map(alloc: arr3[n+m/2:2])
    {
        /* Mixed array sections in same directive */
        #pragma omp target map(to: arr3[n:m]) map(from: arr3[m:n]) map(alloc: arr3[n+m/2:2])
        {
            for (int i = 0; i < m; i++) {
                if (n + i < size) arr3[n + i] += arr3[m + i];
            }
        }
    }
    
    /* STRATEGY 5: Array section in task depend clause */
    int *arr4 = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) arr4[i] = i;
    
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr4[n:m])
        {
            for (int i = 0; i < m; i++) {
                arr4[n + i] = 1;
            }
        }
        
        #pragma omp task depend(in: arr4[n:m])
        {
            for (int i = 0; i < m; i++) {
                arr4[n + i] += 2;
            }
        }
    }
    
    /* STRATEGY 6: Deliberate type checking - passing array section to function */
    /* This may generate warnings/errors that invoke the pretty-printer */
    /* expects_pointer(arr1[n:m]); */  /* Uncomment to potentially trigger diagnostics */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + arr4[i] + s.member_arr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
