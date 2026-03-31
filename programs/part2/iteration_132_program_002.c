/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_sum = 0.0;

/* Complex structure to force register pressure */
struct DataBlock {
    int values[8];
    double fp_values[4];
    char buffer[32];
    short shorts[16];
    long long ints[4];
};

/* Function with deeply nested loops and many live ranges */
void test_nested_loops(struct DataBlock *data, int size) {
    int i, j, k, l;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    int temp1, temp2, temp3, temp4, temp5;
    float f1, f2, f3, f4;
    long long acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Outer loop creates many overlapping live ranges */
    for (i = 0; i < size; i++) {
        /* Multiple intermediate values that need registers */
        temp1 = data[i].values[0] * 2;
        temp2 = data[i].values[1] + temp1;
        temp3 = data[i].values[2] - temp2;
        temp4 = data[i].values[3] / (temp3 + 1);
        temp5 = data[i].values[4] ^ temp4;
        
        /* Nested loop with more temporaries */
        for (j = 0; j < 8; j++) {
            int inner_temp1 = data[i].values[j];
            int inner_temp2 = inner_temp1 * j;
            int inner_temp3 = inner_temp2 + i;
            int inner_temp4 = inner_temp3 ^ 0x55AA;
            
            /* Another level of nesting */
            for (k = 0; k < 4; k++) {
                double fp_temp1 = data[i].fp_values[k];
                double fp_temp2 = fp_temp1 * k;
                double fp_temp3 = fp_temp2 + j;
                double fp_temp4 = fp_temp3 / (i + 1.0);
                
                /* Deepest nesting level */
                for (l = 0; l < 2; l++) {
                    f1 = (float)fp_temp4 * l;
                    f2 = f1 + (float)inner_temp4;
                    f3 = f2 * (float)temp5;
                    f4 = f3 - (float)global_counter;
                    
                    sum1 += f1;
                    sum2 += f2;
                    sum3 += f3 + f4;
                    
                    /* Force register pressure with many operations */
                    acc1 += (long long)(f1 * 1000);
                    acc2 += (long long)(f2 * 1000);
                    acc3 += (long long)(f3 * 1000) + (long long)(f4 * 1000);
                }
            }
        }
        
        /* Mix data types to stress different register classes */
        data[i].shorts[0] = (short)(temp1 & 0xFFFF);
        data[i].shorts[1] = (short)(temp2 & 0xFFFF);
        data[i].ints[0] = acc1;
        data[i].ints[1] = acc2;
        data[i].ints[2] = acc3;
    }
    
    global_sum += sum1 + sum2 + sum3;
    global_counter += (int)(acc1 + acc2 + acc3) % 1000;
}

/* Function with complex control flow and switch statement */
int test_complex_cfg(int *array, int size) {
    int result = 0;
    int i = 0;
    
    /* Complex if-else chain with early returns */
    if (size < 10) {
        for (i = 0; i < size; i++) {
            if (array[i] < 0) return -1;
            if (array[i] > 1000) return 1;
        }
        return 0;
    } else if (size < 100) {
        /* Nested loops with breaks at different levels */
        for (i = 0; i < size; i++) {
            for (int j = 0; j < 5; j++) {
                if (array[i] + j > 500) {
                    break;  /* Inner loop break */
                }
                result += array[i] * j;
                
                for (int k = 0; k < 3; k++) {
                    if (result > 10000) {
                        goto early_exit;  /* Multi-level exit */
                    }
                    result -= k;
                }
            }
            if (result < -1000) {
                break;  /* Outer loop break */
            }
        }
    } else {
        /* Switch with many cases including fall-through */
        for (i = 0; i < size && i < 1000; i++) {
            switch (array[i] % SWITCH_CASES) {
                case 0:
                    result += array[i];
                    /* Fall through */
                case 1:
                    result += array[i] * 2;
                    break;
                case 2:
                case 3:  /* Combined cases */
                    result += array[i] * 3;
                    break;
                case 4:
                    result -= array[i];
                    /* Fall through */
                case 5:
                    result -= array[i] * 2;
                    break;
                case 6:
                    result *= 2;
                    break;
                case 7:
                    result /= (array[i] + 1);
                    break;
                case 8:
                    result ^= array[i];
                    break;
                case 9:
                    result |= array[i];
                    break;
                case 10:
                    result &= array[i];
                    break;
                case 11:
                    result <<= (array[i] % 8);
                    break;
                case 12:
                    result >>= (array[i] % 8);
                    break;
                case 13:
                    result = ~result;
                    break;
                case 14:
                    result = abs(result);
                    break;
                default:
                    result = 0;
                    break;
            }
            
            /* Continue at different nesting levels */
            if (result > 1000000) {
                continue;
            }
            
            /* Nested if with else chain */
            if (array[i] > 500) {
                if (result < 0) {
                    result = -result;
                } else if (result == 0) {
                    result = 1;
                } else {
                    result += 1000;
                }
            } else if (array[i] > 250) {
                result += 500;
            } else if (array[i] > 100) {
                result += 250;
            } else {
                result += 100;
            }
        }
    }
    
early_exit:
    return result;
}

/* Function with inline assembly forcing register constraints */
void test_asm_register_pressure(int *input, int *output, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Force specific registers with inline asm */
        asm volatile (
            "movl %[in0], %%eax\n\t"
            "movl %[in1], %%ebx\n\t"
            "movl %[in2], %%ecx\n\t"
            "movl %[in3], %%edx\n\t"
            "addl %%eax, %%ebx\n\t"
            "addl %%ecx, %%edx\n\t"
            "imull %%ebx, %%eax\n\t"
            "imull %%edx, %%ecx\n\t"
            "movl %%eax, %[out0]\n\t"
            "movl %%ebx, %[out1]\n\t"
            "movl %%ecx, %[out2]\n\t"
            "movl %%edx, %[out3]\n\t"
            : [out0] "=r" (a), [out1] "=r" (b), 
              [out2] "=r" (c), [out3] "=r" (d)
            : [in0] "r" (input[i]), [in1] "r" (input[i+1]),
              [in2] "r" (input[i+2]), [in3] "r" (input[i+3])
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* More asm with different register constraints */
        asm volatile (
            "movq %[in4], %%r8\n\t"
            "movq %[in5], %%r9\n\t"
            "movq %[in6], %%r10\n\t"
            "movq %[in7], %%r11\n\t"
            "addq %%r8, %%r9\n\t"
            "addq %%r10, %%r11\n\t"
            "imulq %%r9, %%r8\n\t"
            "imulq %%r11, %%r10\n\t"
            "movq %%r8, %[out4]\n\t"
            "movq %%r9, %[out5]\n\t"
            "movq %%r10, %[out6]\n\t"
            "movq %%r11, %[out7]\n\t"
            : [out4] "=r" (e), [out5] "=r" (f),
              [out6] "=r" (g), [out7] "=r" (h)
            : [in4] "r" ((long long)input[i+4]),
              [in5] "r" ((long long)input[i+5]),
              [in6] "r" ((long long)input[i+6]),
              [in7] "r" ((long long)input[i+7])
            : "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Store results, creating more register pressure */
        output[i] = a + e;
        output[i+1] = b + f;
        output[i+2] = c + g;
        output[i+3] = d + h;
        output[i+4] = a * e;
        output[i+5] = b * f;
        output[i+6] = c * g;
        output[i+7] = d * h;
    }
}

/* Function with many arguments to stress register/stack passing */
long long test_many_args(int a, int b, int c, int d, int e, int f, int g,
                         int h, int i, int j, int k, int l, int m, int n) {
    /* Use all arguments in complex expressions */
    int t1 = a * b + c - d;
    int t2 = e * f + g - h;
    int t3 = i * j + k - l;
    int t4 = m * n + a - b;
    
    int u1 = t1 ^ t2;
    int u2 = t3 ^ t4;
    int u3 = u1 & u2;
    int u4 = u1 | u2;
    
    long long v1 = (long long)u1 * u2;
    long long v2 = (long long)u3 * u4;
    long long v3 = v1 + v2;
    long long v4 = v1 - v2;
    
    return v3 * v4 + (long long)a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

/* Function with vector-like operations using multiple registers */
void test_vector_ops(float *a, float *b, float *c, int size) {
    int i;
    for (i = 0; i < size; i += 4) {
        /* Manual vector operations to use multiple FP registers */
        float a0 = a[i], a1 = a[i+1], a2 = a[i+2], a3 = a[i+3];
        float b0 = b[i], b1 = b[i+1], b2 = b[i+2], b3 = b[i+3];
        
        /* Multiple FP operations in sequence */
        float t0 = a0 * b0 + 1.0f;
        float t1 = a1 * b1 + 2.0f;
        float t2 = a2 * b2 + 3.0f;
        float t3 = a3 * b3 + 4.0f;
        
        float u0 = t0 / (b0 + 1.0f);
        float u1 = t1 / (b1 + 1.0f);
        float u2 = t2 / (b2 + 1.0f);
        float u3 = t3 / (b3 + 1.0f);
        
        float v0 = sqrtf(fabsf(u0));
        float v1 = sqrtf(fabsf(u1));
        float v2 = sqrtf(fabsf(u2));
        float v3 = sqrtf(fabsf(u3));
        
        c[i] = v0 + v1;
        c[i+1] = v1 + v2;
        c[i+2] = v2 + v3;
        c[i+3] = v3 + v0;
    }
}

/* Main function that orchestrates all tests */
int main() {
    int i, j;
    struct DataBlock *data;
    int *int_array, *output_array;
    float *float_a, *float_b, *float_c;
    
    /* Allocate large arrays */
    data = (struct DataBlock*)malloc(ARRAY_SIZE * sizeof(struct DataBlock));
    int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    output_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!data || !int_array || !output_array || !float_a || !float_b || !float_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        for (j = 0; j < 8; j++) {
            data[i].values[j] = rand() % 1000;
        }
        for (j = 0; j < 4; j++) {
            data[i].fp_values[j] = (double)rand() / RAND_MAX * 100.0;
        }
        for (j = 0; j < 16; j++) {
            data[i].shorts[j] = (short)(rand() % 1000);
        }
        int_array[i] = rand() % 1000;
        float_a[i] = (float)rand() / RAND_MAX * 100.0f;
        float_b[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up phase for profile feedback */
    for (i = 0; i < 5; i++) {
        test_nested_loops(data, 1000);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    long long total_result = 0;
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Nested loops with many live ranges */
        test_nested_loops(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        int cfg_result = test_complex_cfg(int_array, ARRAY_SIZE / 20);
        total_result += cfg_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly with register constraints */
        test_asm_register_pressure(int_array, output_array, ARRAY_SIZE / 40);
        asm volatile("" ::: "memory");
        
        /* Test 4: Many arguments */
        total_result += test_many_args(
            int_array[0], int_array[1], int_array[2], int_array[3],
            int_array[4], int_array[5], int_array[6], int_array[7],
            int_array[8], int_array[9], int_array[10], int_array[11],
            int_array[12], int_array[13]
        );
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        test_vector_ops(float_a, float_b, float_c, ARRAY_SIZE / 8);
        asm volatile("" ::: "memory");
        
        /* Modify data slightly each iteration */
        for (j = 0; j < ARRAY_SIZE / 100; j++) {
            int idx = rand() % ARRAY_SIZE;
            int_array[idx] = (int_array[idx] * 13 + 17) % 1000;
            float_a[idx] = float_a[idx] * 0.99f + 0.01f;
        }
    }
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += (unsigned long long)int_array[i];
        checksum += (unsigned long long)(float_c[i] * 1000.0f);
        for (j = 0; j < 8; j++) {
            checksum += (unsigned long long)data[i].values[j];
        }
    }
    checksum += (unsigned long long)total_result;
    checksum += (unsigned long long)global_counter;
    checksum += (unsigned long long)(global_sum * 1000.0);
    
    printf("Test completed. Checksum: %llu\n", checksum);
    printf("Global counter: %d, Global sum: %f\n", global_counter, global_sum);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(output_array);
    free(float_a);
    free(float_b);
    free(float_c);
    
    return 0;
}
