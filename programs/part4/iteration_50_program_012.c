/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimizations */
volatile int g_volatile_counter = 0;
int g_global_array[1024];
float g_float_array[1024];
double g_double_array[1024];

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, float *farr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dependencies(int *arr, int n, int threshold);
int test_mixed_dependencies(double *darr, float *farr, int *iarr, int n);
int test_loop_carried_dependencies(int *arr, int n, int distance);

/* Test 1: True Data Dependencies (RAW) with different data types */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Integer RAW dependency chain */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i - 1] + i;  // Flow dependency
        g_float_array[i] = g_float_array[i - 1] * 1.5f;  // Float flow dependency
        g_double_array[i] = g_double_array[i - 1] / 2.0;  // Double flow dependency
    }
    
    /* Mixed type dependencies */
    for (int i = 0; i < n; i++) {
        float temp = (float)arr[i];
        g_float_array[i] = temp + g_float_array[i];
        sum += (int)g_float_array[i];
    }
    
    return sum + g_volatile_counter;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr, float *farr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int temp = arr[i];  // Read arr[i]
        arr[i] = i * 2;     // Write arr[i] - WAR dependency with previous read
        
        float f_temp = farr[i];  // Read farr[i]
        farr[i] = 3.14f;         // Write farr[i] - WAR
        
        /* WAW dependencies */
        arr[i] = temp + 1;       // Second write to arr[i] - WAW
        farr[i] = f_temp * 2.0f; // Second write to farr[i] - WAW
        
        result += arr[i] + (int)farr[i];
    }
    
    return result;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1;
    ptr2 = arr1 + 1;  // ptr2 may alias with ptr1+1
    
    for (int i = 0; i < n - 1; i++) {
        /* Memory dependencies with potential aliasing */
        *ptr1 = i * 3;
        *ptr2 = *ptr1 + 5;  // May create memory dependency edge
        
        /* Array accesses with unknown indices */
        int idx = i % 10;
        arr2[idx] = arr1[idx] + arr2[idx + 1];
        
        /* Pointer arithmetic creating ambiguous dependencies */
        ptr1++;
        ptr2 = ptr1 - 1;
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > threshold) {
            arr[i] = arr[i] * 2;
            count++;
        } else if (arr[i] < -threshold) {
            arr[i] = arr[i] / 2;
            count--;
        } else {
            arr[i] = 0;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                g_global_array[j] = arr[i] + j;
            }
        }
    }
    
    return count;
}

/* Test 5: Mixed Dependencies with Different Data Types */
int test_mixed_dependencies(double *darr, float *farr, int *iarr, int n) {
    double d_sum = 0.0;
    float f_sum = 0.0f;
    int i_sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Interleaved dependencies of different types */
        darr[i] = darr[i - 1] + 1.0;           // Double RAW
        farr[i] = (float)darr[i] * 2.0f;       // Cross-type dependency
        iarr[i] = iarr[i - 1] + (int)farr[i];  // Integer with float dependency
        
        /* Complex expression with multiple dependencies */
        d_sum += darr[i] * i;
        f_sum += farr[i] / (i + 1);
        i_sum += iarr[i];
        
        /* Volatile access creates hard dependency */
        i_sum += g_volatile_counter;
    }
    
    return i_sum + (int)d_sum + (int)f_sum;
}

/* Test 6: Loop-Carried Dependencies with Distance > 0 */
int test_loop_carried_dependencies(int *arr, int n, int distance) {
    int sum = 0;
    
    /* Distance 2 loop-carried dependency */
    for (int i = distance; i < n; i++) {
        arr[i] = arr[i - distance] * 2 + 1;  // Distance > 0 dependency
    }
    
    /* Multiple distances in same loop */
    for (int i = 3; i < n; i++) {
        int val = arr[i - 1] + arr[i - 2] + arr[i - 3];
        arr[i] = (val > 0) ? val : -val;
        sum += arr[i];
    }
    
    return sum;
}

/* Test 7: Nested Loops with Complex Dependencies */
int test_nested_loop_dependencies(int *arr, int n, int m) {
    int total = 0;
    
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < m; j++) {
            /* Cross-iteration dependencies in inner loop */
            g_global_array[j] = g_global_array[j - 1] + arr[i];
            
            /* Dependency between inner and outer loops */
            arr[i] += g_global_array[j] % 7;
            
            inner_sum += g_global_array[j];
        }
        
        /* Outer loop dependency chain */
        total += inner_sum * i;
        
        /* Function call acts as memory clobber */
        total += g_volatile_counter;
    }
    
    return total;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    if (argc > 2) {
        m = atoi(argv[2]);
        if (m <= 0) m = 100;
    }
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < 1024; i++) {
        g_global_array[i] = i % 97;
        g_float_array[i] = (float)(i % 53) * 0.5f;
        g_double_array[i] = (double)(i % 71) * 0.25;
    }
    
    int result = 0;
    
    /* Run all tests to trigger different DDG edge types */
    result += test_raw_dependencies(g_global_array, n);
    result += test_war_waw_dependencies(g_global_array, g_float_array, n);
    
    int *ptr1, *ptr2;
    result += test_memory_aliasing(g_global_array, &g_global_array[512], ptr1, ptr2, n);
    
    result += test_control_dependencies(g_global_array, n, 50);
    result += test_mixed_dependencies(g_double_array, g_float_array, g_global_array, n);
    result += test_loop_carried_dependencies(g_global_array, n, 2);
    result += test_nested_loop_dependencies(g_global_array, n / 10, m);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
