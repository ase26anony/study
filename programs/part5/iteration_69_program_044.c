/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 7;
volatile int vol_var2 = 13;
volatile float vol_float1 = 3.14159f;
volatile float vol_float2 = 2.71828f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr, int size) {
    double sum = 0.0;
    double a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    double r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Unrolled loop with many independent calculations */
    for (int idx = 0; idx < size; idx += 16) {
        /* Create many live temporaries to pressure registers */
        a = arr[idx] * vol_var1;
        b = arr[idx + 1] / (vol_var2 + 1);
        c = a + b;
        d = arr[idx + 2] * arr[idx + 3];
        e = arr[idx + 4] - arr[idx + 5];
        f = d * e;
        g = arr[idx + 6] * vol_float1;
        h = arr[idx + 7] * vol_float2;
        i = g + h;
        j = arr[idx + 8] * arr[idx + 9];
        k = arr[idx + 10] / (arr[idx + 11] + 1.0);
        l = j - k;
        m = arr[idx + 12] + arr[idx + 13];
        n = arr[idx + 14] * arr[idx + 15];
        o = m / (n + 1.0);
        p = l + o;
        
        /* Mix integer and FP operations to create different priorities */
        r1 = a * b + c;
        r2 = d - e * f;
        r3 = g / h + i;
        r4 = j * k - l;
        r5 = m + n * o;
        r6 = p * r1 - r2;
        r7 = r3 / r4 + r5;
        r8 = r6 * r7;
        
        sum += r8;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%rax\n"
            "mov $0, %%rbx\n"
            "mov $0, %%rcx\n"
            "mov $0, %%rdx\n"
            "mov $0, %%rsi\n"
            "mov $0, %%rdi\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
        );
    }
    
    return sum;
}

/* Function with mixed dependencies and potential delays */
float mixed_dependency_chain(int iterations) {
    float x = 1.0f;
    float y = 2.0f;
    float z = 3.0f;
    float w = 4.0f;
    float result = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        /* Long latency FP operations that can cause delays */
        float t1 = x / y;  /* FP divide - high latency */
        float t2 = y * z;
        float t3 = z + w;
        
        /* Independent integer operations that can be scheduled around delays */
        int i1 = vol_var1 * i;
        int i2 = vol_var2 + i;
        int i3 = i1 - i2;
        
        /* More FP ops with dependencies */
        float t4 = t1 * t2;
        float t5 = t3 / t4;
        float t6 = t5 + t1;
        
        /* Memory operations with aliasing potential */
        volatile float* mem_ptr = &vol_float1;
        float t7 = *mem_ptr * t6;
        
        /* Control flow to create priority differences */
        if (i3 > 1000) {
            t7 = t7 * 2.0f;
        } else {
            t7 = t7 / 2.0f;
        }
        
        result += t7;
        
        /* Rotate values to create changing dependencies */
        float temp = x;
        x = y;
        y = z;
        z = w;
        w = temp + 0.1f;
    }
    
    return result;
}

/* Function with many independent instructions for candidate selection */
void independent_instruction_block(double *out, const double *in, int n) {
    /* Many independent instructions that can be reordered */
    for (int i = 0; i < n; i++) {
        double v1 = in[i] * 1.1;
        double v2 = in[i] + 2.2;
        double v3 = in[i] - 3.3;
        double v4 = in[i] / 4.4;
        double v5 = v1 * v2;
        double v6 = v3 + v4;
        double v7 = v5 - v6;
        double v8 = v1 + v3;
        double v9 = v2 * v4;
        double v10 = v7 / v8;
        double v11 = v9 + v10;
        double v12 = v5 * v6;
        double v13 = v8 - v9;
        double v14 = v10 * v11;
        double v15 = v12 + v13;
        double v16 = v14 - v15;
        
        out[i] = v16;
        
        /* Artificial resource conflict with inline assembly */
        __asm__ volatile (
            "mfence\n"
            :
            :
            : "memory"
        );
    }
}

/* Main function that creates the hot loops for scheduling analysis */
int main() {
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    double *array1 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *array2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double *result = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!array1 || !array2 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)rand() / RAND_MAX * 100.0;
        array2[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    double total_sum = 0.0;
    float chain_result = 0.0f;
    
    /* Performance-critical loop that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        double sum1 = high_pressure_loop(array1, ARRAY_SIZE);
        float chain = mixed_dependency_chain(100);
        independent_instruction_block(result, array2, ARRAY_SIZE / 4);
        
        /* Mix results to prevent dead code elimination */
        total_sum += sum1 + result[iter % (ARRAY_SIZE / 4)] + chain;
        chain_result += chain;
        
        /* Modify inputs slightly to change scheduling patterns */
        array1[iter % ARRAY_SIZE] += 0.01;
        array2[iter % ARRAY_SIZE] -= 0.01;
    }
    
    /* Use results to prevent optimization */
    printf("Total sum: %f\n", total_sum);
    printf("Chain result: %f\n", chain_result);
    
    /* Checksum to verify correctness */
    double checksum = total_sum + chain_result;
    printf("Checksum: %f\n", checksum);
    
    free(array1);
    free(array2);
    free(result);
    
    return 0;
}
