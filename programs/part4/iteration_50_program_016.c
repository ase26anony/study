/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global variables to prevent optimization */
volatile int g_seed = 42;
int g_result[8] = {0};
float g_farray[1024];

/* Function to create unknown iteration count */
static int get_iterations(void) {
    return 1000 + (g_seed % 100);
}

/* 1. Loop with RAW (true) dependencies and loop-carried dependencies */
int test_raw_dep(int *arr, int n, int stride) {
    int sum = 0;
    /* RAW dependency with distance 1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] + i;  /* Flow dependency */
    }
    
    /* Loop-carried dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-2] * 3 + arr[i-1];  /* Multiple flow dependencies */
    }
    
    /* Mixed integer operations */
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        arr[i] = sum;  /* Creates output dependency for next iteration */
    }
    
    return sum;
}

/* 2. Loop with WAR and WAW dependencies */
float test_war_waw_dep(float *a, float *b, int n) {
    float temp = 0.0f;
    
    /* WAR (anti-dependency) pattern */
    for (int i = 0; i < n; i++) {
        float x = a[i];      /* Read a[i] */
        a[i] = b[i] * 2.0f;  /* Write a[i] - anti-dep with previous read */
        b[i] = x + 1.0f;     /* Write b[i] */
    }
    
    /* WAW (output dependency) pattern */
    for (int i = 0; i < n; i++) {
        a[i] = sinf(i * 0.1f);     /* Write a[i] */
        a[i] = cosf(a[i]);         /* Write a[i] again - output dep */
        temp += a[i];
    }
    
    /* Mixed WAR/WAW with different data types */
    for (int i = 0; i < n; i++) {
        int int_val = (int)a[i];
        float f_val = b[i];
        b[i] = f_val * 0.5f;       /* Write b[i] */
        a[i] = (float)int_val;     /* Write a[i] */
        f_val = a[i] + b[i];       /* Read both */
        temp += f_val;
    }
    
    return temp;
}

/* 3. Loop with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1 + 1;
    ptr2 = arr2 - 1;
    
    /* Loop with ambiguous pointer accesses */
    for (int i = 1; i < n - 1; i++) {
        *ptr1 = arr1[i] * 2;      /* Write through ptr1 */
        arr2[i] = *ptr2 + i;      /* Read through ptr2 */
        ptr1++;
        ptr2++;
        sum += arr1[i] + arr2[i];
    }
    
    /* Array indexing with non-linear pattern */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;    /* Non-linear access pattern */
        arr1[idx] = arr2[i] + g_seed;
        arr2[i] = arr1[idx] - 1;
    }
    
    /* Volatile accesses create hard dependencies */
    volatile int *volatile_ptr = &arr1[0];
    for (int i = 0; i < n; i++) {
        *volatile_ptr = *volatile_ptr + 1;
        volatile_ptr = &arr1[(i + 1) % n];
    }
    
    return sum;
}

/* 4. Loop with control dependencies */
double test_control_dep(double *data, int n, int threshold) {
    double sum = 0.0;
    int count = 0;
    
    /* Loop with internal branching */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            sum += sqrt(data[i]);      /* Control-dependent computation */
            data[i] = log(data[i] + 1.0);
        } else {
            sum += pow(data[i], 1.5);  /* Alternative control-dependent path */
            data[i] = exp(data[i] * 0.5);
        }
        
        /* Nested condition */
        if (i % 3 == 0) {
            data[i] *= 0.9;
            count++;
        } else if (i % 7 == 0) {
            data[i] /= 1.1;
        }
    }
    
    /* Loop with computed goto-like control flow */
    for (int i = 1; i < n; i++) {
        double diff = data[i] - data[i-1];
        if (diff > 0) {
            data[i] = data[i-1] + sin(diff);
        } else {
            data[i] = data[i-1] - cos(-diff);
        }
        sum += data[i];
    }
    
    return sum + count;
}

/* 5. Complex nested loop with mixed dependencies */
int test_nested_mixed(int *matrix, int rows, int cols) {
    int total = 0;
    
    /* Nested loops with various dependencies */
    for (int i = 1; i < rows; i++) {
        for (int j = 1; j < cols; j++) {
            /* RAW within inner loop */
            int val = matrix[i * cols + j - 1];
            
            /* WAR with outer loop iteration */
            matrix[(i-1) * cols + j] = matrix[i * cols + j] + val;
            
            /* WAW on same location */
            matrix[i * cols + j] = val * 2;
            matrix[i * cols + j] = matrix[i * cols + j] + i + j;
            
            total += matrix[i * cols + j];
        }
        
        /* Control dependency in outer loop */
        if (i % 4 == 0) {
            for (int j = 0; j < cols; j++) {
                matrix[i * cols + j] += total;
            }
        }
    }
    
    return total;
}

/* 6. Loop with function calls (act as memory clobbers) */
static int helper_func(int x, int y) {
    return (x * y) % 256;
}

int test_with_calls(int *arr, int n) {
    int result = 0;
    
    for (int i = 1; i < n; i++) {
        /* Function call creates memory dependency */
        int temp = helper_func(arr[i-1], i);
        
        /* Multiple dependencies */
        arr[i] = arr[i] + temp;      /* RAW on arr[i] read, WAR on arr[i-1] */
        result += arr[i];
        
        /* Another call with anti-dependency */
        arr[i-1] = helper_func(result, arr[i]);
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    const int N = 1024;
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    double *darr = malloc(N * sizeof(double));
    int *matrix = malloc(64 * 64 * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 3 + g_seed;
        arr2[i] = i * 5 - g_seed;
        darr[i] = (i % 100) * 0.1;
        g_farray[i] = (float)(i * 0.01);
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = i % 256;
    }
    
    /* Get dynamic iteration count */
    int iterations = get_iterations();
    
    /* Run all tests to create various DDG edges */
    g_result[0] = test_raw_dep(arr1, iterations, 2);
    float fresult = test_war_waw_dep(g_farray, g_farray + 256, iterations);
    g_result[1] = (int)fresult;
    
    g_result[2] = test_memory_aliasing(arr1, arr2, arr1 + 10, arr2 + 20, iterations);
    
    double dresult = test_control_dep(darr, iterations, 5);
    g_result[3] = (int)dresult;
    
    g_result[4] = test_nested_mixed(matrix, 64, 64);
    g_result[5] = test_with_calls(arr1, iterations);
    
    /* Compute final checksum */
    int final_sum = 0;
    for (int i = 0; i < 6; i++) {
        final_sum += g_result[i];
    }
    
    printf("DDG test checksum: %d\n", final_sum);
    
    free(arr1);
    free(arr2);
    free(darr);
    free(matrix);
    
    return final_sum != 0 ? 0 : 1;
}
