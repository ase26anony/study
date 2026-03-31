/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions to exercise the op_prio parenthesization logic, variable
 * bounds to prevent constant folding, and multiple OpenMP contexts to
 * increase the likelihood of diagnostic or dump output.
 *
 * Compile with:
 *   gcc -std=c99 -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower \
 *       -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *
 * Or for optimization dumps:
 *   gcc -std=c99 -O2 -fopenmp -foffload=disable -fdump-tree-optimized \
 *       -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper to prevent constant propagation */
static volatile int g_volatile_size = 100;

/* Function that returns a pointer based on a condition */
int* select_array(int cond, int* a, int* b) {
    return cond ? a : b;
}

/* Function expecting a pointer (type mismatch for array section) */
void expect_pointer(int* p) {
    (void)p;
}

int main(int argc, char* argv[]) {
    /* Use argc to make bounds dynamic */
    int size = (argc > 1) ? atoi(argv[1]) : g_volatile_size;
    if (size <= 0) size = 50;
    
    int offset = (argc > 2) ? atoi(argv[2]) : 10;
    int section_len = (argc > 3) ? atoi(argv[3]) : 20;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* ptr = arr1;
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
    }
    
    /* =================================================================
     * 1. Complex base expressions with operator precedence issues
     * ================================================================= */
    
    /* Base: conditional operator (low precedence) */
    #pragma omp target data map(to: (select_array(1, arr1, arr2))[offset:section_len])
    {
        /* Inside target data region */
        #pragma omp target map(tofrom: (select_array(0, arr1, arr2))[offset:section_len])
        {
            for (int i = 0; i < section_len; i++) {
                /* Use volatile to prevent optimization */
                volatile int idx = offset + i;
                select_array(1, arr1, arr2)[idx] += 1;
            }
        }
    }
    
    /* Base: pointer arithmetic */
    #pragma omp target enter data map(to: (ptr + offset)[0:section_len])
    #pragma omp target map(tofrom: (ptr + offset)[0:section_len])
    {
        for (int i = 0; i < section_len; i++) {
            (ptr + offset)[i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr + offset)[0:section_len])
    
    /* =================================================================
     * 2. Different OpenMP clauses with array sections
     * ================================================================= */
    
    /* Using 'to' and 'from' clauses separately */
    #pragma omp target data map(to: arr1[offset:section_len]) \
                            map(from: arr2[offset:section_len])
    {
        #pragma omp target
        {
            for (int i = 0; i < section_len; i++) {
                arr2[offset + i] = arr1[offset + i] + 100;
            }
        }
    }
    
    /* Using 'alloc' clause */
    int* arr3 = (int*)malloc(size * sizeof(int));
    #pragma omp target data map(alloc: arr3[5:15])
    {
        #pragma omp target
        {
            for (int i = 0; i < 15; i++) {
                arr3[5 + i] = i * 3;
            }
        }
    }
    
    /* =================================================================
     * 3. Type checking edge cases (may trigger diagnostics)
     * ================================================================= */
    
    /* Passing array section to function expecting pointer 
     * (without proper OpenMP context) - may trigger warning */
    #ifdef TRIGGER_WARNING
    expect_pointer(arr1[offset:section_len]);  /* Incorrect usage */
    #endif
    
    /* Mixed types in array section base */
    struct S {
        int* member_array;
        int x;
    } s_var;
    s_var.member_array = arr1;
    
    #pragma omp target data map(to: s_var.member_array[offset:section_len])
    {
        #pragma omp target
        {
            for (int i = 0; i < section_len; i++) {
                s_var.member_array[offset + i] += s_var.x;
            }
        }
    }
    
    /* =================================================================
     * 4. Task depend clause with array sections
     * ================================================================= */
    
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr1[0:10])
        {
            for (int i = 0; i < 10; i++) arr1[i] = 1;
        }
        
        #pragma omp task depend(in: arr1[0:10])
        {
            for (int i = 0; i < 10; i++) arr1[i] += 2;
        }
    }
    
    /* =================================================================
     * 5. Nested loops with array sections (for optimization dumps)
     * ================================================================= */
    
    #pragma omp target data map(tofrom: arr1[0:size])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 5; j++) {
                arr1[i] += j;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
        if (i < size) checksum += arr3[i];  /* Conditional to avoid bounds issue */
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
