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

/* Helper functions to create complex expressions */
int *get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

/* Structure with array member */
struct with_array {
    int arr[N];
    int *ptr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct with_array s;
    struct with_array *p = &s;
    
    /* Base: simple array variable */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* Base: pointer dereference - may need parentheses */
    #pragma omp target map(ptr[0:N/2])
    {
        ptr[0] = 2;
    }
    
    /* Base: structure member access */
    #pragma omp target map(s.arr[1:5])
    {
        s.arr[1] = 3;
    }
    
    /* Base: pointer to structure member access */
    #pragma omp target map(p->arr[2:8])
    {
        p->arr[2] = 4;
    }
    
    /* Base: function call returning pointer */
    #pragma omp target map(get_array()[0:10])
    {
        get_array()[0] = 5;
    }
    
    /* Base: cast expression */
    char buffer[1000];
    #pragma omp target map(((int *)buffer)[10:20])
    {
        ((int *)buffer)[10] = 6;
    }
}

/* Function using array section in depend clause */
void test_depend_clause(void) {
    int x[N], y[N];
    int start = 10, len = 30;
    
    /* Variable expressions for bounds */
    #pragma omp task depend(in: x[0:1]) depend(out: y[start:len])
    {
        for (int i = 0; i < len; i++) {
            y[start + i] = x[0];
        }
    }
    
    /* Arithmetic expressions for bounds */
    #pragma omp task depend(inout: x[start+1:len-start-1])
    {
        for (int i = start + 1; i < len; i++) {
            x[i] *= 2;
        }
    }
    
    /* Function calls for bounds */
    #pragma omp task depend(in: x[compute_lower():compute_length()])
    {
        /* Do something */
    }
    
    /* Conditional expression for lower bound */
    int flag = 1;
    #pragma omp task depend(out: y[flag ? 0 : 10: len])
    {
        for (int i = 0; i < len; i++) {
            y[i] = i;
        }
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    int idx = 5;
    
    /* Linear clause with array section */
    #pragma omp parallel for reduction(+:sum) linear(arr[idx:1]:1)
    for (int i = 0; i < N; i++) {
        arr[idx]++;
        sum += i;
    }
    
    /* Multiple array sections in map clauses */
    int a[N], b[N], c[N];
    int n = N;
    #pragma omp target teams distribute parallel for \
                map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Target data construct with array section */
void test_target_data(void) {
    int arr[N];
    
    #pragma omp target data map(tofrom: arr[0:N])
    {
        #pragma omp target
        {
            for (int i = 0; i < N; i++) {
                arr[i] = i * 2;
            }
        }
    }
}

/* Complex nested expressions as base */
void test_complex_base(void) {
    struct with_array struct1, struct2;
    struct with_array *ptr1 = &struct1;
    struct with_array *ptr2 = &struct2;
    
    /* Chain of operations as base */
    #pragma omp target map(ptr1->ptr[0:10])
    {
        if (ptr1->ptr) ptr1->ptr[0] = 1;
    }
    
    /* Conditional base selection */
    int choice = 1;
    #pragma omp target map((choice ? ptr1 : ptr2)->arr[5:15])
    {
        (choice ? ptr1 : ptr2)->arr[5] = 2;
    }
}

int main(void) {
    /* Initialize data */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_target_data();
    test_complex_base();
    
    printf("OpenMP array section tests completed (compile with -fdump-tree-* to see pretty-printing)\n");
    return 0;
}
