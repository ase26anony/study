/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define NUM_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Complex expression with many intermediate values */
int complex_expression_test(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Declare many variables at function scope to extend live ranges */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Complex expression tree requiring many temporaries */
    t1 = a * b + c;
    t2 = d * e - f;
    t3 = g * h + a;
    t4 = b * c - d;
    t5 = e * f + g;
    t6 = h * a - b;
    t7 = c * d + e;
    t8 = f * g - h;
    t9 = t1 * t2 + t3;
    t10 = t4 * t5 - t6;
    t11 = t7 * t8 + t1;
    t12 = t2 * t3 - t4;
    t13 = t5 * t6 + t7;
    t14 = t8 * t9 - t10;
    t15 = t11 * t12 + t13;
    t16 = t14 * t15 - t9;
    t17 = t10 * t11 + t12;
    t18 = t13 * t14 - t15;
    t19 = t16 * t17 + t18;
    t20 = t19 * t16 - t17;
    
    /* Keep all variables alive across the entire function */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                     "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10),
                     "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15),
                     "r"(t16), "r"(t17), "r"(t18), "r"(t19), "r"(t20));
    
    return t20;
}

/* Deeply nested loops with many live ranges */
void nested_loop_test(int *data, int size) {
    int i, j, k, l, m;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    
    for (i = 0; i < size / 10; i++) {
        tmp1 = data[i];
        for (j = 0; j < size / 20; j++) {
            tmp2 = data[j] + tmp1;
            for (k = 0; k < size / 30; k++) {
                tmp3 = data[k] * tmp2;
                for (l = 0; l < size / 40; l++) {
                    tmp4 = data[l] - tmp3;
                    for (m = 0; m < size / 50; m++) {
                        tmp5 = data[m] ^ tmp4;
                        acc1 += tmp5;
                        acc2 -= tmp5 * 2;
                        acc3 ^= tmp5;
                        acc4 |= tmp5;
                        acc5 &= tmp5;
                        
                        /* Force register pressure with inline asm */
                        asm volatile(
                            "addl %%eax, %%ebx\n\t"
                            "subl %%ecx, %%edx\n\t"
                            "xorl %%esi, %%edi\n\t"
                            : "+a"(acc1), "+b"(acc2), "+c"(acc3), "+d"(acc4), "+S"(acc5)
                            : "D"(tmp5)
                            : "cc"
                        );
                    }
                    /* Early continue to create complex CFG */
                    if (tmp4 < 0) continue;
                    acc1 += tmp4;
                }
                /* Break from inner loop to create more CFG edges */
                if (tmp3 > 1000) break;
                acc2 += tmp3;
            }
            /* Multiple live ranges across loop boundaries */
            asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3), "r"(acc1), "r"(acc2));
        }
        /* Function call within loop creates call-clobbered conflicts */
        acc3 += complex_expression_test(acc1, acc2, tmp1, i, j, k, l);
    }
    
    /* Store results to prevent elimination */
    data[0] = acc1 + acc2 + acc3 + acc4 + acc5;
}

/* Switch statement with many cases for complex CFG */
int switch_cfg_test(int value) {
    int result = 0;
    
    switch (value % NUM_CASES) {
        case 0: {
            int a = value * 2, b = value + 1;
            result = a + b;
            /* Fall through */
        }
        case 1: {
            int c = value * 3, d = value - 1;
            result += c - d;
            break;
        }
        case 2: {
            int e = value << 2, f = value >> 1;
            result = e | f;
            /* Fall through */
        }
        case 3:
        case 4: {
            int g = value * value, h = value % 7;
            result = g / (h + 1);
            break;
        }
        case 5: {
            int i = value + 100, j = value - 50;
            result = i * j;
            /* Fall through */
        }
        case 6:
        case 7:
        case 8: {
            int k = value & 0xFF, l = value | 0xAA;
            result = k ^ l;
            break;
        }
        case 9: {
            int m = value + 999, n = value - 888;
            result = m % (n + 1);
            /* Fall through */
        }
        case 10:
        case 11: {
            int o = value * 11, p = value * 13;
            result = o - p;
            break;
        }
        case 12: {
            int q = value << 3, r = value >> 2;
            result = q & r;
            /* Fall through */
        }
        case 13:
        case 14: {
            int s = value + 777, t = value - 666;
            result = s | t;
            break;
        }
        default: {
            int u = value * 17, v = value * 19;
            result = u + v;
            break;
        }
    }
    
    return result;
}

/* Function with mixed data types to stress different register classes */
double mixed_type_test(char c, short s, int i, long l, float f, double d) {
    /* Force use of different register types */
    double result = 0.0;
    
    /* Integer operations */
    int i1 = c * 2;
    int i2 = s + i;
    long l1 = l * 3;
    long l2 = i1 * i2;
    
    /* Floating point operations */
    float f1 = f * 2.0f;
    float f2 = f1 + (float)i;
    double d1 = d * 3.0;
    double d2 = d1 + (double)l;
    
    /* Mixed operations */
    result = (double)i1 + (double)i2 + (double)l1 + (double)l2 +
             (double)f1 + (double)f2 + d1 + d2;
    
    /* Inline asm with specific register constraints */
    asm volatile(
        "mov %1, %%eax\n\t"
        "mov %2, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(i1)
        : "r"(i1), "r"(i2)
        : "eax", "ebx", "cc"
    );
    
    asm volatile(
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=x"(result)
        : "x"(result), "x"(d2)
        : "xmm0", "xmm1"
    );
    
    return result;
}

/* Function with many arguments to stress register/stack passing */
int many_args_test(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int r1 = a1 + a2 - a3 * a4 / (a5 + 1);
    int r2 = a6 ^ a7 | a8 & a9 << a10;
    int r3 = a11 % a12 + a13 - a14 * a15;
    
    /* Create pointer aliasing to prevent optimization */
    int *p1 = &a1;
    int *p2 = &a2;
    int *p3 = &r1;
    
    *p1 = *p2 + *p3;
    *p2 = *p3 - *p1;
    *p3 = *p1 ^ *p2;
    
    /* Force spills with memory clobber */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3) : "memory");
    
    return r1 + r2 + r3 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
}

/* Function with irreducible control flow using computed goto */
void irreducible_cfg_test(int *data, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int i = 0;
    int sum = 0;
    
    /* Create irreducible loop */
    loop_start:
    if (i >= size) goto loop_end;
    
    /* Jump to different labels based on value */
    int idx = data[i] % 10;
    goto *labels[idx];
    
    label0:
    sum += data[i] * 2;
    i++;
    goto loop_start;
    
    label1:
    sum += data[i] + 1;
    i++;
    goto loop_start;
    
    label2:
    sum += data[i] - 1;
    i++;
    goto loop_start;
    
    label3:
    sum += data[i] << 1;
    i++;
    goto loop_start;
    
    label4:
    sum += data[i] >> 1;
    i++;
    goto loop_start;
    
    label5:
    sum ^= data[i];
    i++;
    goto loop_start;
    
    label6:
    sum |= data[i];
    i++;
    goto loop_start;
    
    label7:
    sum &= data[i];
    i++;
    goto loop_start;
    
    label8:
    sum = sum * 3 + data[i];
    i++;
    goto loop_start;
    
    label9:
    sum = sum / 2 + data[i];
    i++;
    goto loop_start;
    
    loop_end:
    data[0] = sum;
}

/* Vector/SIMD operations to use multiple vector registers */
#ifdef __SSE2__
void simd_register_pressure(float *data, int size) {
    /* Declare many vector variables */
    __m128 v0, v1, v2, v3, v4, v5, v6, v7;
    __m128 v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Load vectors */
    v0 = _mm_load_ps(&data[0]);
    v1 = _mm_load_ps(&data[4]);
    v2 = _mm_load_ps(&data[8]);
    v3 = _mm_load_ps(&data[12]);
    v4 = _mm_load_ps(&data[16]);
    v5 = _mm_load_ps(&data[20]);
    v6 = _mm_load_ps(&data[24]);
    v7 = _mm_load_ps(&data[28]);
    
    /* Complex vector operations */
    v8 = _mm_add_ps(v0, v1);
    v9 = _mm_sub_ps(v2, v3);
    v10 = _mm_mul_ps(v4, v5);
    v11 = _mm_div_ps(v6, v7);
    v12 = _mm_add_ps(v8, v9);
    v13 = _mm_sub_ps(v10, v11);
    v14 = _mm_mul_ps(v12, v13);
    v15 = _mm_add_ps(v14, _mm_set1_ps(1.0f));
    
    /* Store results */
    _mm_store_ps(&data[0], v8);
    _mm_store_ps(&data[4], v9);
    _mm_store_ps(&data[8], v10);
    _mm_store_ps(&data[12], v11);
    _mm_store_ps(&data[16], v12);
    _mm_store_ps(&data[20], v13);
    _mm_store_ps(&data[24], v14);
    _mm_store_ps(&data[28], v15);
}
#endif

/* Main test driver */
int main() {
    /* Initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_data || !float_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed RNG */
    srand(global_seed);
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    int checksum = 0;
    
    /* Warm-up iterations for profile feedback */
    for (int warmup = 0; warmup < 10; warmup++) {
        checksum += complex_expression_test(
            rand() % 100, rand() % 100, rand() % 100,
            rand() % 100, rand() % 100, rand() % 100,
            rand() % 100, rand() % 100
        );
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
    }
    
    /* Run register pressure intensive tests */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops with many live ranges */
        nested_loop_test(int_data, ARRAY_SIZE / (iter + 1));
        checksum += int_data[0];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG with switch */
        for (int i = 0; i < 100; i++) {
            checksum += switch_cfg_test(int_data[i]);
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 3: Mixed data types */
        for (int i = 0; i < 100; i++) {
            double result = mixed_type_test(
                (char)(int_data[i] & 0xFF),
                (short)(int_data[i] & 0xFFFF),
                int_data[i],
                (long)int_data[i] * 2,
                float_data[i],
                (double)float_data[i] * 1.5
            );
            checksum += (int)result;
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 4: Many arguments */
        checksum += many_args_test(
            int_data[0], int_data[1], int_data[2], int_data[3], int_data[4],
            int_data[5], int_data[6], int_data[7], int_data[8], int_data[9],
            int_data[10], int_data[11], int_data[12], int_data[13], int_data[14]
        );
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Test 5: Irreducible CFG */
        irreducible_cfg_test(int_data, ARRAY_SIZE / 10);
        checksum += int_data[0];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        #ifdef __SSE2__
        /* Test 6: SIMD register pressure */
        if (iter % 10 == 0) {
            simd_register_pressure(float_data, ARRAY_SIZE);
            checksum += (int)float_data[0];
        }
        #endif
        
        /* Update global barrier to prevent optimization across iterations */
        global_barrier = checksum;
    }
    
    /* Final verification */
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    
    return 0;
}
