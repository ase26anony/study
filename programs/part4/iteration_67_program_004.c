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
static int get_bound(int base, int offset) {
    volatile int v = base; /* volatile to prevent optimization */
    return v + offset;
}

/* Function that uses array sections in multiple OpenMP contexts */
void process_with_sections(float* arr1, float* arr2, int n, int start, int len, int cond) {
    /* Complex base expression 1: conditional operator as base */
    #pragma omp target data map(to: (cond ? arr1 : arr2)[start:len])
    {
        /* Inside target data region, launch a kernel */
        #pragma omp target map(alloc: (cond ? arr1 : arr2)[start:len])
        {
            for (int i = 0; i < len; i++) {
                float* base = cond ? arr1 : arr2;
                base[start + i] += 1.0f;
            }
        }
    }

    /* Complex base expression 2: pointer arithmetic as base */
    float* ptr = cond ? arr1 : arr2;
    int offset = start / 2;
    #pragma omp target enter data map(to: (ptr + offset)[0:len/2])
    #pragma omp target map(tofrom: (ptr + offset)[0:len/2])
    {
        for (int i = 0; i < len/2; i++) {
            ptr[offset + i] *= 2.0f;
        }
    }
    #pragma omp target exit data map(from: (ptr + offset)[0:len/2])

    /* Array section in depend clause for task */
    #pragma omp task depend(inout: arr1[start:len])
    {
        for (int i = 0; i < len; i++) {
            arr1[start + i] -= 0.5f;
        }
    }
    #pragma omp taskwait
}

/* Another function with different base expression: struct member access */
struct Data {
    float* values;
    int count;
};

void process_struct_sections(struct Data* d, int start, int len) {
    /* Complex base: struct member access */
    #pragma omp target data map(tofrom: d->values[start:len])
    {
        #pragma omp target map(alloc: d->values[start:len])
        {
            for (int i = 0; i < len; i++) {
                d->values[start + i] = i * 1.5f;
            }
        }
    }
}

/* Function that deliberately causes type checking issues */
void problematic_usage(float* arr, int n) {
    /* This may trigger diagnostics about array section usage */
    /* Attempt to pass array section to non-OpenMP function (ill-formed) */
    /* The compiler may emit a warning/error during parsing */
    extern void expect_pointer(float*);
    /* Uncommenting the next line may provoke a diagnostic: */
    /* expect_pointer(arr[0:n]); */
}

int main(int argc, char* argv[]) {
    /* Use command-line arguments to prevent constant propagation */
    int base_size = (argc > 1) ? atoi(argv[1]) : 100;
    int section_start = (argc > 2) ? atoi(argv[2]) : 10;
    int section_len = (argc > 3) ? atoi(argv[3]) : 20;
    int cond = (argc > 4) ? atoi(argv[4]) : 1;

    /* Introduce volatility to prevent constant folding */
    volatile int v_size = base_size;
    int n = get_bound(v_size, 0);
    
    /* Allocate arrays */
    float* arr1 = (float*)malloc(n * sizeof(float));
    float* arr2 = (float*)malloc(n * sizeof(float));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = (float)i;
        arr2[i] = (float)(n - i);
    }

    /* Process with complex array sections */
    process_with_sections(arr1, arr2, n, section_start, section_len, cond);

    /* Process with struct member array section */
    struct Data d = {arr1, n};
    process_struct_sections(&d, section_start/2, section_len/2);

    /* Compute checksum to prevent dead code elimination */
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr1[i] + arr2[i];
    }
    printf("Checksum: %f\n", sum);

    /* Clean up */
    free(arr1);
    free(arr2);

    return 0;
}
