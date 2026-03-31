/* This program demonstrates OpenMP array sections with complex base expressions
   to trigger the OMP_ARRAY_SECTION pretty-printing logic in GCC's tree-pretty-print.cc.
   Compile with: gcc -std=c99 -fopenmp -O1 -fdump-tree-original -fdump-tree-omplower -Wall -c tree-pretty-print-array-section.c
   Additional flags for diagnostics: -Werror=openmp-mapping -fdump-tree-all
*/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 100

/* Helper function to use in conditional expression */
int select_array(int cond, int *a, int *b) {
    return cond ? a : b;
}

/* Function with parameters to prevent constant folding */
void process_section(int *arr, int start, int length, int offset) {
    /* Complex base expression 1: conditional operator (low precedence) */
    #pragma omp target data map(to: (select_array(offset > 0, arr, arr + N/2)[start:length]))
    {
        #pragma omp target map(alloc: (select_array(offset > 0, arr, arr + N/2)[start:length]))
        {
            for (int i = 0; i < length; i++) {
                (select_array(offset > 0, arr, arr + N/2))[start + i] += i;
            }
        }
    }
}

/* Structure with array member */
struct WithArray {
    int data[N];
    int id;
};

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant propagation */
    volatile int base = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int len = (argc > 2) ? atoi(argv[2]) : 20;
    
    int array1[N], array2[N];
    struct WithArray s1 = {{0}, 1};
    int *ptr = array1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        s1.data[i] = i * 2;
    }
    
    /* Array section with pointer arithmetic (addition has lower precedence than array subscript) */
    #pragma omp target data map(tofrom: (ptr + base)[0:len])
    {
        #pragma omp target map(alloc: (ptr + base)[0:len])
        {
            for (int i = 0; i < len; i++) {
                (ptr + base)[i] *= 2;
            }
        }
    }
    
    /* Array section with structure member access */
    #pragma omp target enter data map(to: s1.data[base:len])
    #pragma omp target map(alloc: s1.data[base:len])
    {
        for (int i = 0; i < len; i++) {
            s1.data[base + i] += s1.id;
        }
    }
    #pragma omp target exit data map(from: s1.data[base:len])
    
    /* Call function with conditional base expression */
    process_section(array2, base/2, len/2, base);
    
    /* Array section in task depend clause */
    int *task_arr = array1;
    #pragma omp task depend(inout: task_arr[5:15]) shared(task_arr)
    {
        for (int i = 5; i < 20; i++) {
            task_arr[i] = -task_arr[i];
        }
    }
    
    /* Array section with type mixing to potentially trigger diagnostics */
    /* This may cause warnings with -Werror=openmp-mapping */
    float farray[N];
    #pragma omp target data map(to: farray[0:10])
    {
        #pragma omp target map(from: farray[0:10])
        {
            for (int i = 0; i < 10; i++) {
                farray[i] = i * 0.5f;
            }
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array1[i] + array2[i] + s1.data[i];
    }
    printf("Checksum: %d\n", sum);
    
    return 0;
}
