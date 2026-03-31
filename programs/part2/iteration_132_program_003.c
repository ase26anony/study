/* mcf_test.c - Test program to trigger min-cost flow debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define WARMUP_ITERATIONS 10

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_barrier = 0;

/* Function 1: Deeply nested loops with many live ranges */
int test_complex_loops(int *data, int size) {
    int sum = 0;
    int i, j, k, l, m;
    int temp1, temp2, temp3, temp4, temp5;
    int acc1, acc2, acc3, acc4, acc5;
    
    /* Multiple nested loops creating register pressure */
    for (i = 0; i < size / 10; i++) {
        acc1 = data[i];
        for (j = 0; j < 5; j++) {
            acc2 = acc1 * j;
            for (k = 0; k < 3; k++) {
                acc3 = acc2 + data[k];
                for (l = 0; l < 2; l++) {
                    acc4 = acc3 - data[l];
                    for (m = 0; m < 2; m++) {
                        acc5 = acc4 * data[m];
                        temp1 = acc5 >> 1;
                        temp2 = temp1 & 0xFF;
                        temp3 = temp2 | 0x80;
                        temp4 = temp3 ^ 0x55;
                        temp5 = temp4 << 2;
                        sum += temp5;
                    }
                }
            }
        }
    }
    
    /* Complex expression with many intermediates */
    for (i = 0; i < size; i++) {
        int a = data[i];
        int b = a * 3;
        int c = b + 7;
        int d = c >> 2;
        int e = d & 0xF;
        int f = e * 11;
        int g = f - 13;
        int h = g | 0x1;
        int j = h << 3;
        int k = j ^ 0xAA;
        int l = k % 17;
        int m = l + 19;
        int n = m * 23;
        int o = n / 5;
        int p = o - 29;
        int q = p & 0xFF;
        int r = q | 0x80;
        int s = r ^ 0x55;
        int t = s << 1;
        sum += t;
    }
    
    return sum;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int x = data[i];
        
        /* Complex if-else chain */
        if (x < 0) {
            if (x < -100) return -1;
            if (x < -50) {
                result += x * 2;
                continue;
            }
            result += x;
        } else if (x == 0) {
            result += 1;
        } else if (x < 50) {
            result += x * 3;
        } else if (x < 100) {
            result += x * 4;
        } else {
            if (x > 1000) return -2;
            result += x * 5;
        }
        
        /* Switch with many cases */
        switch (x % 13) {
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
            default: result += 100; break;
        }
        
        /* Nested switch */
        switch (x % 7) {
            case 0:
                if (result > 1000) {
                    result /= 2;
                }
                break;
            case 1:
                result <<= 1;
                break;
            case 2:
                result >>= 1;
                break;
            case 3:
                result ^= 0xFF;
                break;
            case 4:
                result |= 0xAA;
                break;
            case 5:
                result &= 0x55;
                break;
            case 6:
                result = ~result;
                break;
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
int test_asm_constraints(int *data, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        int result;
        
        /* Multiple asm statements competing for registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result)
            : "r" (val)
            : "%eax", "cc"
        );
        
        int result2;
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $7, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (result2)
            : "r" (result)
            : "%ebx", "cc"
        );
        
        int result3;
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl $0x55, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (result3)
            : "r" (result2)
            : "%ecx", "cc"
        );
        
        int result4;
        asm volatile (
            "movl %1, %%edx\n\t"
            "shrl $2, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r" (result4)
            : "r" (result3)
            : "%edx", "cc"
        );
        
        sum += result4;
        
        /* Memory clobber to force spills */
        asm volatile ("" ::: "memory");
    }
    
    return sum;
}

/* Function 4: Mixed data types stressing register classes */
long long test_mixed_types(int *idata, float *fdata, double *ddata, char *cdata, int size) {
    long long lsum = 0;
    double dsum = 0.0;
    float fsum = 0.0f;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Integer operations */
        int ival = idata[i];
        char cval = cdata[i];
        short sval = (short)(ival & 0xFFFF);
        long lval = (long)ival * 3L;
        
        /* Floating point operations */
        float fval = fdata[i];
        double dval = ddata[i];
        
        /* Mixed type computations */
        fval = fval * 1.5f + (float)ival;
        dval = dval * 2.5 + (double)cval;
        
        /* Type conversions */
        int ifromf = (int)fval;
        int ifromd = (int)dval;
        float ffromi = (float)ival;
        double dfroml = (double)lval;
        
        /* More mixed operations */
        lsum += (long long)ifromf * ifromd;
        dsum += dfroml * dval;
        fsum += ffromi * fval;
        
        /* Complex address calculations */
        int idx1 = (i * 3) % size;
        int idx2 = (i * 7) % size;
        int idx3 = (i * 11) % size;
        
        lsum += idata[idx1] * idata[idx2] + idata[idx3];
    }
    
    /* Final mixed type result */
    return lsum + (long long)dsum + (long long)fsum;
}

/* Function 5: Function calls with many arguments */
int __attribute__((noinline)) 
many_args(int a, int b, int c, int d, int e, int f, int g, int h, 
          int i, int j, int k, int l, int m, int n, int o, int p) {
    /* Complex computation using all arguments */
    int sum = a + b + c + d;
    sum = sum * e - f;
    sum = sum / g + h;
    sum = sum ^ i | j;
    sum = sum << k >> l;
    sum = sum + m - n;
    sum = sum * o / p;
    
    /* Create register pressure within */
    int t1 = sum * 2;
    int t2 = t1 + 3;
    int t3 = t2 << 1;
    int t4 = t3 ^ 0xFF;
    int t5 = t4 & 0x7F;
    int t6 = t5 | 0x80;
    int t7 = t6 >> 2;
    int t8 = t7 * 5;
    int t9 = t8 - 7;
    int t10 = t9 + 11;
    
    return t10;
}

int test_many_args(int *data, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i += 16) {
        /* Call with many register and stack arguments */
        int result = many_args(
            data[i], data[i+1], data[i+2], data[i+3],
            data[i+4], data[i+5], data[i+6], data[i+7],
            data[i+8], data[i+9], data[i+10], data[i+11],
            data[i+12], data[i+13], data[i+14], data[i+15]
        );
        sum += result;
    }
    
    return sum;
}

/* Function 6: Pointer aliasing and volatile */
int test_aliasing(int *data, int size) {
    volatile int * volatile ptr1 = data;
    int *ptr2 = data + size/2;
    int *ptr3 = data + size/4;
    int *ptr4 = data + 3*size/4;
    
    int sum = 0;
    int i;
    
    /* Pointer aliasing prevents optimization */
    for (i = 0; i < size/2; i++) {
        *ptr1 = *ptr1 + *ptr2;
        *ptr2 = *ptr2 - *ptr3;
        *ptr3 = *ptr3 ^ *ptr4;
        *ptr4 = *ptr4 | *ptr1;
        
        sum += *ptr1 + *ptr2 + *ptr3 + *ptr4;
        
        ptr1++;
        ptr2--;
        ptr3 += 2;
        ptr4 -= 2;
        
        /* Memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    return sum;
}

/* Main function with warm-up and verification */
int main() {
    int i, j;
    long long total_sum = 0;
    
    /* Allocate and initialize arrays */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!int_data || !float_data || !double_data || !char_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)(rand() % 1000) / 10.0f;
        double_data[i] = (double)(rand() % 1000) / 10.0;
        char_data[i] = (char)(rand() % 256);
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up phase */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (j = 0; j < WARMUP_ITERATIONS; j++) {
        for (i = 0; i < 10; i++) {
            total_sum += test_complex_loops(int_data, ARRAY_SIZE/10);
        }
        asm volatile ("" ::: "memory"); /* Prevent optimization across iterations */
    }
    
    /* Main test phase */
    printf("Main test phase (%d iterations)...\n", ITERATIONS);
    for (j = 0; j < ITERATIONS; j++) {
        /* Call all test functions in sequence */
        total_sum += test_complex_loops(int_data, ARRAY_SIZE);
        asm volatile ("" ::: "memory");
        
        total_sum += test_complex_cfg(int_data, ARRAY_SIZE);
        asm volatile ("" ::: "memory");
        
        total_sum += test_asm_constraints(int_data, ARRAY_SIZE/2);
        asm volatile ("" ::: "memory");
        
        total_sum += test_mixed_types(int_data, float_data, 
                                     double_data, char_data, ARRAY_SIZE/4);
        asm volatile ("" ::: "memory");
        
        total_sum += test_many_args(int_data, ARRAY_SIZE);
        asm volatile ("" ::: "memory");
        
        total_sum += test_aliasing(int_data, ARRAY_SIZE);
        asm volatile ("" ::: "memory");
        
        if (j % 10 == 0) {
            printf("  Completed %d/%d iterations\n", j, ITERATIONS);
        }
    }
    
    /* Final verification */
    printf("Final checksum: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    free(char_data);
    
    printf("Test completed successfully.\n");
    return 0;
}
