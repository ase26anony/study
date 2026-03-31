/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions to exercise the op_prio parenthesization logic, and
 * variable bounds to prevent constant folding.
 *
 * Compilation options for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant propagation */
static int get_bound(int base, int argc, char **argv) {
    volatile int v = base;
    if (argc > 1) v += atoi(argv[1]) % 5;
    return v;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_sections(int *arr1, int *arr2, int n, int offset, int len, int cond) {
    /* 1. Array section with pointer arithmetic base (triggers op_prio check) */
    #pragma omp target data map(tofrom: (arr1 + offset)[0:len])
    {
        #pragma omp target map(alloc: (arr1 + offset)[0:len])
        for (int i = 0; i < len; i++) {
            (arr1 + offset)[i] += 1;
        }
    }

    /* 2. Array section with conditional expression base (triggers op_prio check) */
    #pragma omp target enter data map(to: (cond ? arr1 : arr2)[0:n/2])
    #pragma omp target exit data map(from: (cond ? arr1 : arr2)[0:n/2])

    /* 3. Array section in depend clause (different context) */
    int *section_ptr = cond ? arr1 : arr2;
    #pragma omp task depend(inout: section_ptr[offset:len])
    {
        for (int i = 0; i < len; i++) {
            section_ptr[offset + i] *= 2;
        }
    }
    #pragma omp taskwait
}

/* Another function with structure member access as base */
struct Data {
    int *values;
    int count;
};

void process_struct_section(struct Data *d1, struct Data *d2, int start, int length, int flag) {
    /* Array section with structure member access as base */
    #pragma omp target data map(tofrom: (flag ? d1->values : d2->values)[start:length])
    {
        #pragma omp target teams distribute parallel for \
            map(alloc: (flag ? d1->values : d2->values)[start:length])
        for (int i = 0; i < length; i++) {
            (flag ? d1->values : d2->values)[start + i] += 3;
        }
    }
}

/* Function that might provoke type diagnostics */
void problematic_usage(int *arr, int n) {
    /* This may trigger warnings about array section in non-OpenMP context
     * when compiled with -Werror=openmp-mapping */
    int *section = &arr[0:n];  /* Array section used in non-OpenMP expression */
    (void)section;  /* Suppress unused warning */
}

int main(int argc, char **argv) {
    int N = 100;
    int offset = 10;
    int length = 20;
    
    /* Use command-line arguments to make bounds non-constant */
    N = get_bound(100, argc, argv);
    offset = get_bound(10, argc, argv);
    length = get_bound(20, argc, argv);
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    struct Data d1 = {arr1, N};
    struct Data d2 = {arr2, N};
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
    }
    
    /* Use array sections with complex base expressions */
    process_sections(arr1, arr2, N, offset, length, argc > 2);
    
    /* Use array sections with structure member access */
    process_struct_section(&d1, &d2, offset, length, argc > 3);
    
    /* Potentially problematic usage for diagnostics */
    if (argc > 4) {
        problematic_usage(arr1, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1[i] + arr2[i];
    }
    printf("Checksum: %ld\n", sum);
    
    free(arr1);
    free(arr2);
    return 0;
}
