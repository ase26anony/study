/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions and variable bounds to ensure the uncovered code block
 * is executed during compilation with appropriate dump flags.
 *
 * Compilation recommendations for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int argc, char **argv) {
    volatile int v = base;
    if (argc > 1) v += atoi(argv[1]) % 5;
    return v;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_sections(int n, int start, int len, int *arr1, int *arr2, int cond, int argc, char **argv) {
    int offset = get_bound(2, argc, argv);
    int size = get_bound(10, argc, argv);
    
    /* STRATEGY 1: Complex base expressions with operator precedence issues */
    /* Base expression: conditional operator (lower precedence than array section) */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:len])
    {
        /* Inside target data region - launch a kernel */
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[start:len])
        {
            for (int i = 0; i < len; i++) {
                (cond ? arr1 : arr2)[start + i] += i;
            }
        }
    }
    
    /* STRATEGY 2: Pointer arithmetic as base expression */
    int *ptr = arr1 + offset;
    #pragma omp target enter data map(to: ptr[0:size])
    #pragma omp target map(tofrom: ptr[0:size])
    {
        for (int i = 0; i < size; i++) {
            ptr[i] *= 2;
        }
    }
    #pragma omp target exit data map(from: ptr[0:size])
    
    /* STRATEGY 3: Structure member access */
    struct {
        int *member_array;
        int other;
    } s;
    s.member_array = arr2;
    s.other = 0;
    
    /* This may trigger type warnings/errors when pretty-printed */
    #pragma omp target data map(tofrom: s.member_array[offset:size])
    {
        #pragma omp target map(tofrom: s.member_array[offset:size])
        {
            for (int i = 0; i < size; i++) {
                s.member_array[offset + i] = i * i;
            }
        }
    }
    
    /* STRATEGY 4: Array section in task depend clause */
    int *task_arr = arr1;
    #pragma omp task depend(inout: task_arr[start:len])
    {
        for (int i = 0; i < len; i++) {
            task_arr[start + i] += 1;
        }
    }
    
    /* STRATEGY 5: Multiple clauses with different array sections */
    #pragma omp target data map(to: arr1[0:n/2]) map(from: arr2[n/4:3*n/4])
    {
        #pragma omp target map(to: arr1[0:n/2]) map(from: arr2[n/4:3*n/4])
        {
            for (int i = 0; i < n/2; i++) {
                arr2[n/4 + i] = arr1[i] + arr2[n/4 + i];
            }
        }
    }
}

/* Function that deliberately creates type issues for diagnostics */
void problematic_usage(int *ptr, int n) {
    /* This may trigger warnings about array section usage outside OpenMP context */
    int (*func_ptr)(int*) = NULL;
    
    /* Attempt to use array section in non-OpenMP context - may cause diagnostics */
    /* Commented out to allow compilation, but useful for diagnostic triggering:
    func_ptr = (int (*)(int*)) &ptr[0:n];
    */
    
    /* Use volatile to prevent optimization */
    volatile int *volatile_ptr = ptr;
    #pragma omp target map(tofrom: vol_ptr[0:n])
    {
        for (int i = 0; i < n; i++) {
            vol_ptr[i] = i;
        }
    }
}

int main(int argc, char **argv) {
    int n = 100;
    if (argc > 1) n = atoi(argv[1]) % 1000 + 100;
    
    /* Dynamic allocation prevents constant folding */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    if (!arr1 || !arr2) return 1;
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = n - i;
    }
    
    /* Get bounds from command line to prevent constant propagation */
    int start = get_bound(10, argc, argv);
    int len = get_bound(20, argc, argv);
    int cond = get_bound(1, argc, argv) % 2;
    
    /* Process with various array section patterns */
    process_sections(n, start, len, arr1, arr2, cond, argc, argv);
    
    /* Also call problematic function to potentially trigger diagnostics */
    problematic_usage(arr1, n/2);
    
    /* Compute checksum to prevent dead code elimination */
    long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr1[i] + arr2[i];
    }
    printf("Checksum: %ld\n", sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
