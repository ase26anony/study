/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile double global_sum = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    double fsum = 0.0;
    
    /* Complex nested loop structure */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i] * 2;
        for (j = i; j < size / 8; j++) {
            tmp2 = data[j] + tmp1;
            for (k = j; k < size / 16; k++) {
                tmp3 = data[k] - tmp2;
                for (l = k; l < size / 32; l++) {
                    tmp4 = data[l] / (tmp3 + 1);
                    sum1 += tmp4;
                    
                    /* Complex expression with many intermediates */
                    fsum += (tmp1 * 0.5) + (tmp2 * 0.25) + 
                           (tmp3 * 0.125) + (tmp4 * 0.0625);
                }
                sum2 += tmp3;
            }
            sum3 += tmp2;
        }
        sum4 += tmp1;
    }
    
    /* Force all sums to be used */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4));
    global_sum += fsum;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *data, int size, int mode) {
    int result = 0;
    int i = 0;
    
    /* Multiple nested if-else with early returns */
    if (mode == 0) {
        for (i = 0; i < size; i++) {
            if (data[i] < 0) return -1;
            if (data[i] > 1000) break;
            result += data[i];
        }
        return result;
    } else if (mode == 1) {
        while (i < size) {
            if (data[i] % 2 == 0) {
                result += data[i] * 2;
                i += 2;
                continue;
            } else {
                result -= data[i];
                i++;
                if (result < 0) break;
            }
        }
    }
    
    /* Large switch statement with fall-through */
    switch (mode) {
        case 0: result *= 2; /* Fall through */
        case 1: result += 10; /* Fall through */
        case 2: result -= 5; break;
        case 3: result /= 2; break;
        case 4: result %= 100; break;
        case 5: result = ~result; break;
        case 6: result ^= 0xFF; break;
        case 7: result |= 0xAA; break;
        case 8: result &= 0x55; break;
        case 9: result <<= 2; break;
        case 10: result >>= 1; break;
        default: result = -result;
    }
    
    /* Nested switch inside loop */
    for (i = 0; i < size && i < 100; i++) {
        switch (data[i] % 8) {
            case 0: result += 1; break;
            case 1: result += 2; /* Fall through */
            case 2: result += 3; break;
            case 3: result += 4; /* Fall through */
            case 4: result += 5; /* Fall through */
            case 5: result += 6; break;
            case 6: result += 7; /* Fall through */
            case 7: result += 8; break;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_constraints(int *data, int size) {
    int a, b, c, d, e, f, g, h;
    int i;
    
    for (i = 0; i < size && i < 8; i++) {
        /* Force specific register allocations */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=r"(a) : "r"(data[i]), "0"(a) : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %%ebx, %0\n\t"
            : "=r"(b) : "r"(data[i]), "0"(b) : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl %%ecx, %0\n\t"
            : "=r"(c) : "r"(data[i]), "0"(c) : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl %%edx, %0\n\t"
            : "=r"(d) : "r"(data[i]), "0"(d) : "%edx", "memory"
        );
        
        /* Compete for same registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movl %2, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(e) : "r"(data[i]), "r"(i), "0"(e) : "%eax", "%ebx", "memory"
        );
    }
    
    /* Use all computed values */
    f = a + b;
    g = c - d;
    h = e * f;
    
    asm volatile("" : : "r"(f), "r"(g), "r"(h));
}

/* Function 4: Mixed data types stressing register classes */
double test_mixed_types(char *cdata, short *sdata, int *idata, 
                        long *ldata, float *fdata, double *ddata, int size) {
    double total = 0.0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Mix all data types in complex expressions */
        char c = cdata[i];
        short s = sdata[i];
        int i_val = idata[i];
        long l = ldata[i];
        float f = fdata[i];
        double d = ddata[i];
        
        /* Complex expression using all types */
        double val = (double)c * 0.1 +
                    (double)s * 0.01 +
                    (double)i_val * 0.001 +
                    (double)l * 0.0001 +
                    (double)f * 0.00001 +
                    d;
        
        /* Trigonometric operations using FP registers */
        val = sin(val) * cos(val) + tan(val * 0.5);
        
        total += val;
        
        /* Update arrays with mixed types */
        cdata[i] = (char)(val * 10);
        sdata[i] = (short)(val * 100);
        idata[i] = (int)(val * 1000);
        ldata[i] = (long)(val * 10000);
        fdata[i] = (float)(val * 0.1);
        ddata[i] = val;
    }
    
    return total;
}

/* Function 5: Many function arguments and calls */
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Complex expression using all arguments */
    int result = a1 * a2 + a3 - a4 * a5 / (a6 + 1) +
                a7 % (a8 + 1) + a9 ^ a10 | a11 & a12 +
                (a13 << 2) + (a14 >> 1) + ~a15;
    
    /* Nested function calls */
    result = abs(result);
    result = result * result;
    
    /* Call with many arguments */
    return test_complex_cfg(&a1, 15, result % 10);
}

/* Function 6: Pointer aliasing and volatile variables */
void test_aliasing(volatile int *data1, int *data2, int *data3, int size) {
    int *ptr1 = data1;
    int *ptr2 = data2;
    int *ptr3 = data3;
    int i, j;
    
    /* Create pointer aliases */
    int *alias1 = ptr1;
    int *alias2 = ptr2;
    int *alias3 = ptr3;
    
    /* Complex loop with pointer arithmetic */
    for (i = 0; i < size; i++) {
        /* Multiple updates through different pointers */
        *alias1 = *alias2 + *alias3;
        *alias2 = *alias1 - *alias3;
        *alias3 = *alias1 + *alias2;
        
        /* Volatile access forces memory operations */
        volatile int tmp = *alias1;
        *alias1 = tmp * 2;
        
        /* Update pointers */
        alias1++;
        alias2++;
        alias3++;
        
        /* Additional computation to extend live ranges */
        for (j = 0; j < 4; j++) {
            data1[i] += data2[i] * j;
            data3[i] -= data1[i] / (j + 1);
        }
    }
}

/* Function 7: Vector-like operations */
void test_vector_ops(int *data, int size) {
    int i;
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int w0, w1, w2, w3, w4, w5, w6, w7;
    
    /* Unrolled loop simulating vector operations */
    for (i = 0; i < size - 7; i += 8) {
        /* Load 8 elements */
        v0 = data[i];
        v1 = data[i + 1];
        v2 = data[i + 2];
        v3 = data[i + 3];
        v4 = data[i + 4];
        v5 = data[i + 5];
        v6 = data[i + 6];
        v7 = data[i + 7];
        
        /* SIMD-like operations */
        w0 = v0 * v1;
        w1 = v1 + v2;
        w2 = v2 - v3;
        w3 = v3 ^ v4;
        w4 = v4 | v5;
        w5 = v5 & v6;
        w6 = v6 << 1;
        w7 = v7 >> 2;
        
        /* More operations mixing results */
        v0 = w0 + w1;
        v1 = w1 - w2;
        v2 = w2 * w3;
        v3 = w3 / (w4 + 1);
        v4 = w4 % (w5 + 1);
        v5 = w5 ^ w6;
        v6 = w6 | w7;
        v7 = w7 & w0;
        
        /* Store results */
        data[i] = v0;
        data[i + 1] = v1;
        data[i + 2] = v2;
        data[i + 3] = v3;
        data[i + 4] = v4;
        data[i + 5] = v5;
        data[i + 6] = v6;
        data[i + 7] = v7;
    }
}

/* Main function that runs all tests */
int main() {
    int i, j;
    clock_t start, end;
    double total_time = 0.0;
    
    /* Allocate and initialize test data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!int_data || !char_data || !short_data || 
        !long_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_data[i] = (char)(rand() % 256);
        short_data[i] = (short)(rand() % 1000);
        long_data[i] = (long)(rand() % 10000);
        float_data[i] = (float)(rand() % 1000) / 10.0f;
        double_data[i] = (double)(rand() % 10000) / 100.0;
    }
    
    printf("Starting register pressure stress test...\n");
    printf("Array size: %d, Iterations: %d\n", ARRAY_SIZE, ITERATIONS);
    
    /* Warm-up phase for profile feedback */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (j = 0; j < WARMUP_ITERATIONS; j++) {
        test_nested_loops(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test loop */
    start = clock();
    
    for (j = 0; j < ITERATIONS; j++) {
        /* Test 1: Nested loops */
        test_nested_loops(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        int cfg_result = test_complex_cfg(int_data, ARRAY_SIZE, j % 20);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_asm_constraints(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed data types */
        double mixed_result = test_mixed_types(char_data, short_data, int_data,
                                             long_data, float_data, double_data,
                                             ARRAY_SIZE / 10);
        global_sum += mixed_result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        int args_result = test_many_args(
            int_data[0], int_data[1], int_data[2], int_data[3], int_data[4],
            int_data[5], int_data[6], int_data[7], int_data[8], int_data[9],
            int_data[10], int_data[11], int_data[12], int_data[13], int_data[14]
        );
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        test_aliasing(int_data, int_data + ARRAY_SIZE/2, 
                     int_data + ARRAY_SIZE/4, ARRAY_SIZE/4);
        asm volatile("" ::: "memory");
        
        /* Test 7: Vector operations */
        test_vector_ops(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Update data to prevent pattern optimization */
        for (i = 0; i < ARRAY_SIZE; i++) {
            int_data[i] = (int_data[i] * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    end = clock();
    total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute final checksum */
    long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_data[i];
        checksum += char_data[i];
        checksum += short_data[i];
        checksum += long_data[i];
    }
    
    printf("\nTest completed successfully!\n");
    printf("Total execution time: %.2f seconds\n", total_time);
    printf("Global sum: %.6f\n", global_sum);
    printf("Final checksum: %lld\n", checksum);
    printf("Checksum verification: %s\n", 
           (checksum != 0) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(int_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
