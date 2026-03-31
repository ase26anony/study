/* test-omp-array-section.c
 * 
 * This program demonstrates OpenMP array sections with complex base expressions
 * to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
 * The array sections are used in various OpenMP directives with variable bounds
 * and base expressions involving operators of different precedence levels.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Helper function to compute checksum */
int checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function that uses array sections with complex base expressions */
void process_sections(int *arr1, int *arr2, int start, int length, int offset, int cond) {
    /* Use volatile variables to prevent constant folding */
    volatile int v_start = start;
    volatile int v_len = length;
    volatile int v_offset = offset;
    volatile int v_cond = cond;
    
    /* Array section with conditional base expression (low precedence operator) */
    /* This triggers op_prio checks: (cond ? arr1 : arr2) has lower precedence than [] */
    #pragma omp target data map(tofrom: (v_cond ? arr1 : arr2)[v_start:v_len])
    {
        #pragma omp target map(tofrom: (v_cond ? arr1 : arr2)[v_start:v_len])
        {
            for (int i = 0; i < v_len; i++) {
                (v_cond ? arr1 : arr2)[v_start + i] += i;
            }
        }
    }
    
    /* Array section with pointer arithmetic base (medium precedence) */
    /* (arr1 + offset) has lower precedence than [] */
    #pragma omp target data map(to: (arr1 + v_offset)[0:v_len])
    {
        #pragma omp target map(to: (arr1 + v_offset)[0:v_len])
        {
            for (int i = 0; i < v_len; i++) {
                (arr1 + v_offset)[i] *= 2;
            }
        }
    }
}

/* Structure with array member for member access testing */
struct Data {
    int header;
    int values[N];
    int footer;
};

void process_struct_sections(struct Data *d, int start, int length) {
    volatile int v_start = start;
    volatile int v_len = length;
    
    /* Array section on structure member access */
    /* d->values has lower precedence than [] */
    #pragma omp target data map(tofrom: d->values[v_start:v_len])
    {
        #pragma omp target map(tofrom: d->values[v_start:v_len])
        {
            for (int i = 0; i < v_len; i++) {
                d->values[v_start + i] += 3;
            }
        }
    }
    
    /* More complex: pointer to structure member with offset */
    #pragma omp target data map(to: (&(d->values[10]))[0:v_len])
    {
        #pragma omp target map(to: (&(d->values[10]))[0:v_len])
        {
            for (int i = 0; i < v_len; i++) {
                (&(d->values[10]))[i] -= 1;
            }
        }
    }
}

/* Function that deliberately creates type warnings with array sections */
void problematic_usage(int *arr, int n) {
    volatile int v_n = n;
    
    /* This may trigger diagnostics: passing array section where pointer expected */
    /* Note: This is deliberately questionable usage to trigger diagnostics */
    #pragma omp target data map(to: arr[0:v_n])
    {
        /* Empty - just for the map clause */
    }
    
    /* Array section in depend clause for tasks */
    #pragma omp task depend(inout: arr[0:v_n])
    {
        for (int i = 0; i < v_n; i++) {
            arr[i] = i;
        }
    }
    #pragma omp taskwait
}

int main(int argc, char *argv[]) {
    /* Use argc to get variable bounds, preventing constant propagation */
    int start = (argc > 1) ? atoi(argv[1]) % 50 : 10;
    int length = (argc > 2) ? atoi(argv[2]) % 30 : 20;
    int offset = (argc > 3) ? atoi(argv[3]) % 20 : 5;
    int cond = (argc > 4) ? atoi(argv[4]) % 2 : 1;
    
    /* Initialize arrays */
    int arr1[N], arr2[N];
    struct Data data;
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        data.values[i] = i * 2;
    }
    data.header = 0xDEAD;
    data.footer = 0xBEEF;
    
    printf("Initial checksum arr1: %d\n", checksum(arr1, N));
    printf("Initial checksum arr2: %d\n", checksum(arr2, N));
    printf("Initial checksum data.values: %d\n", checksum(data.values, N));
    
    /* Process with array sections using complex base expressions */
    process_sections(arr1, arr2, start, length, offset, cond);
    
    /* Process structure with array sections */
    process_struct_sections(&data, start, length);
    
    /* Deliberate problematic usage to potentially trigger diagnostics */
    problematic_usage(arr1, length);
    
    printf("Final checksum arr1: %d\n", checksum(arr1, N));
    printf("Final checksum arr2: %d\n", checksum(arr2, N));
    printf("Final checksum data.values: %d\n", checksum(data.values, N));
    
    /* Verify with a simple test */
    int test_sum = 0;
    #pragma omp target map(tofrom: test_sum) map(to: arr1[0:10])
    {
        for (int i = 0; i < 10; i++) {
            test_sum += arr1[i];
        }
    }
    printf("Test sum of first 10 elements: %d\n", test_sum);
    
    return 0;
}
