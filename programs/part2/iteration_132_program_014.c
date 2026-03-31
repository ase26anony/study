/* test_mcf_coverage.c
 * Designed to trigger GCC's min-cost flow solver debug output
 * Compile with: gcc -O3 -fdump-ira-all -fdump-ira-details -fdump-rtl-all -fno-omit-frame-pointer -dA -dp -dD -dP -dR -da test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Memory barrier to prevent cross-function optimization */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Complex expression with many intermediate values */
static inline uint64_t complex_expression(uint64_t a, uint64_t b, uint64_t c, 
                                          uint64_t d, uint64_t e, uint64_t f) {
    /* Force many temporary registers */
    uint64_t t1 = a * b + c;
    uint64_t t2 = d ^ e ^ f;
    uint64_t t3 = (a << 3) | (b >> 5);
    uint64_t t4 = t1 * t2 + t3;
    uint64_t t5 = (c * d) / (e + 1);
    uint64_t t6 = t4 ^ t5;
    uint64_t t7 = (t6 * 0x5DEECE66D + 0xB) & 0xFFFFFFFFFFFF;
    uint64_t t8 = t7 * t1 + t2 * t3;
    uint64_t t9 = t8 | (t4 << 16) | (t5 >> 16);
    
    return t9 * t6 + t7 * t8 + t1 * t2 * t3 * t4 * t5;
}

/* Function with deeply nested loops and many live ranges */
void test1_register_pressure_intensive(int* data, int size) {
    volatile int keep_alive1, keep_alive2, keep_alive3; /* Force register retention */
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    
    /* Variables declared at function scope but used in nested blocks */
    int outer_var1, outer_var2, outer_var3, outer_var4, outer_var5;
    int* ptr1 = &outer_var1;
    int* ptr2 = &outer_var2;
    
    /* Deeply nested loops creating many live ranges */
    for (i = 0; i < size / 100; i++) {
        outer_var1 = data[i] * 3;
        for (j = 0; j < 50; j++) {
            outer_var2 = outer_var1 + j * 7;
            for (k = 0; k < 25; k++) {
                tmp1 = outer_var2 * k;
                tmp2 = tmp1 + data[k];
                for (l = 0; l < 10; l++) {
                    tmp3 = tmp2 * l;
                    tmp4 = tmp3 - data[l];
                    for (m = 0; m < 5; m++) {
                        /* Complex expression using all temporaries */
                        tmp5 = complex_expression(tmp1, tmp2, tmp3, tmp4, m, l);
                        tmp6 = tmp5 * data[m] + tmp1;
                        tmp7 = tmp6 ^ tmp2 | tmp3;
                        tmp8 = tmp7 * tmp4 / (tmp5 + 1);
                        tmp9 = tmp8 + tmp6 - tmp7;
                        tmp10 = tmp9 * tmp5;
                        
                        /* Force all values to be live */
                        sum1 += tmp1;
                        sum2 += tmp2;
                        sum3 += tmp3;
                        sum4 += tmp4;
                        sum5 += tmp5 + tmp6 + tmp7 + tmp8 + tmp9 + tmp10;
                    }
                }
            }
        }
        /* Use pointers to prevent dead store elimination */
        *ptr1 = sum1;
        *ptr2 = sum2;
        keep_alive1 = *ptr1;
        keep_alive2 = *ptr2;
    }
    
    /* Use volatile to force memory writes */
    keep_alive3 = sum1 + sum2 + sum3 + sum4 + sum5;
    printf("Test1 checksum: %d\n", keep_alive3);
}

/* Function with complex control flow creating irreducible CFG */
int test2_complex_control_flow(int* data, int size) {
    int result = 0;
    int i, state = 0;
    
    /* Switch with many cases and fall-through */
    for (i = 0; i < size; i++) {
        int val = data[i] % 20;
        
        switch (val) {
            case 0: result += data[i] * 2; /* Fall through */
            case 1: result += data[i] / 3; break;
            case 2: result += data[i] << 1; /* Fall through */
            case 3: result += data[i] >> 2; /* Fall through */
            case 4: result += data[i] & 0xFF; break;
            case 5: result += data[i] | 0xAA; /* Fall through */
            case 6: result += data[i] ^ 0x55; break;
            case 7: result += ~data[i]; /* Fall through */
            case 8: result += data[i] * data[i]; /* Fall through */
            case 9: result += sqrt(abs(data[i])); break;
            case 10: result += data[i] % 17; /* Fall through */
            case 11: result += data[i] * 3; /* Fall through */
            case 12: result += data[i] / 5; break;
            case 13: result += data[i] << 2; /* Fall through */
            case 14: result += data[i] >> 3; break;
            case 15: result += data[i] & 0xF0; /* Fall through */
            case 16: result += data[i] | 0x0F; break;
            case 17: result += data[i] ^ 0xFF; /* Fall through */
            case 18: result += -data[i]; break;
            case 19: result += data[i] * data[i] / 2; break;
            default: result += 1;
        }
        
        /* Nested if-else with early returns */
        if (result > 1000000) {
            if (state == 0) {
                state = 1;
                continue;
            } else if (state == 1) {
                state = 2;
                break;
            } else {
                return result;
            }
        } else if (result < -1000000) {
            if (i % 2 == 0) {
                continue;
            } else {
                result = 0;
            }
        }
        
        /* Loop with break at different nesting levels */
        for (int j = 0; j < 5; j++) {
            if (j == 2 && result % 3 == 0) {
                break;
            }
            for (int k = 0; k < 3; k++) {
                if (k == 1 && result % 5 == 0) {
                    goto exit_loop;
                }
                result += j * k;
            }
        }
        exit_loop:
        
        /* Computed goto to create irreducible flow */
        static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        int idx = data[i] % 5;
        goto *labels[idx];
        
        L0: result += 1; continue;
        L1: result += 2; continue;
        L2: result += 3; continue;
        L3: result += 4; continue;
        L4: result += 5; continue;
    }
    
    return result;
}

/* Function with inline assembly forcing specific register allocation */
void test3_inline_assembly_conflict(int* data, int size) {
    int i;
    uint64_t rax_val, rbx_val, rcx_val, rdx_val;
    uint64_t r8_val, r9_val, r10_val, r11_val;
    uint64_t r12_val, r13_val, r14_val, r15_val;
    
    for (i = 0; i < size / 100; i++) {
        /* Force competition for RAX */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq $0x12345678, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (rax_val)
            : "r" ((uint64_t)data[i])
            : "%rax", "memory"
        );
        
        /* Force competition for RBX */
        asm volatile (
            "movq %1, %%rbx\n\t"
            "imulq $0x55, %%rbx\n\t"
            "movq %%rbx, %0\n\t"
            : "=r" (rbx_val)
            : "r" ((uint64_t)data[i + 1])
            : "%rbx", "memory"
        );
        
        /* Force competition for multiple registers simultaneously */
        asm volatile (
            "movq %2, %%rcx\n\t"
            "movq %3, %%rdx\n\t"
            "addq %%rcx, %%rdx\n\t"
            "movq %%rdx, %%r8\n\t"
            "subq $0x100, %%r8\n\t"
            "movq %%r8, %0\n\t"
            "movq %%rcx, %1\n\t"
            : "=r" (r8_val), "=r" (rcx_val)
            : "r" ((uint64_t)data[i]), "r" ((uint64_t)data[i + 2])
            : "%rcx", "%rdx", "%r8", "memory"
        );
        
        /* More register pressure with xmm registers */
        double xmm0_val, xmm1_val, xmm2_val, xmm3_val;
        double d1 = data[i] * 1.5;
        double d2 = data[i + 1] * 2.5;
        
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %2, %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "mulsd %3, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (xmm0_val)
            : "m" (d1), "m" (d2), "m" (3.14159)
            : "%xmm0", "%xmm1", "memory"
        );
        
        /* Use all the register values to keep them live */
        rax_val = rax_val * rbx_val + rcx_val;
        r8_val = r8_val ^ rax_val;
        
        /* Memory clobber forces spills */
        asm volatile("" ::: "memory");
    }
    
    printf("Test3 asm values: %lu %lu %lu\n", 
           (unsigned long)rax_val, (unsigned long)rbx_val, (unsigned long)r8_val);
}

/* Function with mixed data types stressing register classes */
void test4_mixed_data_types(int* data, int size) {
    char c1, c2, c3, c4, c5;
    short s1, s2, s3, s4, s5;
    int i1, i2, i3, i4, i5;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    
    /* Vector operations using multiple SIMD registers */
    typedef int v4si __attribute__ ((vector_size (16)));
    typedef float v4sf __attribute__ ((vector_size (16)));
    typedef double v2df __attribute__ ((vector_size (16)));
    
    v4si v1 = {data[0], data[1], data[2], data[3]};
    v4si v2 = {data[4], data[5], data[6], data[7]};
    v4si v3, v4, v5, v6, v7, v8, v9, v10;
    
    v4sf vf1 = {data[0] * 0.1f, data[1] * 0.2f, data[2] * 0.3f, data[3] * 0.4f};
    v4sf vf2 = {data[4] * 0.5f, data[5] * 0.6f, data[6] * 0.7f, data[7] * 0.8f};
    v4sf vf3, vf4, vf5, vf6, vf7, vf8, vf9, vf10;
    
    v2df vd1 = {data[0] * 1.1, data[1] * 1.2};
    v2df vd2 = {data[2] * 1.3, data[3] * 1.4};
    v2df vd3, vd4, vd5, vd6, vd7, vd8, vd9, vd10;
    
    for (int i = 0; i < size / 10; i++) {
        /* Integer vector operations */
        v3 = v1 + v2;
        v4 = v1 * v2;
        v5 = v3 - v4;
        v6 = v1 & v2;
        v7 = v1 | v2;
        v8 = v1 ^ v2;
        v9 = v3 << 2;
        v10 = v4 >> 1;
        
        /* Float vector operations */
        vf3 = vf1 + vf2;
        vf4 = vf1 * vf2;
        vf5 = vf3 - vf4;
        vf6 = vf1 / (vf2 + 1.0f);
        vf7 = __builtin_ia32_sqrtps(vf3);
        vf8 = vf4 * 2.0f;
        vf9 = vf5 + vf6;
        vf10 = vf7 - vf8;
        
        /* Double vector operations */
        vd3 = vd1 + vd2;
        vd4 = vd1 * vd2;
        vd5 = vd3 - vd4;
        vd6 = vd1 / (vd2 + 1.0);
        vd7 = __builtin_ia32_sqrtpd(vd3);
        vd8 = vd4 * 2.0;
        vd9 = vd5 + vd6;
        vd10 = vd7 - vd8;
        
        /* Mix all types together */
        c1 = (char)(v1[0] & 0xFF);
        s1 = (short)(v2[1] & 0xFFFF);
        i1 = v3[2];
        l1 = (long)v4[3];
        f1 = vf5[0];
        d1 = vd6[1];
        
        /* Complex address calculations */
        int idx1 = (c1 * s1 + i1) % size;
        int idx2 = (l1 % 1000 + (int)f1 + (int)d1) % size;
        int idx3 = (idx1 * 3 + idx2 * 7) % size;
        int idx4 = (idx3 ^ 0xABCD) % size;
        
        /* Use all variables to keep them live */
        c2 = c1 + s1 % 256;
        s2 = s1 + i1 % 32768;
        i2 = i1 + l1 % 1000000;
        l2 = l1 + (long)f1;
        f2 = f1 + (float)d1;
        d2 = d1 + (double)l1;
        
        /* Pointer aliasing */
        int* p1 = &i1;
        int* p2 = &i2;
        int* p3 = &data[idx1];
        *p1 = *p2 + *p3;
        *p3 = *p1 - *p2;
        
        /* Update vectors for next iteration */
        v1 = v3 + v5;
        v2 = v4 + v6;
        vf1 = vf3 + vf5;
        vf2 = vf4 + vf6;
        vd1 = vd3 + vd5;
        vd2 = vd4 + vd6;
    }
    
    /* Use all results */
    printf("Test4 mixed: %d %ld %f %lf\n", 
           v1[0] + v2[1], (long)(vf3[2] * 1000), vf4[3], vd5[0]);
}

/* Function with many arguments stressing calling convention */
int __attribute__((noinline)) 
test5_many_arguments(int a1, int a2, int a3, int a4, int a5,
                     int a6, int a7, int a8, int a9, int a10,
                     int a11, int a12, int a13, int a14, int a15,
                     float f1, float f2, float f3, float f4, float f5,
                     double d1, double d2, double d3, double d4, double d5) {
    /* Force all arguments to be used in complex ways */
    int sum = a1 + a2 + a3 + a4 + a5;
    sum += a6 * a7 - a8 / (a9 + 1);
    sum += a10 ^ a11 | a12 & a13;
    sum += a14 << 3 + a15 >> 2;
    
    float fsum = f1 + f2 * f3 - f4 / f5;
    double dsum = d1 * d2 + d3 - d4 / d5;
    
    /* Mixed type computations */
    sum += (int)(fsum * 100);
    sum += (int)(dsum * 1000);
    
    /* Many local variables */
    int t1 = sum * 2, t2 = sum / 3, t3 = sum + 100;
    int t4 = t1 ^ t2, t5 = t3 | t1, t6 = t2 & t3;
    int t7 = t4 * t5, t8 = t6 + t7, t9 = t8 - sum;
    int t10 = t9 << 2, t11 = t10 >> 1, t12 = t11 % 17;
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;
}

/* Main driver that calls all test functions */
int main() {
    int i;
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 10000 - 5000; /* Range: -5000 to 4999 */
    }
    
    printf("Starting MCF coverage test...\n");
    
    /* Warm-up iterations for profile feedback */
    for (i = 0; i < WARMUP_ITERATIONS; i++) {
        test1_register_pressure_intensive(data, ARRAY_SIZE / 10);
        MEMORY_BARRIER();
    }
    
    /* Main test iterations */
    int total_result = 0;
    for (i = 0; i < ITERATIONS; i++) {
        test1_register_pressure_intensive(data, ARRAY_SIZE);
        MEMORY_BARRIER();
        
        int cf_result = test2_complex_control_flow(data, ARRAY_SIZE / 2);
        total_result += cf_result;
        MEMORY_BARRIER();
        
        test3_inline_assembly_conflict(data, ARRAY_SIZE);
        MEMORY_BARRIER();
        
        test4_mixed_data_types(data, ARRAY_SIZE);
        MEMORY_BARRIER();
        
        /* Call function with many arguments */
        int arg_result = test5_many_arguments(
            data[0], data[1], data[2], data[3], data[4],
            data[5], data[6], data[7], data[8], data[9],
            data[10], data[11], data[12], data[13], data[14],
            data[15] * 0.1f, data[16] * 0.2f, data[17] * 0.3f, 
            data[18] * 0.4f, data[19] * 0.5f,
            data[20] * 1.1, data[21] * 1.2, data[22] * 1.3,
            data[23] * 1.4, data[24] * 1.5
        );
        total_result += arg_result;
        MEMORY_BARRIER();
        
        /* Shuffle data to create different patterns */
        if (i % 10 == 0) {
            for (int j = 0; j < ARRAY_SIZE / 100; j++) {
                int idx = rand() % ARRAY_SIZE;
                data[idx] = complex_expression(data[idx], j, i, 
                                              total_result, arg_result, cf_result);
            }
        }
    }
    
    printf("Total checksum: %d\n", total_result);
    printf("Test completed.\n");
    
    free(data);
    return 0;
}
