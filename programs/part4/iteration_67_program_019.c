/* tree-pretty-print-omp-array-section.c
 * Designed to trigger OMP_ARRAY_SECTION pretty-printing in GCC's tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-omplower -c tree-pretty-print-omp-array-section.c
 * Or for diagnostics: gcc -O0 -fopenmp -Wall -Werror=openmp-mapping -c tree-pretty-print-omp-array-section.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper function to prevent constant folding */
static int use_arg(int argc, char **argv) {
    return (argc > 1) ? atoi(argv[1]) : 10;
}

/* Structure with array member for complex base expressions */
struct Data {
    double *array;
    double member_array[100];
    int offset;
};

/* Function using array sections with complex base expressions */
void process_sections(struct Data *d1, struct Data *d2, int cond, 
                      int start, int length, int n, volatile int *bounds) {
    /* Complex base expression 1: conditional operator (low precedence) */
    #pragma omp target data map(tofrom: (cond ? d1->member_array : d2->member_array)[start:length])
    {
        /* Complex base expression 2: pointer arithmetic */
        #pragma omp target map(to: (d1->array + d1->offset)[0:n])
        {
            for (int i = 0; i < n; i++) {
                d1->array[d1->offset + i] += 1.0;
            }
        }
        
        /* Another array section with different bounds */
        #pragma omp target map(from: d2->member_array[bounds[0]:bounds[1]])
        {
            for (int i = 0; i < bounds[1]; i++) {
                d2->member_array[bounds[0] + i] = i * 2.0;
            }
        }
    }
    
    /* Array section in task depend clause */
    #pragma omp task depend(inout: d1->array[start:length])
    {
        for (int i = 0; i < length; i++) {
            d1->array[start + i] *= 2.0;
        }
    }
    
    /* Potential type mismatch to trigger diagnostics */
    double *ptr = d1->array;
    /* This may trigger warnings about array section usage outside OpenMP context */
    #pragma omp target data map(tofrom: ptr[start:length])
    {
        #pragma omp target
        for (int i = 0; i < length; i++) {
            ptr[start + i] += 3.0;
        }
    }
}

/* Function with multiple array section contexts */
void nested_sections(int *arr1, int *arr2, int m, int n, int p) {
    /* Complex expression: structure member access */
    struct Data local_data;
    local_data.array = arr1;
    local_data.offset = m;
    
    /* Array section with variable bounds from arguments */
    #pragma omp target enter data map(to: arr1[m:n])
    
    /* Mixed array sections in same directive */
    #pragma omp target map(to: arr1[m:n], arr2[p:n]) map(from: arr1[0:m])
    {
        for (int i = 0; i < n; i++) {
            arr1[m + i] = arr2[p + i] + i;
        }
        for (int i = 0; i < m; i++) {
            arr1[i] = i * 3;
        }
    }
    
    #pragma omp target exit data map(from: arr1[m:n])
}

int main(int argc, char **argv) {
    /* Use command-line arguments for dynamic bounds to prevent constant folding */
    int size = use_arg(argc, argv);
    int start = (argc > 2) ? atoi(argv[2]) : 5;
    int length = (argc > 3) ? atoi(argv[3]) : 20;
    
    /* Volatile variables to prevent optimization */
    volatile int v_start = start;
    volatile int v_length = length;
    volatile int bounds[2] = {v_start, v_length};
    
    /* Allocate and initialize arrays */
    double *array1 = (double *)malloc(size * sizeof(double));
    double *array2 = (double *)malloc(size * sizeof(double));
    int *int_array1 = (int *)malloc(size * sizeof(int));
    int *int_array2 = (int *)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        array1[i] = i * 1.5;
        array2[i] = i * 2.5;
        int_array1[i] = i;
        int_array2[i] = size - i;
    }
    
    /* Initialize structures with complex expressions */
    struct Data data1, data2;
    data1.array = array1;
    data1.offset = 3;
    data2.array = array2;
    data2.offset = 7;
    
    for (int i = 0; i < 100; i++) {
        data1.member_array[i] = i * 0.1;
        data2.member_array[i] = i * 0.2;
    }
    
    /* Call functions with array sections */
    int cond = (argc > 1) ? 1 : 0;
    process_sections(&data1, &data2, cond, v_start, v_length, size, (int *)bounds);
    
    nested_sections(int_array1, int_array2, v_start, v_length, 2);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size; i++) {
        checksum += array1[i] + array2[i] + int_array1[i] + int_array2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(int_array1);
    free(int_array2);
    
    return 0;
}
