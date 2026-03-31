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
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    double dtmp1, dtmp2, dtmp3;
    
    /* Complex expression with many intermediate values */
    for (i = 0; i < size / 10; i++) {
        for (j = 0; j < 5; j++) {
            tmp1 = data[i * 10 + j];
            for (k = 0; k < 3; k++) {
                tmp2 = data[i * 10 + j + k];
                for (l = 0; l < 2; l++) {
                    tmp3 = data[i * 10 + j + k + l];
                    tmp4 = tmp1 * tmp2 + tmp3;
                    for (m = 0; m < 2; m++) {
                        tmp5 = tmp4 * (tmp1 + tmp2 - tmp3) >> m;
                        sum1 += tmp5;
                        dtmp1 = (double)tmp5 * 1.5;
                        dtmp2 = dtmp1 * 0.75;
                        dtmp3 = dtmp2 / 1.25;
                        sum2 += (int)(dtmp3);
                    }
                    sum3 += tmp4;
                }
                sum4 += tmp2 * tmp1;
            }
            sum5 += tmp1 * j;
        }
    }
    
    /* Force all sums to be used */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4), "r"(sum5));
    global_sum += sum1 + sum2 + sum3 + sum4 + sum5;
}

/* Function 2: Complex control flow with many basic blocks */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Nested if-else chains with early returns */
        if (data[i] < 0) {
            if (data[i] < -1000) {
                if (data[i] < -5000) {
                    result -= 5;
                    continue;
                } else {
                    result -= 2;
                }
            } else if (data[i] < -500) {
                result -= 1;
                goto skip_positive;  /* Create irreducible flow */
            }
        } else if (data[i] > 0) {
            if (data[i] > 1000) {
                if (data[i] > 5000) {
                    result += 5;
                    if (result > 10000) return result;  /* Early return */
                } else {
                    result += 2;
                }
            } else if (data[i] > 500) {
                result += 1;
            } else {
                result += data[i] & 1;
            }
        }
        
    skip_positive:
        
        /* Switch statement with many cases */
        switch (data[i] % 13) {
            case 0: result ^= 0xFF; break;
            case 1: result |= 0xAA; break;
            case 2: result &= 0x55; break;
            case 3: result <<= 1; break;
            case 4: result >>= 2; break;
            case 5: result = ~result; break;
            case 6: result += i; break;
            case 7: result -= i; break;
            case 8: result *= 2; break;
            case 9: result /= 2; break;
            case 10: result %= 256; break;
            case 11: result = (result << 4) | (result >> 4); break;
            case 12: result = result ^ i; break;
            default: break;
        }
        
        /* Loop with break at different nesting levels */
        for (j = 0; j < 5; j++) {
            if (result > 1000000) {
                break;  /* Break from inner loop */
            }
            if (j == 3 && (result & 1)) {
                continue;  /* Skip iteration */
            }
            result += j * data[i];
            
            if (result < 0 && j == 2) {
                goto outer_continue;  /* Jump to outer loop */
            }
        }
        
        /* Computed goto simulation */
        static void *labels[] = {
            &&label0, &&label1, &&label2, &&label3, &&label4
        };
        int idx = data[i] % 5;
        goto *labels[idx];
        
    label0:
        result += 10;
        goto end_switch;
    label1:
        result -= 10;
        goto end_switch;
    label2:
        result *= 3;
        goto end_switch;
    label3:
        result /= 3;
        goto end_switch;
    label4:
        result ^= 0xCC;
        goto end_switch;
    end_switch:
        
    outer_continue:
        /* Empty statement to have a target for goto */
        ;
    }
    
    return result;
}

/* Function 3: Inline assembly with explicit register constraints */
void test_inline_asm(int *data, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    long la, lb, lc;
    double da, db, dc;
    
    for (i = 0; i < size; i += 8) {
        /* Compete for EAX/RAX register */
        a = data[i];
        b = data[i + 1];
        c = data[i + 2];
        d = data[i + 3];
        e = data[i + 4];
        f = data[i + 5];
        g = data[i + 6];
        h = data[i + 7];
        
        /* Fixed register constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (a), "r" (b)
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (c)
            : "r" (c), "r" (d)
            : "%ebx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %2, %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (la)
            : "r" ((long)a), "r" ((long)c)
            : "%rax", "%rbx", "memory"
        );
        
        /* Compete for XMM registers */
        da = (double)a;
        db = (double)b;
        dc = (double)c;
        
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=x" (da)
            : "x" (da), "x" (db)
            : "%xmm0", "memory"
        );
        
        asm volatile (
            "movsd %1, %%xmm1\n\t"
            "mulsd %2, %%xmm1\n\t"
            "movsd %%xmm1, %0\n\t"
            : "=x" (db)
            : "x" (db), "x" (dc)
            : "%xmm1", "memory"
        );
        
        /* Force spills with memory clobber */
        asm volatile ("" : : : "memory");
        
        /* Use all variables to prevent elimination */
        data[i] = a + c + (int)la + (int)da + (int)db;
    }
}

/* Function 4: Variable scoping that extends live ranges */
void test_live_ranges(int *data, int size) {
    int var1, var2, var3, var4, var5, var6, var7, var8, var9, var10;
    int *ptr1, *ptr2, *ptr3;
    volatile int *volatile_ptr;
    
    /* Declare at function scope, use in nested blocks */
    var1 = data[0];
    var2 = data[1];
    var3 = data[2];
    var4 = data[3];
    var5 = data[4];
    
    ptr1 = &var1;
    ptr2 = &var2;
    ptr3 = &var3;
    volatile_ptr = &var4;
    
    {
        int i;
        for (i = 0; i < size / 2; i++) {
            /* Use variables declared outside */
            *ptr1 += data[i];
            *ptr2 -= data[size - i - 1];
            
            /* Deeply nested block */
            {
                int j;
                for (j = 0; j < 10; j++) {
                    var6 = var1 * j;
                    var7 = var2 + j;
                    
                    {
                        int k;
                        for (k = 0; k < 5; k++) {
                            var8 = var6 << k;
                            var9 = var7 >> k;
                            *volatile_ptr = var8 ^ var9;
                        }
                    }
                }
            }
            
            /* Pointer aliasing */
            *ptr3 = *ptr1 + *ptr2;
            var10 = *ptr3 * i;
            
            /* Force memory access */
            *volatile_ptr = var10;
        }
    }
    
    /* Use all variables at the end */
    data[0] = var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Function 5: Mixed data types stressing register classes */
double test_mixed_types(int *idata, float *fdata, double *ddata, char *cdata, 
                        short *sdata, long *ldata, int size) {
    int i;
    double total = 0.0;
    char c1, c2, c3;
    short s1, s2, s3;
    int i1, i2, i3;
    long l1, l2, l3;
    float f1, f2, f3;
    double d1, d2, d3;
    
    for (i = 0; i < size; i++) {
        /* Mixed type operations */
        c1 = cdata[i];
        c2 = cdata[size - i - 1];
        c3 = c1 + c2;
        
        s1 = sdata[i];
        s2 = sdata[size - i - 1];
        s3 = s1 - s2;
        
        i1 = idata[i];
        i2 = idata[size - i - 1];
        i3 = i1 * i2;
        
        l1 = ldata[i];
        l2 = ldata[size - i - 1];
        l3 = l1 / (l2 ? l2 : 1);
        
        f1 = fdata[i];
        f2 = fdata[size - i - 1];
        f3 = f1 + f2;
        
        d1 = ddata[i];
        d2 = ddata[size - i - 1];
        d3 = d1 * d2;
        
        /* Complex expression mixing all types */
        total += (double)c3 + (double)s3 + (double)i3 + (double)l3 + (double)f3 + d3;
        
        /* Type conversions */
        idata[i] = (int)c3 + (int)s3 + (int)f3;
        fdata[i] = (float)c3 + (float)s3 + (float)i3;
        ddata[i] = (double)c3 + (double)s3 + (double)i3 + (double)l3;
    }
    
    return total;
}

/* Function 6: Many function arguments */
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Force register/stack pressure for arguments */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
              a11 + a12 + a13 + a14 + a15;
    
    /* Complex computation using all arguments */
    sum = (sum * a1) / (a2 ? a2 : 1);
    sum += (a3 << 3) | (a4 >> 2);
    sum ^= a5 & a6;
    sum |= a7 ^ a8;
    sum &= ~(a9 | a10);
    sum += (a11 * a12) - (a13 / (a14 ? a14 : 1)) + (a15 % 256);
    
    return sum;
}

/* Function 7: Vector-like operations */
void test_vector_ops(int *data, int size) {
    int i;
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int w0, w1, w2, w3, w4, w5, w6, w7;
    
    for (i = 0; i < size; i += 8) {
        /* Load 8 values (simulating SIMD) */
        v0 = data[i];
        v1 = data[i + 1];
        v2 = data[i + 2];
        v3 = data[i + 3];
        v4 = data[i + 4];
        v5 = data[i + 5];
        v6 = data[i + 6];
        v7 = data[i + 7];
        
        /* Parallel operations */
        w0 = v0 * 3 + 7;
        w1 = v1 * 5 - 2;
        w2 = v2 * 7 + 1;
        w3 = v3 * 11 - 3;
        w4 = v4 * 13 + 5;
        w5 = v5 * 17 - 7;
        w6 = v6 * 19 + 11;
        w7 = v7 * 23 - 13;
        
        /* More operations creating register pressure */
        v0 = w0 + w1;
        v1 = w2 + w3;
        v2 = w4 + w5;
        v3 = w6 + w7;
        v4 = w0 - w1;
        v5 = w2 - w3;
        v6 = w4 - w5;
        v7 = w6 - w7;
        
        /* Store results */
        data[i] = v0 + v4;
        data[i + 1] = v1 + v5;
        data[i + 2] = v2 + v6;
        data[i + 3] = v3 + v7;
        data[i + 4] = v0 - v4;
        data[i + 5] = v1 - v5;
        data[i + 6] = v2 - v6;
        data[i + 7] = v3 - v7;
    }
}

/* Main function */
int main() {
    int i, iter;
    int *data;
    float *fdata;
    double *ddata;
    char *cdata;
    short *sdata;
    long *ldata;
    int result;
    double total = 0.0;
    clock_t start, end;
    
    /* Allocate and initialize arrays */
    data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    fdata = (float*)malloc(ARRAY_SIZE * sizeof(float));
    ddata = (double*)malloc(ARRAY_SIZE * sizeof(double));
    cdata = (char*)malloc(ARRAY_SIZE * sizeof(char));
    sdata = (short*)malloc(ARRAY_SIZE * sizeof(short));
    ldata = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    srand(global_seed);
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 10000 - 5000;
        fdata[i] = (float)(rand() % 10000) / 100.0f;
        ddata[i] = (double)(rand() % 10000) / 100.0;
        cdata[i] = (char)(rand() % 256 - 128);
        sdata[i] = (short)(rand() % 65536 - 32768);
        ldata[i] = (long)rand() * rand();
    }
    
    printf("Starting register pressure stress test...\n");
    start = clock();
    
    /* Warm-up iterations for profile feedback */
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_nested_loops(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function */
        test_nested_loops(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        result = test_complex_cfg(data, ARRAY_SIZE);
        total += result;
        asm volatile("" ::: "memory");
        
        test_inline_asm(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        test_live_ranges(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        total += test_mixed_types(data, fdata, ddata, cdata, sdata, ldata, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Call with many arguments */
        result = test_many_args(
            data[0], data[1], data[2], data[3], data[4],
            data[5], data[6], data[7], data[8], data[9],
            data[10], data[11], data[12], data[13], data[14]
        );
        total += result;
        asm volatile("" ::: "memory");
        
        test_vector_ops(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Update global seed */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    end = clock();
    
    /* Compute final checksum */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= data[i];
        checksum += i;
    }
    
    printf("Test completed.\n");
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Total accumulated: %.2f\n", total);
    printf("Final checksum: %d\n", checksum);
    printf("Global sum: %.2f\n", (double)global_sum);
    
    /* Free memory */
    free(data);
    free(fdata);
    free(ddata);
    free(cdata);
    free(sdata);
    free(ldata);
    
    return 0;
}
