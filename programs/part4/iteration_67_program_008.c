/* test-omp-array-section.c
 * 
 * This program is designed to trigger the pretty-printing logic for
 * OMP_ARRAY_SECTION nodes in GCC's tree-pretty-print.cc, specifically
 * lines 2736-2748. It uses OpenMP array sections with complex base
 * expressions and variable bounds to ensure the uncovered code block
 * is executed during compilation with appropriate dump flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int argc, const char **argv) {
    volatile int v = base; /* volatile to prevent optimization */
    if (argc > 1) v += atoi(argv[1]) % 5;
    return v;
}

/* Function that uses array sections in multiple contexts */
void process_sections(int n, int start, int len, int *arr1, int *arr2, int cond) {
    /* Complex base expression: conditional operator with lower precedence */
    int *base_ptr = cond ? arr1 : arr2;
    
    /* 1. Array section in target data map clause with complex base */
    #pragma omp target data map(tofrom: (cond ? arr1 : arr2)[start:len])
    {
        /* 2. Another array section inside target region */
        #pragma omp target map(tofrom: (base_ptr + start)[0:len])
        {
            for (int i = 0; i < len; i++) {
                base_ptr[start + i] += i;
            }
        }
        
        /* 3. Use array section with pointer arithmetic base */
        int offset = start / 2;
        #pragma omp target map(tofrom: (arr1 + offset)[0:len/2])
        {
            for (int i = 0; i < len/2; i++) {
                arr1[offset + i] *= 2;
            }
        }
    }
    
    /* 4. Array section in task depend clause (OpenMP 4.5+) */
    #pragma omp task depend(inout: arr2[start:len])
    {
        for (int i = 0; i < len; i++) {
            arr2[start + i] -= 1;
        }
    }
    
    #pragma omp taskwait
}

/* Structure with array member for member access base expression */
struct WithArray {
    int header;
    int data[100];
};

int main(int argc, char *argv[]) {
    const int total_size = 100;
    int *array1 = (int *)malloc(total_size * sizeof(int));
    int *array2 = (int *)malloc(total_size * sizeof(int));
    struct WithArray s1 = {0, {0}};
    
    /* Initialize arrays */
    for (int i = 0; i < total_size; i++) {
        array1[i] = i;
        array2[i] = total_size - i;
        if (i < 100) s1.data[i] = i * 2;
    }
    
    /* Get bounds from command line to prevent constant folding */
    int start = get_bound(10, argc, (const char **)argv);
    int length = get_bound(20, argc, (const char **)argv);
    int cond = (argc > 2) ? 1 : 0;
    
    /* Process with various array section patterns */
    process_sections(total_size, start, length, array1, array2, cond);
    
    /* 5. Array section with structure member access as base */
    #pragma omp target data map(tofrom: s1.data[start:length/2])
    {
        #pragma omp target
        {
            for (int i = 0; i < length/2; i++) {
                s1.data[start + i] += 100;
            }
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < total_size; i++) {
        checksum += array1[i] + array2[i];
        if (i < 100) checksum += s1.data[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
