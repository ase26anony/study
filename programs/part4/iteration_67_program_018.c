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

/* Helper to prevent constant folding */
static volatile int vol_bound = 5;

/* Function using array section in OpenMP map clause with complex base */
void process_section(int *base, int offset, int start, int length, int n) {
    /* Complex base: pointer arithmetic with lower precedence than [] */
    #pragma omp target data map(to: (base + offset)[start:length])
    {
        #pragma omp target map(alloc: (base + offset)[start:length])
        for (int i = 0; i < length; i++) {
            (base + offset)[start + i] += i;
        }
    }
}

/* Another function using conditional expression as base */
void process_conditional(int *a, int *b, int cond, int lo, int hi) {
    /* Base is conditional operator (lower precedence than []) */
    #pragma omp target data map(tofrom: (cond ? a : b)[lo:hi])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < hi; i++) {
            (cond ? a : b)[lo + i] *= 2;
        }
    }
}

/* Function that may provoke type-checking diagnostics */
void problematic_usage(double *dbl_arr, int *int_arr, int n) {
    /* Mixing types in array section - may trigger warnings */
    #pragma omp target data map(to: dbl_arr[0:n])  /* OK */
    {
        /* This line, if uncommented, would cause a type error:
           #pragma omp target map(to: int_arr[0:n]) map(from: dbl_arr[0:n])
           But we keep it as a comment to show intent for manual testing.
        */
    }
}

/* Task dependency with array section */
void task_example(int *arr, int start, int len) {
    #pragma omp task depend(out: arr[start:len])
    {
        for (int i = 0; i < len; i++) arr[start + i] = i;
    }
    
    #pragma omp task depend(in: arr[start:len])
    {
        int sum = 0;
        for (int i = 0; i < len; i++) sum += arr[start + i];
        printf("Task sum: %d\n", sum);
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 100;
    int section_start = (argc > 2) ? atoi(argv[2]) : 10;
    int section_len = (argc > 3) ? atoi(argv[3]) : vol_bound;
    
    /* Dynamically allocate arrays to avoid static knowledge */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    double *dbl_arr = (double *)malloc(size * sizeof(double));
    
    if (!arr1 || !arr2 || !dbl_arr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        dbl_arr[i] = i * 0.5;
    }
    
    /* Test 1: Pointer arithmetic base with variable bounds */
    printf("Test 1: Pointer arithmetic base\n");
    process_section(arr1, 5, section_start, section_len, size);
    
    /* Test 2: Conditional operator base */
    printf("Test 2: Conditional operator base\n");
    process_conditional(arr1, arr2, (argc > 1), 0, section_len);
    
    /* Test 3: Structure member access as base */
    struct {
        int *member;
        int count;
    } s;
    s.member = arr1;
    s.count = section_len;
    
    #pragma omp target data map(to: s.member[0:s.count])
    {
        #pragma omp target
        for (int i = 0; i < s.count; i++) {
            s.member[i] += 1;
        }
    }
    
    /* Test 4: Task with depend clause using array section */
    printf("Test 4: Task with array section dependency\n");
    #pragma omp parallel
    #pragma omp single
    task_example(arr2, section_start, section_len);
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += arr1[i] + arr2[i];
    }
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(dbl_arr);
    
    return 0;
}
