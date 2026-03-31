/* omp_array_section_test.c
 * Tests GCC's OMP_ARRAY_SECTION pretty-printing
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -c omp_array_section_test.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays - different base types */
int global_arr[SIZE];
float global_float_arr[SIZE];
double *global_ptr;

/* Prevent optimization and ensure all functions are compiled */
volatile int selector = 0;

/* Function 1: Array section with simple VAR_DECL base */
__attribute__((noinline, used))
void func_map_simple(void)
{
    int local_arr[SIZE];
    
    /* Initialize local array */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i * 2;
    }
    
    /* OMP_ARRAY_SECTION with simple array base */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside the region */
        for (int i = 10; i < 110; i++) {
            local_arr[i] += 1;
        }
    }
    
    /* Another section with different bounds */
    #pragma omp target data map(global_arr[0:SIZE])
    {
        for (int i = 0; i < SIZE; i++) {
            global_arr[i] = local_arr[i];
        }
    }
}

/* Function 2: Array section with ARRAY_REF base (more complex) */
__attribute__((noinline, used))
void func_depend_complex(void)
{
    int n = 5;
    int size = SIZE / 2;
    
    /* OMP_ARRAY_SECTION with variable bounds */
    #pragma omp task depend(inout: global_arr[n:size])
    {
        for (int i = n; i < n + size; i++) {
            global_arr[i] *= 2;
        }
    }
    
    /* Section with arithmetic in bounds */
    int offset = 20;
    #pragma omp task depend(inout: global_float_arr[offset*2:CHUNK])
    {
        for (int i = 40; i < 40 + CHUNK; i++) {
            global_float_arr[i] = i * 1.5f;
        }
    }
    
    #pragma omp taskwait
}

/* Function 3: Array section with pointer base */
__attribute__((noinline, used))
void func_update_pointer(void)
{
    int len = SIZE - 10;
    double local_buf[SIZE];
    
    global_ptr = local_buf;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        local_buf[i] = i * 3.14;
    }
    
    /* OMP_ARRAY_SECTION with pointer base */
    #pragma omp target update from(global_ptr[5:len])
    
    /* Multiple sections in one directive */
    #pragma omp target data map(global_ptr[0:50], global_ptr[100:50])
    {
        /* Mixed sections */
        for (int i = 0; i < 50; i++) {
            global_ptr[i] += 1.0;
            global_ptr[100 + i] -= 1.0;
        }
    }
}

/* Function 4: Nested array sections in larger construct */
__attribute__((noinline, used))
void func_target_region(void)
{
    int matrix[SIZE][SIZE];
    
    /* Initialize matrix */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * SIZE + j;
        }
    }
    
    /* Complex OMP_ARRAY_SECTION within target region */
    #pragma omp target teams distribute parallel for \
            map(tofrom: matrix[10:50][20:30])
    for (int i = 10; i < 60; i++) {
        for (int j = 20; j < 50; j++) {
            matrix[i][j] += (i + j);
        }
    }
    
    /* Another with computed bounds */
    int start_row = 5;
    int row_count = 20;
    int start_col = 15;
    int col_count = 25;
    
    #pragma omp target update to(matrix[start_row:row_count][start_col:col_count])
}

/* Function 5: Mixed sections with different tree codes as base */
__attribute__((noinline, used))
void func_mixed_bases(void)
{
    struct Data {
        int values[SIZE];
        int *ptr;
    } data;
    
    int local_array[SIZE];
    data.ptr = local_array;
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        data.values[i] = i;
        local_array[i] = SIZE - i;
    }
    
    /* Section on struct member array */
    #pragma omp target data map(data.values[30:70])
    {
        for (int i = 30; i < 100; i++) {
            data.values[i] += 100;
        }
    }
    
    /* Section on dereferenced pointer */
    #pragma omp target data map(data.ptr[10:90])
    {
        for (int i = 10; i < 100; i++) {
            data.ptr[i] *= 2;
        }
    }
    
    /* Multiple independent sections */
    #pragma omp task depend(inout: data.values[0:50]) \
                     depend(inout: local_array[50:50])
    {
        for (int i = 0; i < 50; i++) {
            data.values[i] += local_array[50 + i];
        }
    }
    #pragma omp taskwait
}

int main(void)
{
    /* Initialize global arrays */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i;
        global_float_arr[i] = i * 2.0f;
    }
    
    /* Call functions based on volatile selector to prevent dead code elimination */
    if (selector == 0) {
        func_map_simple();
    }
    if (selector < 1) {  /* Always true but compiler doesn't know */
        func_depend_complex();
    }
    if (selector <= 2) {
        func_update_pointer();
    }
    if (selector != 3) {
        func_target_region();
    }
    if (selector >= 0) {  /* Always true */
        func_mixed_bases();
    }
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += global_arr[i];
        checksum += (int)global_float_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional observable side effect */
    volatile int dummy = checksum;
    
    return 0;
}
