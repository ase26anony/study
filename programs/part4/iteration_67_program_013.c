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
static int get_bound(int base, int argc, const char **argv) {
    volatile int v = base; /* volatile to prevent optimization */
    if (argc > 1) v += atoi(argv[1]) % 10;
    return v;
}

/* Function that uses array sections in a way that may provoke diagnostics */
void process_section(float *base, int start, int length, int n) {
    /* Deliberate type mixing: passing array section where pointer might be expected
     * This may trigger warnings/errors during compilation */
#pragma omp target data map(tofrom: base[start:length])
    {
        /* Complex base expression: pointer arithmetic */
#pragma omp target map(tofrom: (base + start)[0:length])
        for (int i = 0; i < length; i++) {
            (base + start)[i] += 1.0f;
        }
    }
}

/* Another function using conditional expression as base */
void process_conditional(int flag, float *a, float *b, int start, int len) {
    /* Complex base: conditional operator with lower precedence than array section */
#pragma omp target data map(tofrom: (flag ? a : b)[start:len])
    {
#pragma omp target map(tofrom: (flag ? a : b)[start:len])
        for (int i = 0; i < len; i++) {
            (flag ? a : b)[start + i] *= 2.0f;
        }
    }
}

/* Function using structure member array section */
struct Data {
    float *values;
    int count;
};

void process_struct(struct Data *d, int offset, int size) {
    /* Complex base: structure member access */
#pragma omp target enter data map(to: d->values[offset:size])
    
#pragma omp target map(tofrom: d->values[offset:size])
    for (int i = 0; i < size; i++) {
        d->values[offset + i] = i * 1.5f;
    }
    
#pragma omp target exit data map(from: d->values[offset:size])
}

/* Task dependency with array section */
void process_with_depend(float *arr, int n) {
    float *section1 = &arr[0];
    float *section2 = &arr[n/2];
    
#pragma omp task depend(out: section1[0:n/4]) shared(section1, n)
    {
#pragma omp target map(tofrom: section1[0:n/4])
        for (int i = 0; i < n/4; i++) section1[i] += 1.0f;
    }
    
#pragma omp task depend(in: section1[0:n/4]) depend(out: section2[0:n/4]) shared(section2, n)
    {
#pragma omp target map(tofrom: section2[0:n/4])
        for (int i = 0; i < n/4; i++) section2[i] = section1[i % (n/4)];
    }
    
#pragma omp taskwait
}

int main(int argc, char *argv[]) {
    const int N = 100;
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
    }
    
    /* Get bounds from command line to prevent constant folding */
    int start = get_bound(10, argc, (const char **)argv);
    int length = get_bound(20, argc, (const char **)argv);
    int offset = get_bound(5, argc, (const char **)argv);
    
    /* Ensure bounds are within array limits */
    if (start + length > N) length = N - start;
    if (offset + length > N) offset = 0;
    
    printf("Processing array sections with bounds: start=%d, length=%d, offset=%d\n",
           start, length, offset);
    
    /* Test 1: Basic array section with pointer arithmetic base */
    process_section(array1, start, length, N);
    
    /* Test 2: Conditional expression as base */
    int flag = (argc > 2) ? 1 : 0;
    process_conditional(flag, array1, array2, offset, length);
    
    /* Test 3: Structure member array section */
    struct Data data = {array1, N};
    process_struct(&data, start, length);
    
    /* Test 4: Task dependencies with array sections */
    #pragma omp parallel
    #pragma omp single
    process_with_depend(array2, N);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i];
    }
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
