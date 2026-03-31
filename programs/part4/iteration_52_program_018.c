/* test-omp-array-sections.c
 * 
 * This test program is designed to exercise GCC's pretty-printer for
 * OpenMP array sections (OMP_ARRAY_SECTION tree nodes). It contains
 * various OpenMP directives with array sections using complex base
 * expressions, varied bounds, and multiple OpenMP constructs.
 * 
 * To trigger the pretty-printer coverage:
 *   gcc -fopenmp -fdump-tree-original -O1 -c test-omp-array-sections.c
 *   gcc -fopenmp -fdump-tree-all -O2 -c test-omp-array-sections.c
 * 
 * The tree dumps will invoke the pretty-printer logic for OMP_ARRAY_SECTION.
 */

#include <stdlib.h>
#include <stdio.h>

#define N 100
#define SIZE 50

/* Helper functions for complex expressions */
int *get_array(void) {
    static int arr[SIZE];
    return arr;
}

int compute_lower(void) { return 5; }
int compute_length(void) { return 20; }

struct Container {
    int arr[N];
    int *ptr;
};

/* Function using array section in map clause with complex base */
void test_map_clause(void) {
    int arr[N];
    int *ptr = arr;
    struct Container s;
    struct Container *p = &s;
    
    /* Base: array variable */
    #pragma omp target map(arr[0:N])
    {
        arr[0] = 1;
    }
    
    /* Base: pointer dereference - may need parentheses */
    #pragma omp target map(ptr[0:N])
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
        y[start] = x[0];
    }
    
    /* Arithmetic expressions for bounds */
    #pragma omp task depend(inout: x[start+1:len-start-1])
    {
        x[start+1] *= 2;
    }
    
    /* Function calls for bounds */
    #pragma omp task depend(in: x[compute_lower():compute_length()])
    {
        x[compute_lower()] = 7;
    }
    
    /* Conditional expression for lower bound */
    int flag = 1;
    #pragma omp task depend(out: y[flag ? 0 : 10: len])
    {
        y[0] = 8;
    }
}

/* Function using array section in reduction-like context */
void test_reduction_context(void) {
    int arr[N];
    int sum = 0;
    
    /* Simulate reduction with array section (using manual reduction) */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        #pragma omp atomic
        arr[i % 10] += i;  /* Using modulo to create collisions */
    }
    
    /* Linear clause with array section */
    int idx = 0;
    #pragma omp parallel for linear(arr[idx:1]:1)
    for (int i = 0; i < 10; i++) {
        arr[idx] = i;
        idx++;
    }
}

/* Function using array section in target data directive */
void test_target_data(void) {
    int a[N], b[N], c[N];
    
    /* Multiple array sections in map clause */
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Nested array sections with different bounds */
    int *arrays[3];
    for (int i = 0; i < 3; i++) {
        arrays[i] = malloc(N * sizeof(int));
    }
    
    #pragma omp target data map(tofrom: arrays[0][0:N], arrays[1][10:20], arrays[2][N/2:N/2])
    {
        #pragma omp target
        {
            for (int i = 0; i < N; i++) {
                arrays[0][i] = i;
            }
            for (int i = 10; i < 30; i++) {
                arrays[1][i] = i * 2;
            }
            for (int i = N/2; i < N; i++) {
                arrays[2][i] = i * 3;
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        free(arrays[i]);
    }
}

/* Function with multidimensional array section simulation */
void test_multidimensional(void) {
    int matrix[10][10];
    
    /* Map a row slice */
    #pragma omp target map(matrix[2][0:10])
    {
        for (int j = 0; j < 10; j++) {
            matrix[2][j] = j;
        }
    }
    
    /* Map a column slice (requires flattening) */
    int column[10];
    for (int i = 0; i < 10; i++) {
        column[i] = matrix[i][3];
    }
    
    #pragma omp target map(tofrom: column[0:10])
    {
        for (int i = 0; i < 10; i++) {
            column[i] *= 2;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        matrix[i][3] = column[i];
    }
}

/* Main driver function */
int main(void) {
    /* Initialize data */
    test_map_clause();
    test_depend_clause();
    test_reduction_context();
    test_target_data();
    test_multidimensional();
    
    printf("OpenMP array section tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
