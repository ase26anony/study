/* test-omp-array-sections.c
 * 
 * This program is designed to trigger the OMP_ARRAY_SECTION pretty-printing
 * logic in GCC's tree-pretty-print.cc (lines 2736-2748).
 * 
 * Compile with: gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions to create complex base expressions */
int* get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int* ptr_arr;
};

/* Function using various OpenMP array sections */
void test_array_sections(int *arr, int n, struct Container *s, int **ptr) {
    int i, j, start, end, flag, len, idx;
    int sum = 0;
    
    start = 10;
    end = 90;
    flag = 1;
    len = 30;
    idx = 0;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr[0:n])
    {
        for (i = 0; i < n; i++) {
            arr[i] *= 2;
        }
    }
    
    /* 2. Array section with complex base: structure member */
    #pragma omp target data map(tofrom: s->arr[0:N])
    {
        #pragma omp target teams distribute parallel for map(to: s->arr[0:N])
        for (i = 0; i < N; i++) {
            s->arr[i] = i;
        }
    }
    
    /* 3. Array section with complex base: pointer dereference */
    #pragma omp task depend(inout: (*ptr)[0:n])
    {
        for (i = 0; i < n; i++) {
            (*ptr)[i] = i * 3;
        }
    }
    
    /* 4. Array section with complex base: function call */
    #pragma omp target map(tofrom: get_array()[0:SIZE])
    {
        int *tmp = get_array();
        for (i = 0; i < SIZE; i++) {
            tmp[i] += i;
        }
    }
    
    /* 5. Array section with complex base: cast expression */
    char buffer[1000];
    #pragma omp task depend(in: (int *)buffer)[0:100/sizeof(int)])
    {
        int *int_buf = (int *)buffer;
        for (i = 0; i < 100/sizeof(int); i++) {
            int_buf[i] = i;
        }
    }
    
    /* 6. Array section with variable lower bound and length */
    #pragma omp task depend(in: arr[start:end-start])
    {
        for (i = start; i < end; i++) {
            arr[i] += 1;
        }
    }
    
    /* 7. Array section with arithmetic expressions */
    #pragma omp target teams distribute parallel for \
                map(to: arr[start+1:end-start-1])
    for (i = start+1; i < end-1; i++) {
        arr[i] *= arr[i-1];
    }
    
    /* 8. Array section with function call expressions */
    #pragma omp task depend(out: arr[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (i = lower; i < lower + length; i++) {
            arr[i] = -arr[i];
        }
    }
    
    /* 9. Array section with conditional (ternary) expression */
    #pragma omp target data map(tofrom: arr[flag ? 0 : 10: len])
    {
        #pragma omp target teams distribute parallel for \
                    map(to: arr[flag ? 0 : 10: len])
        for (i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++) {
            arr[i] += 2;
        }
    }
    
    /* 10. Multiple array sections in same directive */
    int a[N], b[N], c[N];
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* 11. Array section in depend clause with multiple dependencies */
    int x[10], y[100];
    #pragma omp task depend(in: x[0:1]) depend(out: y[1:SIZE-1])
    {
        y[1] = x[0] * 2;
        for (i = 2; i < SIZE-1; i++) {
            y[i] = y[i-1] + 1;
        }
    }
    
    /* 12. Array section in linear clause (requires valid OpenMP context) */
    #pragma omp parallel for reduction(+:sum) private(i)
    for (i = 0; i < n; i++) {
        /* Simulating linear clause with array section */
        sum += arr[i];
        /* Note: Actual linear clause with array sections may have restrictions */
    }
    
    /* 13. Nested complex base expressions */
    struct Container container;
    container.ptr_arr = arr;
    #pragma omp target map(tofrom: container.ptr_arr[5:20])
    {
        for (i = 5; i < 25; i++) {
            container.ptr_arr[i] = i * i;
        }
    }
}

/* Main function to drive the tests */
int main(void) {
    int *arr = (int *)malloc(N * sizeof(int));
    int *ptr_arr = (int *)malloc(N * sizeof(int));
    struct Container s;
    int *dynamic_arr = (int *)malloc(SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
        ptr_arr[i] = i * 2;
        s.arr[i] = i * 3;
    }
    
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 4;
    }
    
    s.ptr_arr = ptr_arr;
    
    /* Call function with various OpenMP array sections */
    test_array_sections(arr, N, &s, &dynamic_arr);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i] + s.arr[i] + ptr_arr[i];
    }
    for (int i = 0; i < SIZE; i++) {
        sum += dynamic_arr[i];
    }
    
    printf("Result checksum: %d\n", sum);
    
    /* Cleanup */
    free(arr);
    free(ptr_arr);
    free(dynamic_arr);
    
    return 0;
}
