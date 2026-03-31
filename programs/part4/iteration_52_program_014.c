/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer
 * for OpenMP array section nodes (OMP_ARRAY_SECTION).
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

/* Structure with array member */
struct with_array {
    int arr[N];
    int* ptr_arr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct with_array s;
    struct with_array *p = &s;
    
    /* 1. Simple array section */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* 2. Pointer dereference as base - may need parentheses */
    #pragma omp target map(ptr[0:N])
    {
        ptr[0] = 2;
    }
    
    /* 3. Structure member access as base */
    #pragma omp target map(s.arr[1:N-1])
    {
        s.arr[1] = 3;
    }
    
    /* 4. Pointer to structure member access */
    #pragma omp target map(p->arr[2:N-2])
    {
        p->arr[2] = 4;
    }
    
    /* 5. Function call returning pointer as base */
    #pragma omp target map(get_array()[0:SIZE])
    {
        get_array()[0] = 5;
    }
    
    /* 6. Cast expression as base */
    char buffer[N * sizeof(int)];
    #pragma omp target map(((int *)buffer)[0:N])
    {
        ((int *)buffer)[0] = 6;
    }
}

/* Function using array sections in depend clauses */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    int i = 5, j = 15;
    
    /* 7. Variable expressions for bounds */
    #pragma omp task depend(inout: x[i:j])
    {
        x[i] = x[i] + 1;
    }
    
    /* 8. Arithmetic expressions for bounds */
    #pragma omp task depend(in: x[0:1]) depend(out: y[start+1:len-start-1])
    {
        y[start+1] = x[0];
    }
    
    /* 9. Function calls for bounds */
    #pragma omp task depend(inout: x[compute_lower():compute_length()])
    {
        x[compute_lower()] = 7;
    }
    
    /* 10. Conditional expression for lower bound */
    int flag = 1;
    #pragma omp task depend(in: x[flag ? 0 : 10: len])
    {
        x[0] = 8;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    int idx = 5;
    
    /* 11. Linear clause with array section */
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx] += i;
    }
    
    /* 12. Multiple array sections in map clauses */
    int a[N], b[N], c[N];
    int n = N;
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Function using array section in target data directive */
void test_target_data(void) {
    int arr[N];
    
    /* 13. Target data with array section */
    #pragma omp target data map(tofrom: arr[0:N])
    {
        #pragma omp target map(alloc: arr[0:N])
        {
            arr[0] = 9;
        }
    }
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize arrays to prevent dead code elimination */
    int *dynamic_arr = (int *)malloc(N * sizeof(int));
    if (dynamic_arr == NULL) return 1;
    
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i;
    }
    
    /* Call test functions to ensure OpenMP constructs are processed */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_target_data();
    
    /* Use the results to prevent optimization */
    printf("Result: %d\n", dynamic_arr[0]);
    
    free(dynamic_arr);
    return 0;
}
