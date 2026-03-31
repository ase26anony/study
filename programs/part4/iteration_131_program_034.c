/* omp_array_sections.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 200
#define CHUNK 50

/* Global arrays for different base cases */
int global_arr[SIZE];
int *dynamic_arr;

/* Prevent optimization and ensure code generation */
volatile int selector = 1;

/* Function 1: Array section with simple VAR_DECL base */
__attribute__((noinline, used))
void func_map_simple(void) {
    int local_arr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        local_arr[i] = i;
    }
    
    /* OMP_ARRAY_SECTION with simple base (op_prio check matters) */
    #pragma omp target data map(local_arr[10:100])
    {
        /* Do some work inside */
        for (int i = 10; i < 110; i++) {
            local_arr[i] *= 2;
        }
    }
    
    /* Use result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += local_arr[i];
    }
    printf("Map simple sum: %d\n", sum);
}

/* Function 2: Array section with ARRAY_REF base (nested subscript) */
__attribute__((noinline, used))
void func_map_complex(void) {
    int matrix[100][100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* OMP_ARRAY_SECTION with ARRAY_REF base - tests op_prio() */
    #pragma omp target data map(matrix[25][10:80])
    {
        for (int j = 10; j < 90; j++) {
            matrix[25][j] += 5;
        }
    }
    
    /* Also test with pointer base */
    int *ptr = &matrix[0][0];
    #pragma omp target data map(ptr[1000:500])
    {
        for (int i = 1000; i < 1500; i++) {
            ptr[i] -= 3;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            sum += matrix[i][j];
        }
    }
    printf("Map complex sum: %d\n", sum);
}

/* Function 3: Array section in depend clause */
__attribute__((noinline, used))
void func_depend(void) {
    int arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 2;
    }
    
    int n = 5;
    int section_size = SIZE / 4;
    
    /* Multiple array sections with variable expressions */
    #pragma omp task depend(inout: arr[n:section_size]) \
                     depend(in: arr[n+section_size:section_size/2])
    {
        for (int i = n; i < n + section_size; i++) {
            arr[i] += 10;
        }
    }
    
    #pragma omp taskwait
    
    /* Array section with arithmetic in bounds */
    int offset = 10;
    #pragma omp task depend(inout: arr[2*offset:SIZE/8])
    {
        for (int i = 2*offset; i < 2*offset + SIZE/8; i++) {
            arr[i] *= 2;
        }
    }
    
    #pragma omp taskwait
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    printf("Depend sum: %d\n", sum);
}

/* Function 4: Array section in target update */
__attribute__((noinline, used))
void func_update(void) {
    static int static_arr[SIZE];
    
    /* Initialize static array */
    for (int i = 0; i < SIZE; i++) {
        static_arr[i] = i * 3;
    }
    
    /* Update with different section expressions */
    int start = 15;
    int len = 75;
    
    #pragma omp target update to(static_arr[start:len])
    
    /* Modify on host */
    for (int i = start; i < start + len; i++) {
        static_arr[i] += 100;
    }
    
    #pragma omp target update from(static_arr[0:SIZE/2])
    
    /* Another update with constant bounds */
    #pragma omp target update to(static_arr[SIZE/4:SIZE/4])
    
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += static_arr[i];
    }
    printf("Update sum: %d\n", sum);
}

/* Function 5: Mixed array sections in target enter/exit data */
__attribute__((noinline, used))
void func_enter_exit(void) {
    int arr1[SIZE], arr2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
    }
    
    /* Multiple array sections in one directive */
    #pragma omp target enter data map(to: arr1[0:50], arr2[25:75])
    
    /* Modify on device would go here in real code */
    
    #pragma omp target exit data map(from: arr1[10:40], arr2[0:100])
    
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < SIZE; i++) {
        sum1 += arr1[i];
        sum2 += arr2[i];
    }
    printf("Enter/exit sums: %d, %d\n", sum1, sum2);
}

/* Function 6: Global and dynamic array sections */
__attribute__((noinline, used))
void func_global_dynamic(void) {
    /* Initialize global array */
    for (int i = 0; i < SIZE; i++) {
        global_arr[i] = i * 5;
    }
    
    /* Dynamic array */
    dynamic_arr = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        dynamic_arr[i] = i * 7;
    }
    
    /* Global array section */
    #pragma omp target data map(global_arr[30:120])
    {
        for (int i = 30; i < 150; i++) {
            global_arr[i] -= 2;
        }
    }
    
    /* Dynamic array section */
    int dyn_start = 40;
    int dyn_len = 80;
    #pragma omp target data map(dynamic_arr[dyn_start:dyn_len])
    {
        for (int i = dyn_start; i < dyn_start + dyn_len; i++) {
            dynamic_arr[i] += 8;
        }
    }
    
    int sum_global = 0, sum_dynamic = 0;
    for (int i = 0; i < SIZE; i++) {
        sum_global += global_arr[i];
        sum_dynamic += dynamic_arr[i];
    }
    
    free(dynamic_arr);
    printf("Global/Dynamic sums: %d, %d\n", sum_global, sum_dynamic);
}

int main(void) {
    /* Use volatile to prevent constant folding and ensure all code paths
       are considered by the compiler */
    volatile int mode = selector;
    
    /* Call different functions based on mode to ensure all are compiled */
    if (mode > 0) {
        func_map_simple();
    }
    if (mode > 1) {
        func_map_complex();
    }
    if (mode > 2) {
        func_depend();
    }
    if (mode > 3) {
        func_update();
    }
    if (mode > 4) {
        func_enter_exit();
    }
    if (mode > 5) {
        func_global_dynamic();
    }
    
    /* Also call everything unconditionally at runtime */
    func_map_simple();
    func_map_complex();
    #pragma omp parallel
    {
        #pragma omp single
        {
            func_depend();
        }
    }
    func_update();
    func_enter_exit();
    func_global_dynamic();
    
    return 0;
}
