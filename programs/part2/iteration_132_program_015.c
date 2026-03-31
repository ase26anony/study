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
    
    /* Complex loop structure creating many basic blocks */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i] * 2;
        for (j = i; j < size / 8; j++) {
            tmp2 = data[j] + tmp1;
            if (tmp2 > 1000) {
                for (k = j; k < size / 16; k++) {
                    tmp3 = data[k] - tmp2;
                    sum1 += tmp3;
                    if (tmp3 < 0) {
                        for (l = k; l < size / 32; l++) {
                            tmp4 = data[l] * tmp3;
                            sum2 += tmp4;
                            dtmp1 = sqrt(fabs(tmp4));
                            if (dtmp1 > 50.0) {
                                for (m = l; m < size / 64; m++) {
                                    tmp5 = data[m] / (tmp4 + 1);
                                    sum3 += tmp5;
                                    dtmp2 = sin(tmp5) * cos(tmp5);
                                    sum4 += (int)(dtmp2 * 1000);
                                }
                            }
                        }
                    } else {
                        dtmp3 = log(fabs(tmp3) + 1.0);
                        sum5 += (int)(dtmp3 * 100);
                    }
                }
            }
        }
    }
    
    /* Force all sums to be used */
    asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3), "r"(sum4), "r"(sum5));
    global_sum += sum1 + sum2 + sum3 + sum4 + sum5;
}

/* Function 2: Complex control flow with switch statements */
int test_control_flow(int *data, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        int val = data[i] % 20;
        
        /* Large switch statement creating many basic blocks */
        switch (val) {
            case 0: result += data[i] * 2; break;
            case 1: result += data[i] / 2; break;
            case 2: result += data[i] + data[(i+1)%size]; break;
            case 3: result += data[i] - data[(i+2)%size]; break;
            case 4: result += data[i] * data[(i+3)%size]; break;
            case 5: result += data[i] / (data[(i+4)%size] + 1); break;
            case 6: result += (data[i] << 2) | (data[i] >> 30); break;
            case 7: result += ~data[i]; break;
            case 8: result += data[i] ^ 0xAAAAAAAA; break;
            case 9: result += data[i] & 0x55555555; break;
            case 10: result += data[i] | 0x12345678; break;
            case 11: result += (int)sin(data[i] * 0.01) * 1000; break;
            case 12: result += (int)cos(data[i] * 0.01) * 1000; break;
            case 13: result += (int)tan(data[i] * 0.001) * 100; break;
            case 14: result += (int)log(fabs(data[i]) + 1) * 100; break;
            case 15: result += (int)sqrt(fabs(data[i])) * 10; break;
            case 16: result += data[i] * data[i]; break;
            case 17: result += (data[i] % 100) * (data[i] / 100); break;
            case 18: result += ~(data[i] * 3); break;
            case 19: result += (data[i] << 1) ^ (data[i] >> 1); break;
            default: result += 1; break;
        }
        
        /* Nested if-else chain */
        if (val < 5) {
            for (j = 0; j < 3; j++) {
                if (result % 2 == 0) {
                    result += j * 10;
                } else if (result % 3 == 0) {
                    result -= j * 5;
                } else if (result % 5 == 0) {
                    result ^= j * 7;
                } else {
                    result |= j * 13;
                }
            }
        } else if (val < 10) {
            result = (result << 3) | (result >> 29);
        } else if (val < 15) {
            result = result ^ (result << 1);
        } else {
            result = result + (result * 3) / 2;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(int *data, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Force specific register allocation with inline asm */
        a = data[i];
        b = data[i+1];
        c = data[i+2];
        d = data[i+3];
        e = data[i+4];
        f = data[i+5];
        g = data[i+6];
        h = data[i+7];
        
        /* Multiple asm statements competing for registers */
        asm volatile (
            "addl %%eax, %%ebx\n\t"
            "subl %%ecx, %%edx\n\t"
            "imull %%esi, %%edi\n\t"
            : "+b" (b), "+d" (d), "+D" (h)
            : "a" (a), "c" (c), "S" (g)
            : "cc", "memory"
        );
        
        asm volatile (
            "movl %%eax, %%ecx\n\t"
            "rorl $5, %%ecx\n\t"
            "xorl %%ebx, %%edx\n\t"
            : "=c" (c), "=d" (d)
            : "a" (b), "b" (f), "d" (e)
            : "cc"
        );
        
        asm volatile (
            "xchgl %%eax, %%ebx\n\t"
            "leal (%%eax,%%ecx,2), %%edx\n\t"
            : "=a" (a), "=b" (b), "=d" (d)
            : "a" (c), "b" (d), "c" (h)
            : "cc"
        );
        
        /* Store results back */
        data[i] = a;
        data[i+1] = b;
        data[i+2] = c;
        data[i+3] = d;
        data[i+4] = e;
        data[i+5] = f;
        data[i+6] = g;
        data[i+7] = h;
    }
}

/* Function 4: Mixed data types and vector-like operations */
double test_mixed_types(int *idata, float *fdata, double *ddata, int size) {
    int i;
    double total = 0.0;
    char c1, c2, c3, c4;
    short s1, s2, s3, s4;
    int i1, i2, i3, i4;
    long l1, l2, l3, l4;
    float f1, f2, f3, f4;
    double d1, d2, d3, d4;
    
    for (i = 0; i < size; i += 8) {
        /* Load different data types */
        c1 = (char)idata[i];
        c2 = (char)idata[i+1];
        c3 = (char)idata[i+2];
        c4 = (char)idata[i+3];
        
        s1 = (short)idata[i];
        s2 = (short)idata[i+1];
        s3 = (short)idata[i+2];
        s4 = (short)idata[i+3];
        
        i1 = idata[i];
        i2 = idata[i+1];
        i3 = idata[i+2];
        i4 = idata[i+3];
        
        l1 = (long)idata[i] * 1000;
        l2 = (long)idata[i+1] * 1000;
        l3 = (long)idata[i+2] * 1000;
        l4 = (long)idata[i+3] * 1000;
        
        f1 = fdata[i];
        f2 = fdata[i+1];
        f3 = fdata[i+2];
        f4 = fdata[i+3];
        
        d1 = ddata[i];
        d2 = ddata[i+1];
        d3 = ddata[i+2];
        d4 = ddata[i+3];
        
        /* Complex mixed-type computations */
        total += (c1 * s1) + (c2 * s2) + (c3 * s3) + (c4 * s4);
        total += (i1 * f1) + (i2 * f2) + (i3 * f3) + (i4 * f4);
        total += (l1 * d1) + (l2 * d2) + (l3 * d3) + (l4 * d4);
        
        total += sin(c1) * cos(s1) * tan(f1) * log(d1 + 1.0);
        total += exp(c2 * 0.01) * pow(f2, 2.0) * sqrt(fabs(d2));
        
        /* Type conversions and back */
        idata[i] = (int)(total * 1000);
        fdata[i] = (float)fmod(total, 1000.0);
        ddata[i] = total / 1000.0;
    }
    
    return total;
}

/* Function 5: Function with many arguments */
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int r1 = a1 * a2 + a3 - a4;
    int r2 = a5 / (a6 + 1) * a7;
    int r3 = a8 ^ a9 | a10;
    int r4 = (a11 << 3) | (a12 >> 5);
    int r5 = a13 * a14 - a15;
    
    int sum = r1 + r2 + r3 + r4 + r5;
    
    /* Nested calls to increase register pressure */
    sum += test_many_args(a2, a3, a4, a5, a6, a7, a8, a9, a10, 
                         a11, a12, a13, a14, a15, a1);
    sum += test_many_args(a3, a4, a5, a6, a7, a8, a9, a10, a11,
                         a12, a13, a14, a15, a1, a2);
    
    return sum % 1000000;
}

/* Function 6: Pointer aliasing and volatile variables */
void test_pointer_aliasing(int *data, int size) {
    int i;
    volatile int *volatile ptr1;
    volatile int *volatile ptr2;
    volatile int *volatile ptr3;
    
    int local1, local2, local3, local4, local5;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    
    /* Create pointer aliases */
    ptr1 = (volatile int *)data;
    ptr2 = (volatile int *)(data + size/2);
    ptr3 = (volatile int *)(data + size/4);
    
    for (i = 0; i < size/4; i++) {
        /* Read through volatile pointers */
        local1 = *ptr1;
        local2 = *ptr2;
        local3 = *ptr3;
        
        /* Complex computation keeping many values live */
        tmp1 = local1 * local2;
        tmp2 = local2 * local3;
        tmp3 = local3 * local1;
        tmp4 = tmp1 + tmp2 + tmp3;
        tmp5 = tmp4 * 2 - tmp1;
        
        /* Write back through different pointers */
        *ptr1 = tmp1;
        *ptr2 = tmp2;
        *ptr3 = tmp3;
        
        /* Update pointers with complex addressing */
        ptr1 = (volatile int *)((char *)ptr1 + sizeof(int));
        ptr2 = (volatile int *)((char *)ptr2 - sizeof(int));
        ptr3 = (volatile int *)((char *)ptr3 + (i % 2 ? sizeof(int) : -sizeof(int)));
        
        /* Keep variables alive across loop iterations */
        local4 = tmp4 + local4;  /* Use uninitialized to prevent optimization */
        local5 = tmp5 ^ local5;
    }
    
    /* Force all locals to be used */
    asm volatile("" : : "r"(local1), "r"(local2), "r"(local3), 
                  "r"(local4), "r"(local5), "r"(tmp1), "r"(tmp2), 
                  "r"(tmp3), "r"(tmp4), "r"(tmp5));
}

/* Main test driver */
int main() {
    int i, iter;
    int *data1, *data2;
    float *fdata;
    double *ddata;
    long long total_checksum = 0;
    clock_t start, end;
    
    /* Allocate and initialize data */
    data1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    fdata = (float *)malloc(ARRAY_SIZE * sizeof(float));
    ddata = (double *)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!data1 || !data2 || !fdata || !ddata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 10000;
        data2[i] = rand() % 10000;
        fdata[i] = (float)(rand() % 10000) / 100.0f;
        ddata[i] = (double)(rand() % 10000) / 100.0;
    }
    
    printf("Starting register pressure stress test...\n");
    start = clock();
    
    /* Warm-up phase for profile feedback */
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        test_nested_loops(data1, ARRAY_SIZE / (iter + 1));
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function */
        test_nested_loops(data1, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        int cf_result = test_control_flow(data2, ARRAY_SIZE);
        total_checksum += cf_result;
        asm volatile("" ::: "memory");
        
        test_inline_asm(data1, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        double mt_result = test_mixed_types(data1, fdata, ddata, ARRAY_SIZE);
        total_checksum += (long long)mt_result;
        asm volatile("" ::: "memory");
        
        int ma_result = test_many_args(
            data1[0], data1[1], data1[2], data1[3], data1[4],
            data1[5], data1[6], data1[7], data1[8], data1[9],
            data1[10], data1[11], data1[12], data1[13], data1[14]
        );
        total_checksum += ma_result;
        asm volatile("" ::: "memory");
        
        test_pointer_aliasing(data2, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Process data arrays */
        for (i = 0; i < ARRAY_SIZE; i++) {
            data1[i] = (data1[i] * 3 + data2[i]) / 2;
            data2[i] = (data2[i] * 5 - data1[i]) % 10000;
            fdata[i] = fdata[i] * 1.1f + (float)data1[i] / 1000.0f;
            ddata[i] = ddata[i] * 1.01 + (double)data2[i] / 10000.0;
        }
    }
    
    end = clock();
    
    /* Final checksum computation */
    for (i = 0; i < ARRAY_SIZE; i++) {
        total_checksum += data1[i] + data2[i] + (int)fdata[i] + (int)ddata[i];
    }
    
    printf("Test completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Final checksum: %lld\n", total_checksum);
    printf("Global sum: %f\n", global_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata);
    free(ddata);
    
    return 0;
}
