/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections
 * with proper parentheses for complex base expressions.
 *
 * Compilation flags to generate tree dumps and diagnostics:
 *   gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 *   gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 *   gcc -O2 -fopenmp -foffload=disable -fdump-tree-optimized -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to compute a simple checksum */
static int checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i];
    return sum;
}

/* Function that uses array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int *ptr, int offset,
                      int start, int length, int n, int cond) {
    /* Use volatile to prevent constant folding of bounds */
    volatile int v_start = start;
    volatile int v_len = length;
    
    /* 1. Array section with conditional base expression.
     *    This triggers op_prio checks because the conditional operator
     *    has lower precedence than the array section subscript.
     */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[v_start:v_len])
    {
        #pragma omp target map(tofrom: (cond ? arr1 : arr2)[v_start:v_len])
        {
            for (int i = 0; i < v_len; i++) {
                (cond ? arr1 : arr2)[v_start + i] += i;
            }
        }
    }
    
    /* 2. Array section with pointer arithmetic base.
     *    The addition has lower precedence than array subscript,
     *    requiring parentheses in the printed representation.
     */
    #pragma omp target enter data map(to: (ptr + offset)[0:n])
    #pragma omp target map(tofrom: (ptr + offset)[0:n])
    {
        for (int i = 0; i < n; i++) {
            (ptr + offset)[i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr + offset)[0:n])
    
    /* 3. Array section in a depend clause (OpenMP tasks).
     *    Creates additional OMP_ARRAY_SECTION nodes in different contexts.
     */
    #pragma omp task depend(inout: arr1[start:length])
    {
        for (int i = 0; i < length; i++)
            arr1[start + i] += 1;
    }
    
    /* 4. Mixed array sections in a single directive.
     *    Uses different clauses and complex bases.
     */
    #pragma omp target data map(to: arr1[0:n]) map(from: (ptr + 1)[0:n/2])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < n; i++) {
            arr1[i] = i;
            if (i < n/2)
                (ptr + 1)[i] = arr1[i] * 2;
        }
    }
}

/* Struct with array member to test member access as base */
struct Container {
    int header;
    int data[100];
    int *ptr;
};

void process_struct_section(struct Container *c1, struct Container *c2,
                            int start, int len, int cond) {
    /* 5. Array section with structure member access as base.
     *    The member access '.' has higher precedence than '?',
     *    but lower than array subscript, creating interesting precedence cases.
     */
    #pragma omp target data map(tofrom: (cond ? c1->data : c2->data)[start:len])
    {
        #pragma omp target map(tofrom: (cond ? c1->data : c2->data)[start:len])
        {
            for (int i = 0; i < len; i++) {
                (cond ? c1->data : c2->data)[start + i] += 3;
            }
        }
    }
    
    /* 6. Nested complex base with multiple operators */
    #pragma omp target map(tofrom: (c1->ptr + start)[0:len])
    {
        for (int i = 0; i < len; i++) {
            (c1->ptr + start)[i] = i * 4;
        }
    }
}

/* Function that may cause type warnings/errors with array sections */
void problematic_usage(int *arr, int n) {
    /* This may trigger diagnostics because array sections are used
     * outside of OpenMP data clauses, potentially causing type mismatches.
     */
    int *section = (int *)&arr[0:n];  /* Cast to suppress error for demonstration */
    (void)section;
    
    /* Array section in a non-OpenMP context that might be diagnosed */
    #pragma omp parallel
    {
        /* Using array section syntax incorrectly may trigger warnings */
        int local = arr[0:n/2][0];  /* Invalid but parsed */
        (void)local;
    }
}

int main(int argc, char **argv) {
    /* Use command-line arguments for dynamic bounds to prevent constant folding */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int len = (argc > 3) ? atoi(argv[3]) : 20;
    int offset = (argc > 4) ? atoi(argv[4]) : 5;
    int cond = (argc > 5) ? atoi(argv[5]) : 1;
    
    if (size < 1) size = 100;
    if (start < 0) start = 10;
    if (len < 1) len = 20;
    if (offset < 0) offset = 5;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *ptr_arr = (int *)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        ptr_arr[i] = i * 2;
    }
    
    /* Initialize struct with array member */
    struct Container c1, c2;
    c1.header = 1;
    c2.header = 2;
    c1.ptr = (int *)malloc(size * sizeof(int));
    c2.ptr = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < 100; i++) {
        c1.data[i] = i * 3;
        c2.data[i] = i * 4;
    }
    for (int i = 0; i < size; i++) {
        c1.ptr[i] = i * 5;
        c2.ptr[i] = i * 6;
    }
    
    /* Process array sections with complex base expressions */
    process_sections(arr1, arr2, ptr_arr, offset, start, len, size, cond);
    
    /* Process struct-based array sections */
    process_struct_section(&c1, &c2, start, len, cond);
    
    /* Potentially problematic usage that might trigger diagnostics */
    problematic_usage(arr1, size);
    
    /* Compute and print checksums to ensure execution and prevent dead code elimination */
    int sum1 = checksum(arr1, size);
    int sum2 = checksum(arr2, size);
    int sum3 = checksum(ptr_arr, size);
    int sum4 = checksum(c1.data, 100);
    
    printf("Checksums: arr1=%d, arr2=%d, ptr_arr=%d, c1.data=%d\n",
           sum1, sum2, sum3, sum4);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(ptr_arr);
    free(c1.ptr);
    free(c2.ptr);
    
    return 0;
}
