/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 50
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

/* Complex structure to force register pressure */
typedef struct {
    int a, b, c, d;
    double x, y, z;
    char buffer[32];
} DataPacket;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l, m;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i];
        for (j = 0; j < size / 8; j++) {
            tmp2 = data[j];
            for (k = 0; k < size / 16; k++) {
                tmp3 = data[k];
                /* Complex expression with many intermediates */
                sum1 += tmp1 * tmp2 + tmp3;
                sum2 += tmp1 - tmp2 * tmp3;
                sum3 += (tmp1 << 2) | (tmp2 >> 3);
                sum4 += tmp1 ^ tmp2 ^ tmp3;
                
                /* Floating point calculations mixed in */
                acc1 += sin(tmp1 * 0.01) * cos(tmp2 * 0.01);
                acc2 += exp(tmp3 * 0.001);
                
                for (l = 0; l < 8; l++) {
                    tmp4 = data[i + l];
                    tmp5 = data[j + l];
                    tmp6 = data[k + l];
                    
                    /* More register pressure */
                    sum1 += tmp4 * tmp5 - tmp6;
                    sum2 += tmp4 / (tmp5 + 1) + tmp6;
                    
                    for (m = 0; m < 4; m++) {
                        tmp7 = data[i + m];
                        tmp8 = data[j + m];
                        
                        /* Keep all variables alive across loops */
                        sum3 += tmp7 * tmp8 + tmp4 + tmp5 + tmp6;
                        sum4 += (tmp7 << m) | (tmp8 >> m);
                        
                        acc3 += log(fabs(tmp7) + 1.0) * sqrt(tmp8 + 1.0);
                        acc4 += pow(tmp7 * 0.1, tmp8 * 0.01);
                    }
                }
            }
        }
    }
    
    /* Force all results to be used */
    global_counter += sum1 + sum2 + sum3 + sum4;
    global_accumulator += acc1 + acc2 + acc3 + acc4;
}

/* Function 2: Complex control flow with switch statements */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Large switch with fall-through cases */
        switch (data[i] % SWITCH_CASES) {
            case 0:
                result += data[i] * 2;
                /* Fall through */
            case 1:
                result += data[i] >> 1;
                break;
            case 2:
                result += data[i] & 0xFF;
                /* Fall through */
            case 3:
                result += data[i] | 0xAA;
                /* Fall through */
            case 4:
                result += data[i] ^ 0x55;
                break;
            case 5:
                if (data[i] > 1000) {
                    result += data[i] / 3;
                    goto special_case;
                }
                /* Fall through */
            case 6:
                result += data[i] * data[i];
                break;
            case 7:
                result += sqrt(fabs(data[i]));
                break;
            case 8:
                for (j = 0; j < 10; j++) {
                    if (data[i] % (j + 1) == 0) {
                        result += j;
                        continue;
                    }
                    result -= 1;
                }
                break;
            case 9:
                result += (data[i] << 3) | (data[i] >> 5);
                break;
            case 10:
                if (data[i] < 0) {
                    return result;  /* Early return */
                }
                /* Fall through */
            case 11:
                result += data[i] % 77;
                break;
            case 12:
                result += sin(data[i] * 0.01) * 1000;
                break;
            case 13:
                result += cos(data[i] * 0.01) * 1000;
                break;
            case 14:
                result += tan(data[i] * 0.01) * 1000;
                break;
            default:
                result += 1;
        }
        
        /* Label for computed goto */
        special_case:
        if (i % 100 == 0) {
            /* Indirect jump simulation */
            void *labels[] = { &&label1, &&label2, &&label3 };
            goto *labels[data[i] % 3];
            
            label1:
                result += 111;
                continue;
            label2:
                result += 222;
                continue;
            label3:
                result += 333;
                continue;
        }
        
        /* Nested if-else chain */
        if (data[i] < 100) {
            if (data[i] < 50) {
                if (data[i] < 25) {
                    result += 25;
                } else {
                    result += 50;
                }
            } else {
                if (data[i] < 75) {
                    result += 75;
                } else {
                    result += 100;
                }
            }
        } else if (data[i] < 500) {
            result += 500;
        } else if (data[i] < 1000) {
            result += 1000;
        } else {
            result += 2000;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(int *data, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    
    for (i = 0; i < size; i += 8) {
        /* Force specific register allocations */
        a = data[i];
        b = data[i + 1];
        c = data[i + 2];
        d = data[i + 3];
        e = data[i + 4];
        f = data[i + 5];
        g = data[i + 6];
        h = data[i + 7];
        
        /* Compete for specific registers */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (a)
            : "r" (b)
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %0, %%ebx\n\t"
            "imull %1, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "+r" (c)
            : "r" (d)
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %0, %%ecx\n\t"
            "xorl %1, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "+r" (e)
            : "r" (f)
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %0, %%edx\n\t"
            "orl %1, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "+r" (g)
            : "r" (h)
            : "%edx", "memory"
        );
        
        /* More register pressure with XMM registers */
        double x = a * 0.5, y = b * 0.5, z = c * 0.5, w = d * 0.5;
        
        asm volatile (
            "movsd %0, %%xmm0\n\t"
            "addsd %1, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "+x" (x)
            : "x" (y)
            : "%xmm0"
        );
        
        asm volatile (
            "movsd %0, %%xmm1\n\t"
            "mulsd %1, %%xmm1\n\t"
            "movsd %%xmm1, %0\n\t"
            : "+x" (z)
            : "x" (w)
            : "%xmm1"
        );
        
        /* Force spills with memory clobber */
        asm volatile ("" ::: "memory");
        
        data[i] = a + c + e + g;
        data[i + 1] = b + d + f + h;
        data[i + 2] = (int)(x * 100);
        data[i + 3] = (int)(z * 100);
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types(char *cdata, short *sdata, int *idata, 
                        long *ldata, float *fdata, double *ddata,
                        int size) {
    int i;
    double total = 0.0;
    
    for (i = 0; i < size; i++) {
        /* Use all different data types in complex expressions */
        char c = cdata[i];
        short s = sdata[i];
        int i_val = idata[i];
        long l = ldata[i];
        float f = fdata[i];
        double d = ddata[i];
        
        /* Complex expression mixing types */
        total += (double)c * 0.1 +
                 (double)s * 0.01 +
                 (double)i_val * 0.001 +
                 (double)l * 0.0001 +
                 (double)f * 1.5 +
                 d * 2.0;
        
        /* Type conversions that need different registers */
        cdata[i] = (char)(total * 0.01);
        sdata[i] = (short)(total * 0.1);
        idata[i] = (int)(total);
        ldata[i] = (long)(total * 10);
        fdata[i] = (float)(total * 0.5);
        ddata[i] = total * 2.0;
    }
    
    return total;
}

/* Function 5: Many function calls with register arguments */
int test_many_arguments(int a1, int a2, int a3, int a4, int a5,
                        int a6, int a7, int a8, int a9, int a10,
                        int a11, int a12, int a13, int a14, int a15) {
    /* Force register/stack argument passing */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
              a11 + a12 + a13 + a14 + a15;
    
    /* Recursive calls to increase register pressure */
    if (sum > 1000000) {
        return sum;
    }
    
    return test_many_arguments(a1 + 1, a2 + 2, a3 + 3, a4 + 4, a5 + 5,
                               a6 + 6, a7 + 7, a8 + 8, a9 + 9, a10 + 10,
                               a11 + 11, a12 + 12, a13 + 13, a14 + 14, a15 + 15);
}

/* Function 6: Pointer aliasing to prevent optimization */
void test_pointer_aliasing(DataPacket *packets, int count) {
    int i;
    int *alias1, *alias2, *alias3;
    double *dalias1, *dalias2;
    
    for (i = 0; i < count; i++) {
        /* Create multiple aliases to the same data */
        alias1 = &packets[i].a;
        alias2 = &packets[i].b;
        alias3 = &packets[i].c;
        dalias1 = &packets[i].x;
        dalias2 = &packets[i].y;
        
        /* Complex operations with aliases */
        *alias1 = *alias2 + *alias3;
        *alias2 = *alias1 - *alias3;
        *alias3 = *alias1 * *alias2;
        
        *dalias1 = sin(*dalias2) * cos(packets[i].z);
        *dalias2 = exp(*dalias1) * log(fabs(packets[i].z) + 1.0);
        
        /* String operations that need many registers */
        sprintf(packets[i].buffer, "Value: %d %d %d %lf %lf", 
                *alias1, *alias2, *alias3, *dalias1, *dalias2);
    }
}

int main() {
    int i, j;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize large arrays */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *cdata = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *sdata = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long *ldata = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *fdata = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *ddata = (double*)malloc(ARRAY_SIZE * sizeof(double));
    DataPacket *packets = (DataPacket*)malloc((ARRAY_SIZE/10) * sizeof(DataPacket));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 10000;
        data2[i] = rand() % 10000;
        cdata[i] = rand() % 256;
        sdata[i] = rand() % 65536;
        ldata[i] = rand() * 1000L;
        fdata[i] = (float)rand() / RAND_MAX * 100.0f;
        ddata[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    for (i = 0; i < ARRAY_SIZE/10; i++) {
        packets[i].a = rand() % 1000;
        packets[i].b = rand() % 1000;
        packets[i].c = rand() % 1000;
        packets[i].d = rand() % 1000;
        packets[i].x = (double)rand() / RAND_MAX;
        packets[i].y = (double)rand() / RAND_MAX;
        packets[i].z = (double)rand() / RAND_MAX;
    }
    
    printf("Starting register pressure tests...\n");
    start = clock();
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < 5; j++) {
        test_nested_loops(data1, ARRAY_SIZE/10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (j = 0; j < ITERATIONS; j++) {
        /* Test 1: Nested loops */
        test_nested_loops(data1, ARRAY_SIZE/4);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        int cfg_result = test_complex_cfg(data2, ARRAY_SIZE/2);
        global_counter += cfg_result;
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_inline_asm(data1, ARRAY_SIZE/8);
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed data types */
        double mixed_result = test_mixed_types(cdata, sdata, data1, 
                                              ldata, fdata, ddata, 
                                              ARRAY_SIZE/16);
        global_accumulator += mixed_result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        int arg_result = test_many_arguments(1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                            11, 12, 13, 14, 15);
        global_counter += arg_result % 1000;
        asm volatile("" ::: "memory");
        
        /* Test 6: Pointer aliasing */
        test_pointer_aliasing(packets, ARRAY_SIZE/100);
        asm volatile("" ::: "memory");
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute final checksum */
    unsigned long long checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i] + cdata[i] + sdata[i];
        checksum += (unsigned long long)ldata[i];
        checksum += (unsigned long long)fdata[i];
        checksum += (unsigned long long)ddata[i];
    }
    
    for (i = 0; i < ARRAY_SIZE/10; i++) {
        checksum += packets[i].a + packets[i].b + packets[i].c + packets[i].d;
        checksum += (unsigned long long)packets[i].x;
        checksum += (unsigned long long)packets[i].y;
        checksum += (unsigned long long)packets[i].z;
        for (j = 0; j < 32; j++) {
            checksum += packets[i].buffer[j];
        }
    }
    
    checksum += global_counter + (unsigned long long)global_accumulator;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %.6f\n", global_accumulator);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(cdata);
    free(sdata);
    free(ldata);
    free(fdata);
    free(ddata);
    free(packets);
    
    return 0;
}
