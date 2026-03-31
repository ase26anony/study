/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections.
 *
 * The code uses complex base expressions (conditional operator, pointer
 * arithmetic, structure member access) with variable bounds to ensure
 * the op_prio() comparisons and non-constant bound handling are exercised.
 * Array sections are used in multiple OpenMP contexts (map, to, from, depend)
 * to increase the likelihood of the pretty-printer being invoked during
 * compilation dumps or diagnostic emission.
 *
 * Compilation suggestions for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent constant propagation of bounds */
static volatile int global_start = 5;
static volatile int global_len = 20;

/* Structure with array member to test member access as base */
struct with_array {
    int data[100];
    int offset;
};

/* Function that returns a pointer to an array, used to create
 * complex base expressions for array sections */
int* get_array(int choice, int *a, int *b) {
    return (choice > 0) ? a : b;
}

/* Function that uses array sections in map clauses with complex bases */
void target_computation(struct with_array *s, int *arr1, int *arr2, 
                        int n, int start, int len, int cond) {
    /* Use argc/argv-derived values to prevent constant folding */
    int lower = start;
    int length = len;
    
    /* 1. Array section with conditional operator as base - will trigger
     *    parentheses in pretty-printing due to op_prio comparison */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[lower:length])
    {
        #pragma omp target map(alloc: (cond ? arr1 : arr2)[lower:length])
        {
            for (int i = 0; i < length; i++) {
                int *base = cond ? arr1 : arr2;
                base[lower + i] += i;
            }
        }
    }
    
    /* 2. Array section with pointer arithmetic as base */
    int offset = s->offset;
    #pragma omp target enter data map(to: (arr1 + offset)[0:n])
    #pragma omp target map(alloc: (arr1 + offset)[0:n])
    {
        for (int i = 0; i < n; i++) {
            arr1[offset + i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (arr1 + offset)[0:n])
    
    /* 3. Array section with structure member access as base */
    #pragma omp target data map(tofrom: s->data[lower:length])
    {
        #pragma omp target map(alloc: s->data[lower:length])
        {
            for (int i = 0; i < length; i++) {
                s->data[lower + i] = i * i;
            }
        }
    }
    
    /* 4. Multiple array sections in different clauses */
    int *dynamic_base = get_array(cond, arr1, arr2);
    #pragma omp target map(to: dynamic_base[0:n]) \
                       map(from: arr2[n/2:n/2])
    {
        for (int i = 0; i < n; i++) {
            dynamic_base[i] = arr2[n/2 + i % (n/2)];
        }
    }
}

/* Function that uses array section in task depend clause */
void task_with_depend(int *arr, int start, int len) {
    int *section = &arr[start];
    
    #pragma omp task depend(inout: section[0:len])
    {
        for (int i = 0; i < len; i++) {
            section[i] += 1;
        }
    }
}

/* Deliberate type error to potentially trigger diagnostic pretty-printing */
void problematic_type_usage(int *arr, int n) {
    /* This may cause a warning/error about array section in non-OpenMP context */
    int *section = &arr[0];
    
    /* Attempt to use array section syntax in a non-OpenMP context
     * This is invalid C but may be parsed as an array section node */
    #ifdef TRIGGER_ERROR
    /* This would be invalid: int *ptr = arr[0:n]; */
    #endif
    
    /* But we can use it in OpenMP with potential type issues */
    double *wrong_type = (double *)arr;
    #pragma omp target data map(wrong_type[0:n])  /* Type mismatch may trigger diagnostic */
    {
        /* Empty - just for the map clause */
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments for dynamic bounds to prevent constant folding */
    int start = (argc > 1) ? atoi(argv[1]) : global_start;
    int len = (argc > 2) ? atoi(argv[2]) : global_len;
    int n = 100;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    struct with_array s = { .offset = 10 };
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = n - i;
        s.data[i] = 0;
    }
    
    /* Call function with complex array section usage */
    target_computation(&s, arr1, arr2, n, start, len, argc > 3);
    
    /* Use task with array section in depend clause */
    #pragma omp parallel
    #pragma omp single
    {
        task_with_depend(arr1, start, len);
        #pragma omp taskwait
    }
    
    /* Try to trigger diagnostic with problematic usage */
    if (argc > 4) {
        problematic_type_usage(arr1, n);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + s.data[i];
    }
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
