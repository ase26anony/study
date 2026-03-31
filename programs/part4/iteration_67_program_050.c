/* test_omp_array_section.c
 * 
 * This program demonstrates OpenMP array section usage with complex base
 * expressions to trigger the OMP_ARRAY_SECTION pretty-printing logic in
 * GCC's tree-pretty-print.cc. The code uses variable bounds, pointer
 * arithmetic, conditional operators, and structure member accesses as
 * base expressions for array sections within OpenMP directives.
 *
 * Compilation recommendations for coverage:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test_omp_array_section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test_omp_array_section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test_omp_array_section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Structure containing an array member for member access tests */
struct with_array {
    int *member_array;
    int len;
};

/* Function using array sections with complex base expressions */
void target_computation(struct with_array *s1, struct with_array *s2, 
                        int *ptr, int offset, int n, int cond) {
    /* Complex base 1: conditional operator as base */
    int *base1 = cond ? s1->member_array : s2->member_array;
    
    /* Complex base 2: pointer arithmetic as base */
    int *base2 = ptr + offset;
    
    /* Use volatile to prevent constant folding of bounds */
    volatile int start = offset;
    volatile int length = n / 2;
    
    /* Target data region with array sections having complex bases */
    #pragma omp target data \
        map(to: (cond ? s1->member_array : s2->member_array)[start:length]) \
        map(to: (ptr + offset)[0:n]) \
        map(to: s1->member_array[0:s1->len]) \
        map(from: base2[start:length])
    {
        /* Nested target region with different array section usage */
        #pragma omp target \
            map(alloc: (ptr + offset)[0:n]) \
            map(tofrom: s1->member_array[start:length])
        {
            int i;
            #pragma omp parallel for
            for (i = 0; i < length; i++) {
                /* Use both array sections in computation */
                if (cond) {
                    s1->member_array[start + i] += base2[i];
                } else {
                    (ptr + offset)[i] = s1->member_array[start + i] * 2;
                }
            }
        }
        
        /* Additional usage: array section in task depend clause */
        #pragma omp task depend(inout: base1[start:length])
        {
            for (int i = 0; i < length; i++) {
                base1[start + i] += 1;
            }
        }
        #pragma omp taskwait
    }
}

/* Function that deliberately creates type warning opportunities */
void problematic_usage(int *arr, int n) {
    /* This may trigger diagnostics about array section usage */
    int *section = &arr[0:n];  /* Array section in non-OpenMP context */
    (void)section;  /* Suppress unused warning */
    
    /* OpenMP usage with potential type issues */
    #pragma omp target enter data map(to: arr[0:n])
    #pragma omp target exit data map(from: arr[0:n])
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int offset = (argc > 2) ? atoi(argv[2]) : 10;
    int cond = (argc > 3) ? atoi(argv[3]) : 1;
    
    if (size <= 0) size = 100;
    if (offset < 0) offset = 10;
    
    /* Allocate and initialize arrays */
    int *array1 = (int *)malloc(size * sizeof(int));
    int *array2 = (int *)malloc(size * sizeof(int));
    int *array3 = (int *)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        array3[i] = i * 3;
    }
    
    /* Initialize structures with array members */
    struct with_array s1 = {array1, size};
    struct with_array s2 = {array2, size};
    
    /* Call function with complex array section bases */
    target_computation(&s1, &s2, array3, offset, size, cond);
    
    /* Call function that may trigger diagnostics */
    problematic_usage(array1, size);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + array3[i];
    }
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
