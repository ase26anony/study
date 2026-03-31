/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections
 * with parentheses for complex base expressions.
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
static int get_value(int base, int increment) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + increment;
}

/* Function that uses array sections in a map clause with complex base */
void process_section(int *arr, int offset, int size, int n) {
    /* Complex base expression: pointer arithmetic with parentheses */
    #pragma omp target data map(to: (arr + offset)[0:size])
    {
        /* Another complex base: conditional operator */
        int *base_ptr = (offset > n/2) ? arr : (arr + n/2);
        #pragma omp target map(tofrom: base_ptr[0:size/2])
        {
            for (int i = 0; i < size/2; i++) {
                base_ptr[i] += i;
            }
        }
    }
}

/* Function with deliberate type-ish warning potential */
void mixed_sections(int *ptr1, int *ptr2, int len) {
    /* Using array section in depend clause (OpenMP 4.5+) */
    #pragma omp task depend(inout: ptr1[0:len])
    {
        for (int i = 0; i < len; i++) ptr1[i] *= 2;
    }
    
    /* Different base: structure-like access through pointer */
    struct wrapper { int *data; } w;
    w.data = ptr2;
    #pragma omp task depend(inout: ptr2[0:len]) depend(in: ptr1[0:len])
    {
        for (int i = 0; i < len; i++) w.data[i] += ptr1[i];
    }
    #pragma omp taskwait
}

int main(int argc, char *argv[]) {
    /* Use argc to create runtime-dependent sizes, preventing constant folding */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    if (n < 10) n = 10;
    
    /* Dynamic allocation ensures no compile-time knowledge of addresses */
    int *array1 = (int *)malloc(n * sizeof(int));
    int *array2 = (int *)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array1[i] = i;
        array2[i] = n - i;
    }
    
    /* Get bounds from volatile/function to prevent constant propagation */
    int start = get_value(1, argc);
    int length = get_value(n/4, argc);
    
    /* First: array section with pointer arithmetic base */
    printf("Processing section 1...\n");
    process_section(array1, start, length, n);
    
    /* Second: array section in multiple depend clauses with complex bases */
    printf("Processing section 2...\n");
    mixed_sections(array1 + start, array2 + start, length);
    
    /* Third: nested array sections in target enter/exit data */
    int offset = get_value(2, 0);
    int section_len = get_value(5, 0);
    #pragma omp target enter data map(to: (array1 + offset)[0:section_len])
    #pragma omp target map(alloc: (array1 + offset)[0:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            (array1 + offset)[i] += 100;
        }
    }
    #pragma omp target exit data map(from: (array1 + offset)[0:section_len])
    
    /* Compute checksum to prevent dead code elimination */
    long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array1[i] + array2[i];
    }
    printf("Checksum: %ld\n", sum);
    
    free(array1);
    free(array2);
    return 0;
}
