/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define NUM_CASES 15

/* Global volatile variables to extend live ranges */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
struct DataPacket {
    int id;
    double values[8];
    float floats[4];
    long long timestamps[2];
    char metadata[64];
};

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(struct DataPacket *data, int size) {
    int i, j, k, l;
    double temp1, temp2, temp3, temp4;
    float f1, f2, f3, f4;
    long long sum1, sum2;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i].values[0];
        temp2 = data[i].values[1];
        
        for (j = 0; j < 8; j++) {
            f1 = data[i].floats[0];
            f2 = data[i].floats[1];
            
            for (k = 0; k < 4; k++) {
                temp3 = temp1 * f1 + temp2 * f2;
                f3 = data[i].floats[2] * k;
                
                for (l = 0; l < 2; l++) {
                    temp4 = temp3 + data[i].values[j % 4] * l;
                    f4 = f3 * data[i].floats[3];
                    
                    /* Complex expression with many intermediates */
                    sum1 = (long long)(temp4 * 1000) + (long long)(f4 * 100);
                    sum2 = data[i].timestamps[l] + sum1;
                    
                    data[i].timestamps[l] = sum2;
                    global_accumulator += temp4 + f4;
                }
                
                /* Force register pressure with many live variables */
                data[i].values[k] = temp1 + temp2 + temp3 + temp4;
                data[i].floats[k] = f1 + f2 + f3 + f4;
            }
            
            /* Inline assembly with register constraints */
            asm volatile (
                "mov %0, %%rax\n\t"
                "add %1, %%rax\n\t"
                "mov %%rax, %0\n\t"
                : "+r" (sum1)
                : "r" (sum2)
                : "%rax", "cc"
            );
        }
        
        global_counter += i;
    }
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *array, int size) {
    int result = 0;
    int i = 0;
    
    /* Complex CFG with many basic blocks */
    while (i < size) {
        int val = array[i];
        
        /* Switch with many cases creates complex CFG */
        switch (val % NUM_CASES) {
            case 0:
                result += val * 2;
                if (result > 1000) return result;  /* Early return */
                break;
            case 1:
                result += val / 2;
                for (int j = 0; j < 5; j++) {
                    result += array[(i + j) % size];
                }
                break;
            case 2:
                result += val << 2;
                if (val % 3 == 0) continue;  /* Continue to next iteration */
                break;
            case 3:
                result += val >> 1;
                /* Nested if-else chain */
                if (val > 500) {
                    result += 100;
                } else if (val > 200) {
                    result += 50;
                } else if (val > 100) {
                    result += 20;
                } else {
                    result += 5;
                }
                break;
            case 4:
                result += val * val;
                /* Another loop inside switch case */
                for (int k = 0; k < 3; k++) {
                    result += k * val;
                }
                break;
            case 5:
                result += val % 100;
                /* goto creating irreducible flow */
                if (val < 0) goto negative_case;
                break;
            case 6:
                result += val + 100;
                /* Function call with many args */
                result += test_many_args(val, val+1, val+2, val+3, 
                                        val+4, val+5, val+6, val+7,
                                        val+8, val+9, val+10);
                break;
            case 7:
                result += val - 50;
                break;
            case 8:
                result += val * 3;
                break;
            case 9:
                result += val / 3;
                break;
            case 10:
                result += val % 7;
                break;
            case 11:
                result += val << 1;
                break;
            case 12:
                result += val >> 2;
                break;
            case 13:
                result += val * 5;
                break;
            case 14:
                result += val / 5;
                break;
        }
        
        /* More register pressure */
        int temp1 = result * 2;
        int temp2 = result / 2;
        int temp3 = temp1 + temp2;
        int temp4 = temp3 * val;
        int temp5 = temp4 % 256;
        
        result = temp5;
        
        i++;
        
        negative_case:
        if (val < 0) {
            result -= 100;
            break;  /* Break from while */
        }
    }
    
    return result;
}

/* Function 3: Many arguments to stress register/stack passing */
int test_many_args(int a, int b, int c, int d, int e,
                   int f, int g, int h, int i, int j, int k) {
    /* All arguments compete for registers */
    int sum = a + b + c + d + e + f + g + h + i + j + k;
    
    /* More register pressure with local variables */
    int t1 = sum * a;
    int t2 = sum * b;
    int t3 = sum * c;
    int t4 = sum * d;
    int t5 = sum * e;
    int t6 = sum * f;
    int t7 = sum * g;
    int t8 = sum * h;
    int t9 = sum * i;
    int t10 = sum * j;
    int t11 = sum * k;
    
    /* Complex expression keeping all temporaries alive */
    return t1 - t2 + t3 - t4 + t5 - t6 + t7 - t8 + t9 - t10 + t11;
}

/* Function 4: SIMD-like operations using multiple registers */
void test_vector_ops(float *a, float *b, float *c, int size) {
    /* Declare many variables at function scope */
    float v1, v2, v3, v4, v5, v6, v7, v8;
    float w1, w2, w3, w4, w5, w6, w7, w8;
    float x1, x2, x3, x4, x5, x6, x7, x8;
    
    for (int i = 0; i < size; i += 8) {
        /* Load 8 values - simulates SIMD load */
        v1 = a[i];     v2 = a[i+1];   v3 = a[i+2];   v4 = a[i+3];
        v5 = a[i+4];   v6 = a[i+5];   v7 = a[i+6];   v8 = a[i+7];
        
        w1 = b[i];     w2 = b[i+1];   w3 = b[i+2];   w4 = b[i+3];
        w5 = b[i+4];   w6 = b[i+5];   w7 = b[i+6];   w8 = b[i+7];
        
        /* Complex vector operations */
        x1 = v1 * w1 + v2 * w2;
        x2 = v3 * w3 + v4 * w4;
        x3 = v5 * w5 + v6 * w6;
        x4 = v7 * w7 + v8 * w8;
        
        x5 = v1 / w1 - v2 / w2;
        x6 = v3 / w3 - v4 / w4;
        x7 = v5 / w5 - v6 / w6;
        x8 = v7 / w7 - v8 / w8;
        
        /* More operations keeping all variables alive */
        c[i]   = x1 + x5;
        c[i+1] = x2 + x6;
        c[i+2] = x3 + x7;
        c[i+3] = x4 + x8;
        c[i+4] = x1 - x5;
        c[i+5] = x2 - x6;
        c[i+6] = x3 - x7;
        c[i+7] = x4 - x8;
        
        /* Inline assembly with multiple constraints */
        asm volatile (
            "movss %0, %%xmm0\n\t"
            "movss %1, %%xmm1\n\t"
            "addss %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "+m" (c[i])
            : "m" (c[i+1])
            : "%xmm0", "%xmm1"
        );
    }
}

/* Function 5: Mixed data types stressing register classes */
double test_mixed_types(char *chars, short *shorts, int *ints, 
                        long *longs, float *floats, double *doubles, int n) {
    double total = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* All different types competing for different register classes */
        char c = chars[i];
        short s = shorts[i];
        int i_val = ints[i];
        long l = longs[i];
        float f = floats[i];
        double d = doubles[i];
        
        /* Complex expression mixing types */
        double temp = (double)c + (double)s + (double)i_val + 
                     (double)l + (double)f + d;
        
        /* More operations with type conversions */
        float f_temp = (float)temp;
        long l_temp = (long)temp;
        int i_temp = (int)temp;
        
        total += temp + f_temp + l_temp + i_temp;
        
        /* Store back to force register spills */
        chars[i] = (char)(temp / 256);
        shorts[i] = (short)(temp / 65536);
        ints[i] = (int)temp;
        longs[i] = (long)temp;
        floats[i] = (float)temp;
        doubles[i] = temp;
    }
    
    return total;
}

/* Function 6: Pointer aliasing to prevent optimizations */
void test_pointer_aliasing(int *a, int *b, int *c, int n) {
    /* Create aliases */
    int *ptr1 = a;
    int *ptr2 = b;
    int *ptr3 = c;
    
    /* volatile pointers to prevent optimization */
    volatile int *vptr1 = a;
    volatile int *vptr2 = b;
    
    for (int i = 0; i < n; i++) {
        /* Complex pointer arithmetic */
        int val1 = *ptr1;
        int val2 = *ptr2;
        int val3 = *ptr3;
        
        /* Operations that keep values alive */
        int sum = val1 + val2 + val3;
        int prod = val1 * val2 * val3;
        int diff = val1 - val2 - val3;
        
        /* Store through different pointers */
        *ptr1 = sum;
        *ptr2 = prod;
        *ptr3 = diff;
        
        /* Read through volatile pointers */
        int v1 = *vptr1;
        int v2 = *vptr2;
        
        /* Use volatile values */
        global_counter += v1 + v2;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    /* Allocate large arrays */
    struct DataPacket *data = malloc(ARRAY_SIZE * sizeof(struct DataPacket));
    int *int_array = malloc(ARRAY_SIZE * sizeof(int));
    float *float_a = malloc(ARRAY_SIZE * sizeof(float));
    float *float_b = malloc(ARRAY_SIZE * sizeof(float));
    float *float_c = malloc(ARRAY_SIZE * sizeof(float));
    
    char *char_array = malloc(ARRAY_SIZE * sizeof(char));
    short *short_array = malloc(ARRAY_SIZE * sizeof(short));
    long *long_array = malloc(ARRAY_SIZE * sizeof(long));
    double *double_array = malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Initialize DataPacket */
        data[i].id = i;
        for (int j = 0; j < 8; j++) {
            data[i].values[j] = (double)rand() / RAND_MAX;
        }
        for (int j = 0; j < 4; j++) {
            data[i].floats[j] = (float)rand() / RAND_MAX;
        }
        data[i].timestamps[0] = rand();
        data[i].timestamps[1] = rand();
        
        /* Initialize other arrays */
        int_array[i] = rand() % 1000;
        float_a[i] = (float)rand() / RAND_MAX;
        float_b[i] = (float)rand() / RAND_MAX;
        char_array[i] = rand() % 256;
        short_array[i] = rand() % 65536;
        long_array[i] = rand();
        double_array[i] = (double)rand() / RAND_MAX;
    }
    
    long long total_checksum = 0;
    
    /* Warm-up iterations */
    for (int iter = 0; iter < 10; iter++) {
        test_nested_loops(data, 1000);
    }
    
    /* Memory barrier between tests */
    asm volatile("" ::: "memory");
    
    /* Run all tests multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        test_nested_loops(data, ARRAY_SIZE / 4);
        total_checksum += global_counter;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG */
        int cfg_result = test_complex_cfg(int_array, ARRAY_SIZE);
        total_checksum += cfg_result;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Many arguments */
        int args_result = test_many_args(
            iter, iter+1, iter+2, iter+3, iter+4,
            iter+5, iter+6, iter+7, iter+8, iter+9, iter+10
        );
        total_checksum += args_result;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Vector operations */
        test_vector_ops(float_a, float_b, float_c, ARRAY_SIZE);
        total_checksum += (int)float_c[ARRAY_SIZE-1];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 5: Mixed types */
        double mixed_result = test_mixed_types(
            char_array, short_array, int_array,
            long_array, float_a, double_array,
            ARRAY_SIZE / 10
        );
        total_checksum += (long long)mixed_result;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        test_pointer_aliasing(int_array, 
                             int_array + ARRAY_SIZE/2,
                             int_array + ARRAY_SIZE/4,
                             ARRAY_SIZE / 20);
        total_checksum += global_counter;
        
        /* Shuffle data periodically */
        if (iter % 10 == 0) {
            for (int i = 0; i < ARRAY_SIZE / 100; i++) {
                int idx = rand() % ARRAY_SIZE;
                int_array[idx] = rand() % 1000;
            }
        }
    }
    
    /* Final checksum calculation */
    long long final_checksum = total_checksum;
    for (int i = 0; i < ARRAY_SIZE; i += 100) {
        final_checksum += data[i].id;
        final_checksum += (int)data[i].values[0];
        final_checksum += int_array[i];
    }
    
    printf("Final checksum: %lld\n", final_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(float_a);
    free(float_b);
    free(float_c);
    free(char_array);
    free(short_array);
    free(long_array);
    free(double_array);
    
    return 0;
}
