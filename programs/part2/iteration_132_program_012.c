/* mcf_stress_test.c
 * A program designed to stress GCC's register allocator and trigger
 * the min-cost flow solver's debug output for uncovered lines in mcf.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_counter = 0;
volatile double global_sum = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0, sum5 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    double dtmp1, dtmp2, dtmp3;
    
    /* Complex nested loops creating many overlapping live ranges */
    for (i = 0; i < size / 10; i++) {
        tmp1 = data[i] * 2;
        for (j = 0; j < size / 20; j++) {
            tmp2 = data[j] + tmp1;
            for (k = 0; k < size / 40; k++) {
                tmp3 = data[k] * tmp2;
                for (l = 0; l < size / 80; l++) {
                    tmp4 = data[l] - tmp3;
                    for (m = 0; m < size / 160; m++) {
                        tmp5 = data[m] + tmp4;
                        sum1 += tmp5;
                        
                        /* Complex expression with many intermediates */
                        dtmp1 = (double)tmp1 * 1.5;
                        dtmp2 = (double)tmp2 * 2.5;
                        dtmp3 = dtmp1 + dtmp2 + (double)tmp3 * 3.5;
                        global_sum += dtmp3;
                    }
                    sum2 += tmp4;
                }
                sum3 += tmp3;
            }
            sum4 += tmp2;
        }
        sum5 += tmp1;
    }
    
    /* Force all sums to be used */
    global_counter += sum1 + sum2 + sum3 + sum4 + sum5;
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
                    result += data[i] * 6;
                } else {
                    result += data[i] * 7;
                }
            }
        } else if (data[i] < 200) {
            result += data[i] * 8;
        } else if (data[i] < 300) {
            result += data[i] * 9;
        } else if (data[i] < 400) {
            result += data[i] * 10;
        } else {
            result += data[i] * 11;
        }
        
        /* Early returns at different points */
        if (result > 1000000) {
            return result;
        }
        
        if (i % 100 == 0 && result < 0) {
            return -result;
        }
    }
    
    /* Switch statement with many cases */
    switch (result % 20) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
        case 3: result += 4; break;
        case 4: result += 5; break;
        case 5: result += 6; break;
        case 6: result += 7; break;
        case 7: result += 8; break;
        case 8: result += 9; break;
        case 9: result += 10; break;
        case 10: result += 11; break;
        case 11: result += 12; break;
        case 12: result += 13; break;
        case 13: result += 14; break;
        case 14: result += 15; break;
        case 15: result += 16; break;
        case 16: result += 17; break;
        case 17: result += 18; break;
        case 18: result += 19; break;
        case 19: result += 20; break;
        default: result += 100; break;
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(int *data, int size) {
    int i;
    int a, b, c, d, e, f;
    
    for (i = 0; i < size; i += 8) {
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull $2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i+1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "subl $3, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i+2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "xorl $0xFF, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i+3])
            : "%edx", "memory"
        );
        
        asm volatile (
            "movl %1, %%esi\n\t"
            "shrl $2, %%esi\n\t"
            "movl %%esi, %0\n\t"
            : "=r" (e)
            : "r" (data[i+4])
            : "%esi", "memory"
        );
        
        asm volatile (
            "movl %1, %%edi\n\t"
            "andl $0x7F, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r" (f)
            : "r" (data[i+5])
            : "%edi", "memory"
        );
        
        /* Use all results to prevent elimination */
        data[i] = a + b + c + d + e + f;
    }
}

/* Function 4: Mixed data types stressing different register classes */
double test_mixed_types(int *idata, float *fdata, double *ddata, char *cdata, short *sdata, int size) {
    int i;
    double total = 0.0;
    char c1, c2, c3;
    short s1, s2, s3;
    int i1, i2, i3;
    float f1, f2, f3;
    double d1, d2, d3;
    
    for (i = 0; i < size; i++) {
        /* Operations on all different types */
        c1 = cdata[i];
        c2 = cdata[(i + 1) % size];
        c3 = c1 + c2;
        cdata[i] = c3;
        
        s1 = sdata[i];
        s2 = sdata[(i + 2) % size];
        s3 = s1 * s2;
        sdata[i] = s3;
        
        i1 = idata[i];
        i2 = idata[(i + 3) % size];
        i3 = i1 / (i2 + 1);
        idata[i] = i3;
        
        f1 = fdata[i];
        f2 = fdata[(i + 4) % size];
        f3 = f1 - f2;
        fdata[i] = f3;
        
        d1 = ddata[i];
        d2 = ddata[(i + 5) % size];
        d3 = d1 * d2;
        ddata[i] = d3;
        
        /* Complex expression mixing all types */
        total += (double)c3 + (double)s3 + (double)i3 + (double)f3 + d3;
    }
    
    return total;
}

/* Function 5: Many function calls within loops */
int test_many_calls(int *data, int size) {
    int i, sum = 0;
    
    for (i = 0; i < size; i++) {
        /* Multiple function calls creating call-clobbered conflicts */
        sum += abs(data[i]);
        sum += (int)sqrt(fabs((double)data[i]));
        sum += (int)log(fabs((double)data[i]) + 1.0);
        sum += (int)sin((double)data[i] * 0.01);
        sum += (int)cos((double)data[i] * 0.01);
        
        /* Inline asm with memory clobber between calls */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Function 6: Vector-like operations using multiple registers */
void test_vector_ops(int *data, int size) {
    int i;
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int t0, t1, t2, t3, t4, t5, t6, t7;
    
    for (i = 0; i < size; i += 8) {
        /* Load 8 values - simulating vector load */
        v0 = data[i];
        v1 = data[i + 1];
        v2 = data[i + 2];
        v3 = data[i + 3];
        v4 = data[i + 4];
        v5 = data[i + 5];
        v6 = data[i + 6];
        v7 = data[i + 7];
        
        /* Parallel operations on all "vector" elements */
        t0 = v0 * 2 + 1;
        t1 = v1 * 3 + 2;
        t2 = v2 * 4 + 3;
        t3 = v3 * 5 + 4;
        t4 = v4 * 6 + 5;
        t5 = v5 * 7 + 6;
        t6 = v6 * 8 + 7;
        t7 = v7 * 9 + 8;
        
        /* Cross-element dependencies */
        t0 += t1;
        t2 += t3;
        t4 += t5;
        t6 += t7;
        
        t0 += t2;
        t4 += t6;
        
        t0 += t4;
        
        /* Store results */
        data[i] = t0;
        data[i + 1] = t1;
        data[i + 2] = t2;
        data[i + 3] = t3;
        data[i + 4] = t4;
        data[i + 5] = t5;
        data[i + 6] = t6;
        data[i + 7] = t7;
    }
}

/* Function 7: Function with many arguments */
int test_many_args(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j,
                   int k, int l, int m, int n, int o, int p) {
    /* Complex expression using all arguments */
    int result = a * b + c * d - e * f + g * h - i * j + k * l - m * n + o * p;
    result = result * 2 - result / 3 + result % 7;
    
    /* Many intermediate calculations */
    int t1 = a + b + c;
    int t2 = d + e + f;
    int t3 = g + h + i;
    int t4 = j + k + l;
    int t5 = m + n + o;
    int t6 = p + t1 + t2;
    
    result += t1 * t2 - t3 * t4 + t5 * t6;
    
    return result;
}

/* Function 8: Pointer aliasing to prevent optimizations */
void test_pointer_aliasing(int *data, int size) {
    int *ptr1 = data;
    int *ptr2 = data + size / 2;
    int *ptr3 = data + size / 4;
    int *ptr4 = data + 3 * size / 4;
    int i;
    
    /* Create aliasing patterns */
    for (i = 0; i < size / 4; i++) {
        *ptr1 = *ptr2 + *ptr3;
        *ptr2 = *ptr1 - *ptr4;
        *ptr3 = *ptr2 * *ptr1;
        *ptr4 = *ptr3 / (*ptr1 + 1);
        
        ptr1++;
        ptr2++;
        ptr3++;
        ptr4++;
        
        /* Additional computations to increase register pressure */
        int tmp1 = *ptr1;
        int tmp2 = *ptr2;
        int tmp3 = *ptr3;
        int tmp4 = *ptr4;
        
        tmp1 = tmp1 * 2 + tmp2;
        tmp2 = tmp2 * 3 + tmp3;
        tmp3 = tmp3 * 4 + tmp4;
        tmp4 = tmp4 * 5 + tmp1;
        
        *ptr1 = tmp1;
        *ptr2 = tmp2;
        *ptr3 = tmp3;
        *ptr4 = tmp4;
    }
}

int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize large arrays */
    int *data1 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    float *fdata = (float *)malloc(ARRAY_SIZE * sizeof(float));
    double *ddata = (double *)malloc(ARRAY_SIZE * sizeof(double));
    char *cdata = (char *)malloc(ARRAY_SIZE * sizeof(char));
    short *sdata = (short *)malloc(ARRAY_SIZE * sizeof(short));
    
    if (!data1 || !data2 || !fdata || !ddata || !cdata || !sdata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        fdata[i] = (float)(rand() % 1000) / 10.0f;
        ddata[i] = (double)(rand() % 1000) / 10.0;
        cdata[i] = (char)(rand() % 256);
        sdata[i] = (short)(rand() % 65536);
    }
    
    printf("Starting register pressure stress test...\n");
    start = clock();
    
    /* Warm-up phase for profile feedback */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (j = 0; j < WARMUP_ITERATIONS; j++) {
        test_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test phase */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    long long total_checksum = 0;
    
    for (j = 0; j < ITERATIONS; j++) {
        /* Call all test functions in sequence */
        test_nested_loops(data1, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        int cfg_result = test_complex_cfg(data2, ARRAY_SIZE / 2);
        total_checksum += cfg_result;
        asm volatile("" ::: "memory");
        
        test_inline_asm(data1, ARRAY_SIZE / 8);
        asm volatile("" ::: "memory");
        
        double mixed_result = test_mixed_types(data2, fdata, ddata, cdata, sdata, ARRAY_SIZE / 4);
        total_checksum += (long long)mixed_result;
        asm volatile("" ::: "memory");
        
        int call_result = test_many_calls(data1, ARRAY_SIZE / 16);
        total_checksum += call_result;
        asm volatile("" ::: "memory");
        
        test_vector_ops(data2, ARRAY_SIZE / 8);
        asm volatile("" ::: "memory");
        
        int args_result = test_many_args(
            data1[0], data1[1], data1[2], data1[3],
            data1[4], data1[5], data1[6], data1[7],
            data1[8], data1[9], data1[10], data1[11],
            data1[12], data1[13], data1[14], data1[15]
        );
        total_checksum += args_result;
        asm volatile("" ::: "memory");
        
        test_pointer_aliasing(data1, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Process all arrays to compute final checksum */
        for (i = 0; i < ARRAY_SIZE; i += 16) {
            int chunk_sum = 0;
            for (int k = 0; k < 16 && i + k < ARRAY_SIZE; k++) {
                chunk_sum += data1[i + k] + data2[i + k] + 
                           (int)fdata[i + k] + (int)ddata[i + k] +
                           cdata[i + k] + sdata[i + k];
            }
            total_checksum += chunk_sum;
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Final computation to ensure all data is used */
    int final_check = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= data1[i];
        final_check ^= data2[i];
        final_check ^= (int)fdata[i];
        final_check ^= (int)ddata[i];
        final_check ^= cdata[i];
        final_check ^= sdata[i];
    }
    
    total_checksum += final_check;
    
    printf("Test completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %lld\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global sum: %.2f\n", global_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata);
    free(ddata);
    free(cdata);
    free(sdata);
    
    return 0;
}
