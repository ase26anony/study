/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748, which handle the formatting of OpenMP array sections.
 *
 * The code uses complex base expressions (conditional operator, pointer
 * arithmetic, structure member access) with variable bounds to ensure
 * the op_prio() checks and non-constant bound handling are exercised.
 * Array sections are used in multiple OpenMP contexts (map, to, from,
 * depend) to increase the likelihood of the pretty-printer being invoked
 * during compilation dumps or diagnostics.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c test-omp-array-section.c
 * Or for diagnostics: gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c test-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Structure containing an array member to test member access as base expression */
struct with_array {
    int data[100];
    int *ptr;
};

/* Function that returns one of two arrays based on a condition */
int *select_array(int cond, int *a, int *b) {
    return cond ? a : b;
}

/* Function using array section in map clause with complex base expression */
void target_computation(struct with_array *s, int *ptr1, int *ptr2, 
                        int start, int length, int n, int cond) {
    /* Use a conditional expression as base for array section.
     * This will test op_prio() comparison: (cond ? ptr1 : ptr2)[start:length]
     * The conditional operator has lower precedence than array subscripting,
     * so parentheses may be needed in the printed output.
     */
    #pragma omp target map(tofrom: (cond ? ptr1 : ptr2)[start:length])
    {
        for (int i = 0; i < length; i++) {
            (cond ? ptr1 : ptr2)[start + i] *= 2;
        }
    }

    /* Use pointer arithmetic as base: (ptr1 + offset)[0:length] */
    int offset = start / 2;
    #pragma omp target map(tofrom: (ptr1 + offset)[0:length])
    {
        for (int i = 0; i < length; i++) {
            (ptr1 + offset)[i] += i;
        }
    }

    /* Use structure member access as base: s->data[start:length] */
    #pragma omp target map(tofrom: s->data[start:length])
    {
        for (int i = 0; i < length; i++) {
            s->data[start + i] = s->data[start + i] * 3 + 1;
        }
    }
}

/* Another function to create more OMP_ARRAY_SECTION nodes in different contexts */
void more_array_sections(int *arr, int *brr, int m, int n, int flag) {
    /* Use array sections in to/from clauses */
    #pragma omp target enter data map(to: arr[0:m])
    #pragma omp target enter data map(to: brr[m/2:n])
    
    /* Use in task depend clause - though array sections in depend are
     * OpenMP 5.0+, we include it to potentially trigger different parsing paths */
    #pragma omp task depend(inout: arr[0:m]) shared(arr)
    {
        for (int i = 0; i < m; i++) arr[i] = i;
    }
    
    #pragma omp target map(from: brr[m/2:n])
    {
        for (int i = 0; i < n; i++) brr[m/2 + i] = arr[i % m];
    }
    
    #pragma omp target exit data map(from: arr[0:m])
    #pragma omp target exit data map(from: brr[m/2:n])
}

/* Function that might provoke type-related diagnostics */
void problematic_usage(double *dbl_arr, int *int_arr, int n) {
    /* This may trigger warnings about type mismatches when array sections
     * are used in non-OpenMP contexts or with wrong types */
    volatile int start = 0;
    volatile int len = n;
    
    /* Array section with volatile bounds to prevent constant folding */
    #pragma omp target map(tofrom: dbl_arr[start:len])
    {
        for (int i = 0; i < len; i++) {
            dbl_arr[start + i] = dbl_arr[start + i] * 2.0;
        }
    }
    
    /* Attempt to use array section in a potentially problematic way */
    /* This might trigger diagnostics during compilation */
    int *ptr = int_arr;
    #pragma omp target is_device_ptr(ptr) map(tofrom: ptr[0:n/2])
    {
        for (int i = 0; i < n/2; i++) {
            ptr[i] = ptr[i] + 1;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to get variable sizes, preventing constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 50;
    int section_start = (argc > 2) ? atoi(argv[2]) : 10;
    int section_len = (argc > 3) ? atoi(argv[3]) : 20;
    
    if (size < section_start + section_len) {
        size = section_start + section_len + 10;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    double *dbl_arr = (double *)malloc(size * sizeof(double));
    
    struct with_array s;
    s.ptr = arr2;
    
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        dbl_arr[i] = i * 0.5;
        if (i < 100) s.data[i] = i * 2;
    }
    
    /* Use volatile variables for bounds to ensure they remain as expressions */
    volatile int vstart = section_start;
    volatile int vlen = section_len;
    
    /* Test 1: Complex base expressions with variable bounds */
    printf("Starting target computations with array sections...\n");
    target_computation(&s, arr1, arr2, vstart, vlen, size, argc > 1);
    
    /* Test 2: More array sections in different OpenMP contexts */
    more_array_sections(arr1, arr2, size/2, size/4, 1);
    
    /* Test 3: Potentially problematic usage that might trigger diagnostics */
    problematic_usage(dbl_arr, arr1, size);
    
    /* Verify results to prevent dead code elimination */
    int checksum = 0;
    double dbl_checksum = 0.0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
        dbl_checksum += dbl_arr[i];
        if (i < 100) checksum += s.data[i];
    }
    
    printf("Checksums: int=%d, double=%.2f\n", checksum, dbl_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dbl_arr);
    
    return 0;
}
