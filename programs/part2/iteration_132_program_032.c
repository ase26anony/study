/* test_mcf_coverage.c - Program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_acc = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    volatile int i, j, k, l;  /* Force register pressure */
    int sum = 0;
    int prod = 1;
    int diff = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    
    /* Complex loop structure creating many basic blocks */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i];
        if (tmp1 & 1) {
            for (j = 0; j < size / 8; j++) {
                tmp2 = data[j] * tmp1;
                if (tmp2 > 1000) {
                    for (k = 0; k < size / 16; k++) {
                        tmp3 = data[k] + tmp2;
                        sum += tmp3;
                        if (tmp3 % 7 == 0) {
                            for (l = 0; l < size / 32; l++) {
                                tmp4 = data[l] - tmp3;
                                prod *= (tmp4 & 0xFF);
                                diff -= tmp4;
                            }
                        } else {
                            tmp5 = tmp3 * 3;
                            prod /= (tmp5 | 1);
                        }
                    }
                } else {
                    sum -= tmp2;
                }
            }
        } else {
            prod += tmp1;
        }
        
        /* Early return in some iterations */
        if (i == size / 8) {
            return;
        }
    }
    
    global_acc += sum + prod + diff;
}

/* Function 2: Complex switch statement with fall-through */
int test_complex_switch(int value, int *data, int size) {
    int result = 0;
    volatile int i;
    
    switch (value % SWITCH_CASES) {
        case 0:
            result = data[0] * 2;
            /* Fall through */
        case 1:
            result += data[1] / 3;
            break;
        case 2:
            result = data[2] << 1;
            /* Fall through */
        case 3:
        case 4:
            result ^= data[3];
            break;
        case 5:
            for (i = 0; i < size / 2; i++) {
                result += data[i] * i;
            }
            break;
        case 6:
            result = ~data[4];
            /* Fall through */
        case 7:
            result |= data[5];
            break;
        case 8:
            result = data[6] & 0xAA;
            /* Fall through */
        case 9:
            result <<= 2;
            break;
        case 10:
            result = data[7] >> 3;
            /* Fall through */
        case 11:
        case 12:
            result += data[8] * data[9];
            break;
        case 13:
            result = -data[10];
            break;
        case 14:
            result = abs(data[11]);
            break;
        default:
            result = 0xDEADBEEF;
    }
    
    /* Multiple returns create complex CFG */
    if (result < 0) {
        return result * 2;
    } else if (result > 1000000) {
        return result / 2;
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_register_pressure(int *data, int size) {
    int a, b, c, d, e, f, g, h;
    volatile int i;
    
    for (i = 0; i < size / 10; i++) {
        /* Force use of specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %%ebx, %0\n\t"
            : "=r" (b)
            : "r" (data[i + 1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl %%ecx, %0\n\t"
            : "=r" (c)
            : "r" (data[i + 2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "orl %%edx, %0\n\t"
            : "=r" (d)
            : "r" (data[i + 3])
            : "%edx", "memory"
        );
        
        /* Use all computed values to keep them alive */
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (data[i] | 1);
        
        /* Memory barrier to force spills */
        asm volatile("" ::: "memory");
        
        data[i] = h;
    }
}

/* Function 4: Mixed data types stressing register classes */
double test_mixed_types(double *ddata, float *fdata, 
                        int *idata, short *sdata, char *cdata, int size) {
    double dsum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    short ssum = 0;
    char csum = 0;
    volatile int i;
    
    for (i = 0; i < size; i++) {
        /* Mix operations on different types */
        dsum += ddata[i] * 1.5;
        fsum += fdata[i] / 2.0f;
        isum += idata[i] << 1;
        ssum += sdata[i] * 3;
        csum += cdata[i] - 5;
        
        /* Complex expression mixing types */
        ddata[i] = dsum + (double)fsum + (double)isum + 
                   (double)ssum + (double)csum;
        
        /* Conditional that creates control flow */
        if (i % 7 == 0) {
            fdata[i] = (float)dsum * 0.25f;
            idata[i] = (int)fsum ^ 0x55;
        } else if (i % 3 == 0) {
            sdata[i] = (short)(isum % 256);
            cdata[i] = (char)(ssum & 0xFF);
        }
    }
    
    return dsum + fsum + isum + ssum + csum;
}

/* Function 5: Many function arguments for register/stack pressure */
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int r1 = a1 * a2 + a3;
    int r2 = a4 - a5 * a6;
    int r3 = a7 ^ a8 | a9;
    int r4 = a10 << (a11 & 3);
    int r5 = a12 >> (a13 % 4);
    int r6 = a14 + a15;
    
    /* Nested conditionals */
    if (r1 > r2) {
        if (r3 < r4) {
            return r5 + r6;
        } else {
            return r5 - r6;
        }
    } else if (r2 > r3) {
        if (r4 > r5) {
            return r1 * r6;
        } else {
            return r1 / (r6 | 1);
        }
    }
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

/* Function 6: Pointer aliasing to prevent optimization */
void test_pointer_aliasing(int *ptr1, int *ptr2, int *ptr3, int size) {
    volatile int i;
    int *aliases[3] = {ptr1, ptr2, ptr3};
    
    /* Create complex pointer access pattern */
    for (i = 0; i < size; i++) {
        *aliases[i % 3] += i;
        aliases[(i + 1) % 3][0] -= *aliases[i % 3];
        aliases[(i + 2) % 3][size - i - 1] *= 2;
        
        /* Indirect jump simulation */
        switch (i % 5) {
            case 0: ptr1[i] = ptr2[i] + ptr3[i]; break;
            case 1: ptr2[i] = ptr1[i] - ptr3[i]; break;
            case 2: ptr3[i] = ptr1[i] * ptr2[i]; break;
            case 3: ptr1[i] = ptr2[i] / (ptr3[i] | 1); break;
            case 4: ptr2[i] = ptr3[i] ^ ptr1[i]; break;
        }
    }
}

/* Main function with warm-up and verification */
int main() {
    int i, j;
    uint64_t checksum = 0;
    
    /* Allocate and initialize large arrays with different types */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        double_data[i] = (double)(rand() % 1000) / 3.14;
        float_data[i] = (float)(rand() % 1000) / 2.71f;
        short_data[i] = (short)(rand() % 1000);
        char_data[i] = (char)(rand() % 256);
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up phase for profile feedback */
    for (j = 0; j < ITERATIONS / 10; j++) {
        test_nested_loops(int_data, ARRAY_SIZE / 4);
        asm volatile("" ::: "memory");  /* Prevent optimization across calls */
    }
    
    /* Main test iterations */
    for (j = 0; j < ITERATIONS; j++) {
        /* Call each test function multiple times */
        test_nested_loops(int_data, ARRAY_SIZE);
        
        int switch_result = test_complex_switch(j, int_data, ARRAY_SIZE);
        checksum += switch_result;
        
        test_asm_register_pressure(int_data, ARRAY_SIZE);
        
        double mixed_result = test_mixed_types(double_data, float_data,
                                             int_data, short_data, char_data,
                                             ARRAY_SIZE / 2);
        checksum += (uint64_t)mixed_result;
        
        int args_result = test_many_args(
            j, j+1, j+2, j+3, j+4, j+5, j+6, j+7, j+8, j+9,
            j+10, j+11, j+12, j+13, j+14, j+15
        );
        checksum += args_result;
        
        test_pointer_aliasing(int_data, 
                             int_data + ARRAY_SIZE/3, 
                             int_data + 2*ARRAY_SIZE/3,
                             ARRAY_SIZE/4);
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final computation for verification */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += int_data[i];
        checksum += (uint64_t)double_data[i];
        checksum += (uint64_t)float_data[i];
        checksum += short_data[i];
        checksum += char_data[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    printf("Global accumulator: %f\n", global_acc);
    
    /* Cleanup */
    free(int_data);
    free(double_data);
    free(float_data);
    free(short_data);
    free(char_data);
    
    return 0;
}
