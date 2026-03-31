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

/* Function that returns different arrays based on condition */
int* select_array(int cond, int* a, int* b) {
    return cond ? a : b;
}

/* Function expecting pointer (to potentially cause type warnings) */
void expect_pointer(int *p) {
    if (p) *p = 0;
}

int main(int argc, char **argv) {
    /* Use argc/argv to make bounds non-constant */
    volatile int start = use_arg(argc, argv);
    int length = use_arg(argc, argv) + 3;
    int offset = 2;
    
    /* Initialize arrays */
    int N = 100;
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *ptr = arr1;
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    
    /* STRUCT with array member (for member access base expression) */
    struct {
        int *member_array;
        int x;
    } struct_var;
    struct_var.member_array = arr1;
    struct_var.x = 10;
    
    /* 1. Target data region with array section using complex base expression:
          (conditional operator as base) */
    #pragma omp target data map(tofrom: (select_array(start > 5, arr1, arr2))[start:length])
    {
        /* 2. Target region inside with another array section:
              (pointer arithmetic as base) */
        #pragma omp target map(tofrom: (ptr + offset)[0:length])
        {
            for (int i = 0; i < length; i++) {
                (ptr + offset)[i] += 1;
            }
        }
        
        /* 3. Another array section: structure member access as base */
        #pragma omp target map(to: struct_var.member_array[offset:length])
        {
            for (int i = 0; i < length; i++) {
                struct_var.member_array[offset + i] *= 2;
            }
        }
    }
    
    /* 4. Task depend clause with array section (different context) */
    int *task_arr = arr2;
    #pragma omp task depend(inout: task_arr[start:length])
    {
        for (int i = 0; i < length; i++) {
            task_arr[start + i] += 3;
        }
    }
    
    /* 5. Deliberate type warning/error candidate: 
          Passing array section to function expecting pointer */
    /* This may trigger diagnostics with -Werror=openmp-mapping */
    /* expect_pointer(arr1[start:length]); */ /* Uncomment for diagnostic testing */
    
    /* 6. OpenACC-style array section in enter/exit data */
    #pragma omp target enter data map(to: arr1[0:length])
    #pragma omp target exit data map(from: arr1[0:length])
    
    /* Compute checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i];
    }
    printf("Checksum: %d\n", sum);
    
    free(arr1);
    free(arr2);
    return 0;
}
