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
volatile double global_accumulator = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1, prod3 = 1, prod4 = 1;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Complex loop structure with many intermediate values */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i] * 3;
        temp2 = data[i] + 7;
        
        for (j = 0; j < 8; j++) {
            temp3 = temp1 * j + temp2;
            temp4 = data[j] * temp3;
            
            for (k = 0; k < 4; k++) {
                temp5 = temp4 >> k;
                temp6 = temp3 << k;
                
                for (l = 0; l < 2; l++) {
                    temp7 = temp5 + temp6 + l;
                    temp8 = temp7 * global_seed;
                    
                    sum1 += temp8;
                    prod1 *= (temp8 & 0xFF) + 1;
                }
                
                sum2 += temp6;
                prod2 *= (temp6 & 0x7F) + 1;
            }
            
            sum3 += temp4;
            prod3 *= (temp4 & 0x3F) + 1;
        }
        
        sum4 += temp2;
        prod4 *= (temp2 & 0x1F) + 1;
    }
    
    /* Force all values to be used */
    global_accumulator += sum1 + sum2 + sum3 + sum4;
    global_accumulator += prod1 + prod2 + prod3 + prod4;
}

/* Function 2: Complex control flow with many basic blocks */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex if-else chain creating many basic blocks */
        if (data[i] < 100) {
            if (data[i] < 50) {
                if (data[i] < 25) {
                    if (data[i] < 10) {
                        result += data[i] * 2;
                    } else {
                        result += data[i] * 3;
                    }
                } else {
                    if (data[i] < 37) {
                        result += data[i] * 4;
                    } else {
                        result += data[i] * 5;
                    }
                }
            } else {
                if (data[i] < 75) {
                    if (data[i] < 62) {
                        result += data[i] * 6;
                    } else {
                        result += data[i] * 7;
                    }
                } else {
                    if (data[i] < 87) {
                        result += data[i] * 8;
                    } else {
                        result += data[i] * 9;
                    }
                }
            }
        } else {
            /* Switch statement with many cases */
            switch (data[i] % 15) {
                case 0: result += data[i] + 1; break;
                case 1: result += data[i] + 2; break;
                case 2: result += data[i] + 3; break;
                case 3: result += data[i] + 4; break;
                case 4: result += data[i] + 5; break;
                case 5: result += data[i] + 6; break;
                case 6: result += data[i] + 7; break;
                case 7: result += data[i] + 8; break;
                case 8: result += data[i] + 9; break;
                case 9: result += data[i] + 10; break;
                case 10: result += data[i] + 11; break;
                case 11: result += data[i] + 12; break;
                case 12: result += data[i] + 13; break;
                case 13: result += data[i] + 14; break;
                case 14: result += data[i] + 15; break;
                default: result += data[i]; break;
            }
        }
        
        /* Early returns in loop */
        if (result > 1000000) {
            return result / 2;
        }
        
        if (i % 100 == 0 && result < 0) {
            return 0;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_constraints(int *data, int size) {
    int i;
    int a, b, c, d, e, f;
    
    for (i = 0; i < size; i += 8) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $100, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i + 1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl $50, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i + 2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl $0xFF, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i + 3])
            : "%edx", "memory"
        );
        
        /* Use all constrained values */
        e = a + b + c + d;
        
        asm volatile (
            "movl %1, %%esi\n\t"
            "movl %2, %%edi\n\t"
            "addl %%esi, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (f)
            : "r" (e), "r" (data[i + 4])
            : "%esi", "%edi", "memory"
        );
        
        global_accumulator += f;
    }
}

/* Function 4: Mixed data types stressing register classes */
double test_mixed_types(int *int_data, double *double_data, char *char_data, int size) {
    int i;
    double result = 0.0;
    float f1, f2, f3;
    short s1, s2, s3;
    char c1, c2, c3;
    long long ll1, ll2, ll3;
    
    for (i = 0; i < size; i++) {
        /* Mixed type computations */
        f1 = (float)int_data[i] * 0.5f;
        f2 = (float)double_data[i];
        f3 = f1 + f2 * 2.0f;
        
        s1 = (short)(int_data[i] % 65536);
        s2 = (short)char_data[i] * 2;
        s3 = s1 + s2;
        
        c1 = char_data[i];
        c2 = (char)(int_data[i] % 256);
        c3 = c1 ^ c2;
        
        ll1 = (long long)int_data[i] * 1000LL;
        ll2 = (long long)(double_data[i] * 1000.0);
        ll3 = ll1 + ll2;
        
        /* Force use of all values */
        result += f3 + s3 + c3 + (double)(ll3 % 10000);
    }
    
    return result;
}

/* Function 5: Many function calls with register arguments */
int recursive_func(int n, int depth) {
    if (depth <= 0) return n;
    
    int a = recursive_func(n + 1, depth - 1);
    int b = recursive_func(n - 1, depth - 1);
    int c = recursive_func(n * 2, depth - 1);
    int d = recursive_func(n / 2, depth - 1);
    
    return a + b + c + d;
}

void test_many_calls(int *data, int size) {
    int i;
    int results[8];
    
    for (i = 0; i < size; i += 8) {
        /* Multiple function calls with many arguments */
        results[0] = recursive_func(data[i], 3);
        results[1] = recursive_func(data[i + 1], 2);
        results[2] = recursive_func(data[i + 2], 1);
        results[3] = recursive_func(data[i + 3], 4);
        results[4] = recursive_func(data[i + 4], 2);
        results[5] = recursive_func(data[i + 5], 3);
        results[6] = recursive_func(data[i + 6], 1);
        results[7] = recursive_func(data[i + 7], 2);
        
        /* Use all results */
        int sum = 0;
        for (int j = 0; j < 8; j++) {
            sum += results[j];
        }
        global_accumulator += sum;
    }
}

/* Function 6: Vector-like operations */
void test_vector_ops(int *data, int size) {
    int i;
    /* Declare many variables to increase register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int w0, w1, w2, w3, w4, w5, w6, w7;
    
    for (i = 0; i < size; i += 16) {
        /* SIMD-like manual unrolling */
        v0 = data[i] * 3;
        v1 = data[i + 1] * 5;
        v2 = data[i + 2] * 7;
        v3 = data[i + 3] * 11;
        v4 = data[i + 4] * 13;
        v5 = data[i + 5] * 17;
        v6 = data[i + 6] * 19;
        v7 = data[i + 7] * 23;
        
        w0 = data[i + 8] * 2;
        w1 = data[i + 9] * 4;
        w2 = data[i + 10] * 6;
        w3 = data[i + 11] * 8;
        w4 = data[i + 12] * 10;
        w5 = data[i + 13] * 12;
        w6 = data[i + 14] * 14;
        w7 = data[i + 15] * 16;
        
        /* Cross dependencies */
        v0 = v0 + w7;
        v1 = v1 + w6;
        v2 = v2 + w5;
        v3 = v3 + w4;
        v4 = v4 + w3;
        v5 = v5 + w2;
        v6 = v6 + w1;
        v7 = v7 + w0;
        
        /* Reduce */
        int sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
                  w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7;
        
        global_accumulator += sum;
    }
}

/* Main function with warm-up and verification */
int main() {
    int i, iter;
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    
    /* Initialize with random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (double)(rand() % 1000) / 10.0;
        char_data[i] = (char)(rand() % 256);
    }
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up phase for profile feedback */
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_nested_loops(int_data, ARRAY_SIZE / 4);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    double total_result = 0.0;
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Call all test functions in sequence */
        test_nested_loops(int_data, ARRAY_SIZE / 4);
        asm volatile("" ::: "memory");
        
        int cfg_result = test_complex_cfg(int_data, ARRAY_SIZE);
        total_result += cfg_result;
        asm volatile("" ::: "memory");
        
        test_asm_constraints(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        double mixed_result = test_mixed_types(int_data, double_data, char_data, ARRAY_SIZE / 2);
        total_result += mixed_result;
        asm volatile("" ::: "memory");
        
        test_many_calls(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        test_vector_ops(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Update global seed */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Add global accumulator to final result */
    total_result += global_accumulator;
    
    printf("Test completed. Final result: %f\n", total_result);
    printf("Checksum: %llx\n", (unsigned long long)fabs(total_result * 1000.0));
    
    free(int_data);
    free(double_data);
    free(char_data);
    
    return 0;
}
