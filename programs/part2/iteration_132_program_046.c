/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Complex control flow with many live ranges */
int test1_complex_cfg(int *data, int size) {
    volatile int keep_alive = 0;
    int sum = 0;
    int temp1, temp2, temp3, temp4, temp5;
    int *ptr1 = &temp1, *ptr2 = &temp2;
    
    /* Variables declared at function scope but used in nested blocks */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    for (int idx = 0; idx < size; idx++) {
        a = data[idx];
        b = data[(idx + 1) % size];
        c = data[(idx + 2) % size];
        d = data[(idx + 3) % size];
        
        /* Deeply nested if-else chain */
        if (a > 1000) {
            e = a * b;
            if (b < 500) {
                f = c + d;
                if (c > d) {
                    g = e * f;
                    if (d % 2 == 0) {
                        h = g / 2;
                        for (int inner = 0; inner < 5; inner++) {
                            i = h + inner;
                            j = i * a;
                            k = j - b;
                            /* Force register pressure with many live values */
                            asm volatile("" : "+r"(i), "+r"(j), "+r"(k) : : "memory");
                        }
                    } else {
                        h = g * 3;
                        switch (a % 7) {
                            case 0: i = h + 1; break;
                            case 1: i = h + 2; break;
                            case 2: i = h + 3; break;
                            case 3: i = h + 4; break;
                            case 4: i = h + 5; break;
                            case 5: i = h + 6; break;
                            case 6: i = h + 7; break;
                            default: i = h;
                        }
                    }
                } else {
                    g = e - f;
                    for (int loop = 0; loop < 3; loop++) {
                        h = g + loop;
                        i = h * c;
                        j = i / (d + 1);
                        k = j - a;
                        l = k + b;
                        m = l * 2;
                        n = m - c;
                        o = n + d;
                        p = o * 3;
                        q = p / 4;
                        r = q + e;
                        s = r - f;
                        t = s * g;
                        /* All variables alive here */
                        keep_alive = a + b + c + d + e + f + g + h + i + j + 
                                    k + l + m + n + o + p + q + r + s + t;
                    }
                }
            } else {
                f = c - d;
                /* Early return creates complex CFG */
                if (b > 750) return sum + f;
            }
        } else {
            e = a + b;
            /* Computed goto-like structure */
            switch (a % 10) {
                case 0: f = e * 2; continue;
                case 1: f = e * 3; break;
                case 2: f = e * 4; continue;
                case 3: f = e * 5; break;
                case 4: f = e * 6; continue;
                case 5: f = e * 7; break;
                case 6: f = e * 8; continue;
                case 7: f = e * 9; break;
                case 8: f = e * 10; continue;
                case 9: f = e * 11; break;
            }
        }
        
        /* Complex expression with many intermediates */
        temp1 = a * b + c * d - e * f + g * h;
        temp2 = temp1 * i - j * k + l * m - n * o;
        temp3 = temp2 / (p + 1) * (q + 2) - (r * 3) + (s * 4);
        temp4 = temp3 * t - a * b + c * d - e * f;
        temp5 = temp4 + g * h - i * j + k * l;
        
        sum += temp5;
        
        /* Pointer aliasing prevents optimizations */
        *ptr1 = *ptr2 + temp3;
        *ptr2 = *ptr1 - temp4;
    }
    
    return sum;
}

/* Function with inline assembly forcing specific registers */
double test2_asm_pressure(double *data, int size) {
    double result = 0.0;
    double temp[8];
    
    for (int i = 0; i < size; i += 8) {
        /* Force use of specific xmm registers */
        double x0, x1, x2, x3, x4, x5, x6, x7;
        
        asm volatile(
            "movsd (%1), %%xmm0\n\t"
            "movsd 8(%1), %%xmm1\n\t"
            "movsd 16(%1), %%xmm2\n\t"
            "movsd 24(%1), %%xmm3\n\t"
            "movsd 32(%1), %%xmm4\n\t"
            "movsd 40(%1), %%xmm5\n\t"
            "movsd 48(%1), %%xmm6\n\t"
            "movsd 56(%1), %%xmm7\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m"(x0), "=m"(x1), "=m"(x2), "=m"(x3),
              "=m"(x4), "=m"(x5), "=m"(x6), "=m"(x7)
            : "r"(data + i)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "memory"
        );
        
        /* Complex floating point computations */
        temp[0] = x0 * x1 + x2 * x3;
        temp[1] = x4 * x5 - x6 * x7;
        temp[2] = sin(x0) * cos(x1) + tan(x2);
        temp[3] = exp(x3) * log(fabs(x4) + 1.0);
        temp[4] = sqrt(x5 * x5 + x6 * x6);
        temp[5] = pow(x7, 2.5) + pow(x0, 1.5);
        temp[6] = temp[0] * temp[1] - temp[2] * temp[3];
        temp[7] = temp[4] / (temp[5] + 1.0) + temp[6];
        
        /* More inline assembly with register constraints */
        double t0, t1, t2, t3;
        asm volatile(
            "movsd %4, %%xmm8\n\t"
            "movsd %5, %%xmm9\n\t"
            "mulsd %%xmm9, %%xmm8\n\t"
            "addsd %6, %%xmm8\n\t"
            "subsd %7, %%xmm8\n\t"
            "movsd %%xmm8, %0\n\t"
            "movsd %4, %%xmm10\n\t"
            "addsd %5, %%xmm10\n\t"
            "mulsd %6, %%xmm10\n\t"
            "divsd %7, %%xmm10\n\t"
            "movsd %%xmm10, %1\n\t"
            : "=m"(t0), "=m"(t1)
            : "m"(temp[0]), "m"(temp[1]), "m"(temp[2]), "m"(temp[3]), "m"(temp[4]), "m"(temp[5])
            : "xmm8", "xmm9", "xmm10", "memory"
        );
        
        result += t0 + t1 + temp[6] + temp[7];
    }
    
    return result;
}

/* Mixed data types and many function arguments */
long long test3_mixed_types(char *cdata, short *sdata, int *idata, 
                           float *fdata, double *ddata, int size) {
    long long total = 0;
    
    for (int i = 0; i < size; i++) {
        /* Many live values of different types */
        char c1 = cdata[i];
        char c2 = cdata[(i + 1) % size];
        short s1 = sdata[i];
        short s2 = sdata[(i + 2) % size];
        int i1 = idata[i];
        int i2 = idata[(i + 3) % size];
        float f1 = fdata[i];
        float f2 = fdata[(i + 4) % size];
        double d1 = ddata[i];
        double d2 = ddata[(i + 5) % size];
        
        /* Complex type conversions and computations */
        int t1 = (int)c1 * (int)c2 + (int)s1 - (int)s2;
        float t2 = (float)i1 * 1.5f + (float)i2 * 0.5f;
        double t3 = (double)f1 * 2.0 + (double)f2 * 3.0;
        long long t4 = (long long)t1 * (long long)i1 + (long long)s1;
        double t5 = d1 * d2 + t3;
        float t6 = f1 / (f2 + 1.0f) * t2;
        int t7 = (int)(t5 * 100.0) + (int)(t6 * 10.0f);
        long long t8 = t4 + (long long)t7 * (long long)t1;
        
        /* Function call with many arguments stresses calling convention */
        total += process_values(c1, c2, s1, s2, i1, i2, f1, f2, d1, d2,
                               t1, t2, t3, t4, t5, t6, t7, t8, i);
    }
    
    return total;
}

/* Helper function with many parameters */
long long process_values(char c1, char c2, short s1, short s2,
                        int i1, int i2, float f1, float f2,
                        double d1, double d2, int t1, float t2,
                        double t3, long long t4, double t5,
                        float t6, int t7, long long t8, int idx) {
    /* Force spills with many parameters */
    volatile long long result = 0;
    
    result = (long long)c1 + (long long)c2;
    result += (long long)s1 * (long long)s2;
    result += (long long)i1 - (long long)i2;
    result += (long long)(f1 * 100.0f) + (long long)(f2 * 200.0f);
    result += (long long)(d1 * 1000.0) - (long long)(d2 * 500.0);
    result += (long long)t1 * (long long)t2;
    result += (long long)t3 + t4;
    result += (long long)t5 - (long long)t6;
    result += (long long)t7 * t8;
    result += idx;
    
    return result;
}

/* Vector operations with many SIMD registers */
void test4_vector_ops(int *data, int size, int *result) {
    /* Manual vectorization to use many vector registers */
    for (int i = 0; i < size; i += 16) {
        int v0[4], v1[4], v2[4], v3[4];
        int r0[4], r1[4], r2[4], r3[4];
        
        /* Load vectors */
        for (int j = 0; j < 4; j++) {
            v0[j] = data[i + j];
            v1[j] = data[i + j + 4];
            v2[j] = data[i + j + 8];
            v3[j] = data[i + j + 12];
        }
        
        /* Vector computations */
        for (int j = 0; j < 4; j++) {
            r0[j] = v0[j] * v1[j] + v2[j] - v3[j];
            r1[j] = v0[j] + v1[j] * v2[j] - v3[j];
            r2[j] = v0[j] - v1[j] + v2[j] * v3[j];
            r3[j] = v0[j] * v2[j] + v1[j] - v3[j];
            
            /* More complex operations */
            r0[j] = (r0[j] << 3) | (r1[j] >> 2);
            r1[j] = (r1[j] << 2) ^ (r2[j] >> 1);
            r2[j] = (r2[j] << 1) & (r3[j] >> 3);
            r3[j] = (r3[j] << 4) | (r0[j] & 0xF);
            
            /* Cross-vector operations */
            r0[j] += r1[j] * r2[j] - r3[j];
            r1[j] -= r0[j] + r2[j] * r3[j];
            r2[j] *= r0[j] - r1[j] + r3[j];
            r3[j] /= (r0[j] + r1[j] - r2[j] + 1);
        }
        
        /* Store results */
        for (int j = 0; j < 4; j++) {
            result[i + j] = r0[j];
            result[i + j + 4] = r1[j];
            result[i + j + 8] = r2[j];
            result[i + j + 12] = r3[j];
        }
    }
}

/* Irreducible control flow with gotos */
int test5_irreducible_cfg(int *data, int size) {
    int sum = 0;
    int state = 0;
    
    for (int i = 0; i < size; i++) {
        int x = data[i];
        
        /* Create irreducible region with gotos */
        if (state == 0) {
            if (x % 3 == 0) goto label_a;
            else if (x % 3 == 1) goto label_b;
            else goto label_c;
        } else if (state == 1) {
            if (x % 4 == 0) goto label_d;
            else if (x % 4 == 1) goto label_e;
            else goto label_f;
        } else {
            if (x % 5 == 0) goto label_g;
            else if (x % 5 == 1) goto label_h;
            else goto label_i;
        }
        
    label_a:
        sum += x * 2;
        state = (state + 1) % 3;
        continue;
        
    label_b:
        sum += x * 3;
        state = (state + 2) % 3;
        if (x > 1000) goto label_d;
        continue;
        
    label_c:
        sum += x * 4;
        state = (state + 1) % 3;
        if (x < 500) goto label_a;
        continue;
        
    label_d:
        sum += x * 5;
        state = (state + 2) % 3;
        if (x % 2 == 0) goto label_g;
        continue;
        
    label_e:
        sum += x * 6;
        state = (state + 1) % 3;
        if (x % 7 == 0) goto label_b;
        continue;
        
    label_f:
        sum += x * 7;
        state = (state + 2) % 3;
        if (x % 11 == 0) goto label_e;
        continue;
        
    label_g:
        sum += x * 8;
        state = (state + 1) % 3;
        if (x % 13 == 0) goto label_c;
        continue;
        
    label_h:
        sum += x * 9;
        state = (state + 2) % 3;
        if (x % 17 == 0) goto label_f;
        continue;
        
    label_i:
        sum += x * 10;
        state = (state + 1) % 3;
        if (x % 19 == 0) goto label_h;
        continue;
    }
    
    return sum;
}

int main() {
    /* Allocate and initialize large arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *result_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 10000;
        double_data[i] = (double)(rand() % 10000) / 100.0;
        char_data[i] = (char)(rand() % 256);
        short_data[i] = (short)(rand() % 65536);
        float_data[i] = (float)(rand() % 10000) / 50.0f;
    }
    
    long long total_checksum = 0;
    
    /* Warm-up iterations for profile feedback */
    printf("Starting warm-up iterations...\n");
    for (int warmup = 0; warmup < WARMUP_ITERATIONS; warmup++) {
        int warmup_result = test1_complex_cfg(int_data, ARRAY_SIZE / 10);
        total_checksum += warmup_result;
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
    }
    
    printf("Starting main tests...\n");
    
    /* Run each test multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex CFG with many live ranges */
        int result1 = test1_complex_cfg(int_data, ARRAY_SIZE);
        total_checksum += result1;
        asm volatile("" ::: "memory");
        
        /* Test 2: Inline assembly with register pressure */
        double result2 = test2_asm_pressure(double_data, ARRAY_SIZE);
        total_checksum += (long long)result2;
        asm volatile("" ::: "memory");
        
        /* Test 3: Mixed data types */
        long long result3 = test3_mixed_types(char_data, short_data, int_data,
                                            float_data, double_data, ARRAY_SIZE / 2);
        total_checksum += result3;
        asm volatile("" ::: "memory");
        
        /* Test 4: Vector operations */
        test4_vector_ops(int_data, ARRAY_SIZE, result_data);
        for (int i = 0; i < 100; i++) {
            total_checksum += result_data[i];
        }
        asm volatile("" ::: "memory");
        
        /* Test 5: Irreducible control flow */
        int result5 = test5_irreducible_cfg(int_data, ARRAY_SIZE);
        total_checksum += result5;
        asm volatile("" ::: "memory");
        
        if (iter % 10 == 0) {
            printf("Completed iteration %d\n", iter);
        }
    }
    
    printf("Final checksum: %lld\n", total_checksum);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(char_data);
    free(short_data);
    free(float_data);
    free(result_data);
    
    return 0;
}
