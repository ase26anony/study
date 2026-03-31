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

/* Helper functions for complex base expressions */
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
void test_array_sections(void) {
    int arr1[N];
    int arr2[N];
    int arr3[N];
    int* ptr = arr1;
    struct Container s = {0};
    struct Container* p = &s;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    s.ptr_arr = arr2;
    
    int start = 10;
    int end = 90;
    int len = 30;
    int idx = 5;
    int flag = 1;
    
    /* 1. Simple array section in map clause */
    #pragma omp target map(tofrom: arr1[0:N])
    {
        for (int i = 0; i < N; i++) {
            arr1[i] *= 2;
        }
    }
    
    /* 2. Complex base: pointer dereference */
    #pragma omp target map(tofrom: (*ptr)[0:N/2])
    {
        for (int i = 0; i < N/2; i++) {
            (*ptr)[i] += 1;
        }
    }
    
    /* 3. Complex base: structure member access */
    #pragma omp target map(to: s.arr[1:5]) map(from: arr3[1:5])
    {
        for (int i = 1; i < 6; i++) {
            arr3[i] = s.arr[i];
        }
    }
    
    /* 4. Complex base: pointer to structure member */
    #pragma omp target map(tofrom: p->arr[2:8])
    {
        for (int i = 2; i < 10; i++) {
            p->arr[i] = i * 3;
        }
    }
    
    /* 5. Complex base: function call returning pointer */
    #pragma omp target map(tofrom: get_array()[0:N/4])
    {
        int* local_arr = get_array();
        for (int i = 0; i < N/4; i++) {
            local_arr[i] = i * 4;
        }
    }
    
    /* 6. Complex base: cast expression */
    char buffer[N * sizeof(int)];
    #pragma omp target map(tofrom: ((int *)buffer)[10:20])
    {
        int* int_buf = (int*)buffer;
        for (int i = 10; i < 30; i++) {
            int_buf[i] = i - 10;
        }
    }
    
    /* 7. Variable lower bound and length */
    #pragma omp target teams distribute parallel for \
                map(to: arr1[start:len]) map(from: arr2[start:len])
    {
        for (int i = start; i < start + len; i++) {
            arr2[i] = arr1[i] * 3;
        }
    }
    
    /* 8. Arithmetic expressions in bounds */
    #pragma omp target data map(tofrom: arr1[start+1:end-start-1])
    {
        #pragma omp target map(tofrom: arr1[start+1:end-start-1])
        {
            for (int i = start + 1; i < end - 1; i++) {
                arr1[i] += 5;
            }
        }
    }
    
    /* 9. Function calls in bounds */
    #pragma omp target map(tofrom: \
        arr1[compute_lower():compute_length()])
    {
        int lower = compute_lower();
        int length = compute_length();
        for (int i = lower; i < lower + length; i++) {
            arr1[i] -= 2;
        }
    }
    
    /* 10. Conditional expression in lower bound */
    #pragma omp task depend(inout: arr1[flag ? 0 : 10: len])
    {
        for (int i = (flag ? 0 : 10); i < (flag ? 0 : 10) + len; i++) {
            arr1[i] = 0;
        }
    }
    
    /* 11. Multiple array sections in depend clauses */
    #pragma omp task depend(in: arr1[0:1]) \
                     depend(out: arr2[1:N-1])
    {
        for (int i = 1; i < N; i++) {
            arr2[i] = arr1[0] + i;
        }
    }
    
    /* 12. Linear clause with array section (GCC extension) */
    #pragma omp parallel for linear(arr1[idx:1]:1)
    for (int i = 0; i < 10; i++) {
        arr1[idx] += i;
        idx++;
    }
    
    /* 13. Reduction with array section (requires custom reduction) */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += arr1[i];
    }
    
    /* 14. Multiple array sections in same directive */
    #pragma omp target map(to: arr1[0:n], arr2[0:n]) \
                      map(from: arr3[0:n])
    {
        int n = N/2;
        for (int i = 0; i < n; i++) {
            arr3[i] = arr1[i] + arr2[i];
        }
    }
}

/* Additional test functions to increase variety */
void test_more_complex_bases(void) {
    int matrix[10][20];
    int (*matrix_ptr)[20] = matrix;
    
    /* Array section with multi-dimensional base */
    #pragma omp target map(tofrom: matrix[0][5:10])
    {
        for (int i = 0; i < 10; i++) {
            for (int j = 5; j < 15; j++) {
                matrix[i][j] = i * j;
            }
        }
    }
    
    /* Complex pointer expression as base */
    #pragma omp target map(tofrom: (*(matrix_ptr + 2))[3:8])
    {
        for (int j = 3; j < 11; j++) {
            matrix[2][j] *= 2;
        }
    }
}

/* Main function to drive the tests */
int main(void) {
    printf("Testing OpenMP array sections for GCC pretty-printer coverage\n");
    
    test_array_sections();
    test_more_complex_bases();
    
    printf("Tests completed (compile with -fopenmp -fdump-tree-* for coverage)\n");
    return 0;
}
