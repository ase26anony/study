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
static int get_bound(int base, int argc, char **argv) {
    volatile int v = base; /* volatile to prevent optimization */
    if (argc > 1) v += atoi(argv[1]); /* use command line input */
    return v;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_sections(int *arr1, int *arr2, int n, int offset, int len, int cond) {
    /* 1. Complex base expression: conditional operator with lower precedence */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[offset:len])
    {
        /* Inside data region: use array section in target computation */
        #pragma omp target map(alloc: (cond ? arr1 : arr2)[offset:len])
        {
            for (int i = 0; i < len; i++) {
                (cond ? arr1 : arr2)[offset + i] += i;
            }
        }
    }

    /* 2. Complex base: pointer arithmetic (ptr + offset)[0:len] */
    int *ptr = arr1;
    #pragma omp target enter data map(to: (ptr + offset)[0:len])
    #pragma omp target map(alloc: (ptr + offset)[0:len])
    {
        for (int i = 0; i < len; i++) {
            (ptr + offset)[i] *= 2;
        }
    }
    #pragma omp target exit data map(from: (ptr + offset)[0:len])

    /* 3. Another variant: base with addition inside parentheses */
    #pragma omp target data map(tofrom: (arr1 + (offset > 0 ? offset : 0))[0:len])
    {
        #pragma omp target
        for (int i = 0; i < len; i++) {
            (arr1 + (offset > 0 ? offset : 0))[i] += 1;
        }
    }
}

/* Struct with array member to test member access as base */
struct WithArray {
    int data[100];
    int *ptr;
};

void process_struct_section(struct WithArray *s, int start, int length) {
    /* 4. Complex base: struct member access */
    #pragma omp target data map(tofrom: s->data[start:length])
    {
        #pragma omp target
        for (int i = 0; i < length; i++) {
            s->data[start + i] = i * i;
        }
    }

    /* 5. Even more complex: conditional struct member */
    #pragma omp target data map(to: (s->ptr ? s->ptr : s->data)[0:length])
    {
        #pragma omp target
        for (int i = 0; i < length; i++) {
            (s->ptr ? s->ptr : s->data)[i] = -i;
        }
    }
}

/* Function that may cause type warnings/errors with array sections */
void problematic_usage(int *arr, int n) {
    /* This may trigger diagnostics because array section is used
     * outside of explicit OpenMP data clause (though inside a target region) */
    #pragma omp target map(tofrom: arr[0:n])
    {
        /* Using array section in a pointer context - might cause warnings */
        int *section_start = &arr[0]; /* Not the array section itself, but related */
        for (int i = 0; i < n; i++) {
            section_start[i] += arr[i];
        }
    }
}

/* Task dependency with array sections */
void task_example(int *arr, int n, int chunk) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr[0:chunk])
        {
            for (int i = 0; i < chunk; i++) arr[i] = 1;
        }
        
        #pragma omp task depend(in: arr[0:chunk]) depend(out: arr[chunk:n-chunk])
        {
            for (int i = chunk; i < n; i++) arr[i] = 2;
        }
        
        #pragma omp task depend(in: arr[chunk:n-chunk])
        {
            int sum = 0;
            for (int i = 0; i < n; i++) sum += arr[i];
            printf("Task sum: %d\n", sum);
        }
    }
}

int main(int argc, char **argv) {
    const int N = 100;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    struct WithArray s = {{0}, NULL};
    s.ptr = (int *)malloc(N * sizeof(int));

    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        s.data[i] = i * 2;
        if (s.ptr) s.ptr[i] = i * 3;
    }

    /* Get bounds from command line to prevent constant folding */
    int offset = get_bound(10, argc, argv);
    int length = get_bound(20, argc, argv);
    int cond = (argc > 2) ? atoi(argv[2]) : 1;

    /* Process with complex array section bases */
    process_sections(array1, array2, N, offset, length, cond);
    
    /* Process struct with array sections */
    process_struct_section(&s, offset, length);
    
    /* Potential problematic usage (may trigger diagnostics) */
    problematic_usage(array1, N);
    
    /* Task with array section dependencies */
    task_example(array2, N, length);

    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i] + s.data[i];
        if (s.ptr) checksum += s.ptr[i];
    }
    printf("Checksum: %d\n", checksum);

    /* Cleanup */
    free(array1);
    free(array2);
    if (s.ptr) free(s.ptr);

    return 0;
}
