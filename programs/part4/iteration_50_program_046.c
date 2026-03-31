/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
int g_results[8] = {0};
float g_float_array[1024];
int g_int_array[1024];

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, float *farr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dependencies(int *arr, int n, int threshold);
int test_mixed_data_types(int *iarr, float *farr, double *darr, int n);
int test_loop_carried_deps(int *arr, int n, int distance);
int test_nested_loop_deps(int *arr, int n, int m);
int test_function_call_deps(int *arr, int n);

/* Helper function with side effects to prevent inlining */
__attribute__((noinline)) 
int helper_with_side_effect(int x, int y) {
    g_volatile_counter++;
    return x * y + g_volatile_counter;
}

/* 1. Test RAW (Read-After-Write) dependencies */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies in a chain */
    for (int i = 1; i < n; i++) {
        /* True dependency chain: arr[i] depends on arr[i-1] */
        arr[i] = arr[i - 1] + i;
        
        /* Another RAW dependency with computation */
        int temp = arr[i] * 2;
        arr[i] = temp - 1;
        
        /* Cross-iteration RAW dependency */
        if (i > 2) {
            arr[i] += arr[i - 2];
        }
        
        sum += arr[i];
    }
    
    /* Prevent dead code elimination */
    return sum + g_volatile_counter;
}

/* 2. Test WAR (Write-After-Read) and WAW (Write-After-Write) dependencies */
int test_war_waw_dependencies(int *arr, float *farr, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int read_val = arr[i];  /* Read arr[i] */
        
        /* WAR: arr[i] is written after being read above */
        arr[i] = i * 2 + read_val;
        
        /* WAW: Multiple writes to same location */
        farr[i] = i * 1.5f;
        farr[i] = farr[i] * 2.0f;  /* Overwrites previous value */
        
        /* Another WAW with different computation paths */
        if (i % 3 == 0) {
            arr[i] = read_val + 10;
        } else {
            arr[i] = read_val + 20;
        }
        
        total += arr[i] + (int)farr[i];
    }
    
    return total;
}

/* 3. Test memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int result = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1;
    ptr2 = arr2;
    
    /* Compiler can't know if ptr1 and ptr2 alias */
    for (int i = 0; i < n - 1; i++) {
        *ptr1 = i * 3;
        *ptr2 = *ptr1 + 5;  /* Possible memory dependency */
        
        /* Array access with variable index - creates ambiguous dependencies */
        int idx = i % 10;
        arr1[idx] = arr2[idx] + 7;
        
        /* Pointer arithmetic that may cause aliasing */
        ptr1 = &arr1[(i + 1) % n];
        ptr2 = &arr2[(i + 2) % n];
        
        result += *ptr1 + *ptr2;
    }
    
    return result;
}

/* 4. Test control dependencies */
int test_control_dependencies(int *arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Control-dependent computations */
        if (val > threshold) {
            arr[i] = val * 2;
            count += 3;
        } else if (val < -threshold) {
            arr[i] = val / 2;
            count -= 2;
        } else {
            arr[i] = val + 1;
            count++;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                arr[i] += j;
            }
        }
    }
    
    return count;
}

/* 5. Test mixed data type dependencies */
int test_mixed_data_types(int *iarr, float *farr, double *darr, int n) {
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Integer to float conversion creates dependencies */
        farr[i] = (float)iarr[i] * 1.5f;
        
        /* Float to double conversion */
        darr[i] = (double)farr[i] * 2.5;
        
        /* Mixed-type computation */
        iarr[i] = (int)darr[i] + iarr[i % 10];
        
        /* More mixed operations */
        if (i % 2 == 0) {
            farr[i] = farr[i] + (float)i;
        } else {
            darr[i] = darr[i] - (double)i;
        }
        
        sum += darr[i] + farr[i] + iarr[i];
    }
    
    return (int)sum;
}

/* 6. Test loop-carried dependencies with distance > 0 */
int test_loop_carried_deps(int *arr, int n, int distance) {
    int sum = 0;
    
    /* Loop-carried dependency with distance */
    for (int i = distance; i < n; i++) {
        /* Depends on value from 'distance' iterations ago */
        arr[i] = arr[i - distance] + i * 3;
        
        /* Multiple distances */
        if (i >= 2) {
            arr[i] += arr[i - 2];
        }
        if (i >= 4) {
            arr[i] -= arr[i - 4];
        }
        
        sum += arr[i];
    }
    
    /* Another loop with different distance */
    for (int i = 0; i < n - 3; i++) {
        arr[i] = arr[i + 3] / 2;
        sum += arr[i];
    }
    
    return sum;
}

/* 7. Test nested loop dependencies */
int test_nested_loop_deps(int *arr, int n, int m) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int row_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < m; j++) {
            /* Cross-iteration dependency in inner loop */
            arr[i * m + j] = arr[i * m + j - 1] + (i * j);
            
            /* Dependency on outer loop variable */
            row_sum += arr[i * m + j] * i;
            
            /* Dependency on previous row */
            if (i > 0) {
                arr[i * m + j] += arr[(i - 1) * m + j] / 2;
            }
        }
        
        total += row_sum;
    }
    
    return total;
}

/* 8. Test dependencies with function calls */
int test_function_call_deps(int *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Function call creates memory clobbering - conservative dependencies */
        int computed = helper_with_side_effect(arr[i], i);
        
        /* Result depends on function call */
        arr[i] = computed + arr[i % 5];
        
        /* Another computation that depends on arr[i] */
        if (i > 0) {
            arr[i] += arr[i - 1] / 2;
        }
        
        result += arr[i];
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int n = 1000;
    int m = 50;
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 1024; i++) {
        g_int_array[i] = (i * 3) % 97;
        g_float_array[i] = (float)(i % 53) * 1.1f;
    }
    
    /* Dynamically allocate arrays to prevent static analysis */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    double *darr = (double*)malloc(n * sizeof(double));
    
    /* Initialize with runtime values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 7) % 113;
        arr2[i] = (i * 11) % 131;
        farr[i] = (float)(i % 71) * 0.7f;
        darr[i] = (double)(i % 89) * 0.9;
    }
    
    /* Call all test functions to create various DDG edges */
    g_results[0] = test_raw_dependencies(arr1, n);
    g_results[1] = test_war_waw_dependencies(arr2, farr, n);
    
    int *ptr1, *ptr2;
    g_results[2] = test_memory_aliasing(arr1, arr2, ptr1, ptr2, n);
    
    g_results[3] = test_control_dependencies(arr1, n, 50);
    g_results[4] = test_mixed_data_types(arr1, farr, darr, n);
    g_results[5] = test_loop_carried_deps(arr2, n, 3);
    g_results[6] = test_nested_loop_deps(arr1, 20, 25);
    g_results[7] = test_function_call_deps(arr2, n);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < 8; i++) {
        final_sum += g_results[i];
    }
    
    /* Print result to ensure code executes */
    printf("DDG test checksum: %d (volatile counter: %d)\n", 
           final_sum, g_volatile_counter);
    
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return final_sum != 0 ? 0 : 1;
}
