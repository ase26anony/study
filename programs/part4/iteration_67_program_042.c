/* test-omp-array-section.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc, specifically lines 2736-2748.
 * It uses OpenMP array sections with complex base expressions and variable
 * bounds to ensure the uncovered code block is executed during compilation
 * with appropriate dump or diagnostic flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to introduce variability and prevent constant folding */
static int get_bound(int base, int offset) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + offset;
}

/* Function that uses array sections in a way that may cause type warnings */
void process_section(float *section, int len) {
    for (int i = 0; i < len; i++) {
        section[i] += 1.0f;
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to make bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    int start = (argc > 2) ? atoi(argv[2]) : 10;
    int len = (argc > 3) ? atoi(argv[3]) : 20;
    
    if (n < start + len) n = start + len;
    
    /* Allocate and initialize arrays */
    float *arr1 = (float *)malloc(n * sizeof(float));
    float *arr2 = (float *)malloc(n * sizeof(float));
    float *arr3 = (float *)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(i * 2);
        arr3[i] = (float)(i * 3);
    }
    
    /* STRUCTURE WITH ARRAY MEMBER - for complex base expression */
    struct WithArray {
        float *member_array;
        int offset;
    };
    
    struct WithArray s1 = {arr1, 5};
    struct WithArray s2 = {arr2, 10};
    
    /* VARIABLE BOUNDS from volatile/function to prevent constant folding */
    volatile int vstart = start;
    int lower = get_bound(vstart, 2);   /* Complex lower bound expression */
    int length = get_bound(len, -3);    /* Complex length expression */
    
    /* OPENMP TARGET DATA REGION with array sections having complex bases */
    
    /* 1. Array section with conditional base expression (triggers op_prio check) */
    #pragma omp target data map(tofrom: ((argc > 4) ? s1.member_array : s2.member_array)[lower:length])
    {
        /* 2. Array section with pointer arithmetic base (another op_prio case) */
        #pragma omp target map(tofrom: (arr3 + s1.offset)[0:length])
        {
            #pragma omp parallel for
            for (int i = 0; i < length; i++) {
                float *base = (argc > 4) ? s1.member_array : s2.member_array;
                base[lower + i] += 1.5f;
                arr3[s1.offset + i] += 2.5f;
            }
        }
        
        /* 3. Different clauses with array sections */
        #pragma omp target enter data map(alloc: arr1[lower:length/2])
        #pragma omp target exit data map(from: arr1[lower:length/2])
    }
    
    /* 4. TASK WITH DEPEND clause using array section */
    #pragma omp task depend(inout: arr2[start:len]) shared(arr2)
    {
        for (int i = start; i < start + len; i++) {
            arr2[i] *= 2.0f;
        }
    }
    
    /* 5. POTENTIAL TYPE WARNING: Passing array section to function 
     * (without proper OpenMP context) - may trigger diagnostic formatting */
    if (argc > 5) {
        /* This may generate a warning about passing array section to function */
        process_section(arr1 + start, len);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
