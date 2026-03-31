/* test_mcf_coverage.c
 * 
 * This program is designed to stress GCC's register allocator and trigger
 * the min-cost flow solver's debug output, specifically targeting the
 * print_edge function's uncovered block for ENTRY/EXIT node handling.
 *
 * Compile with: gcc -O3 -fdump-ira-all -fdump-ira-details -fdump-rtl-all 
 *               -fno-omit-frame-pointer -dA -dp -dD -dP -dR -da 
 *               -fdump-rtl-bbro -fdump-rtl-regclass test_mcf_coverage.c -o test_mcf
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
volatile double global_accumulator = 0.0;

/* Complex nested loop with many live ranges */
void test_deep_nesting(int *data, int size) {
    int i, j, k, l;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int temp1, temp2, temp3, temp4;
    double fp1, fp2, fp3, fp4;
    
    /* Deeply nested loops creating many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        temp1 = data[i];
        fp1 = sqrt(fabs(temp1));
        
        for (j = i; j < size / 3; j++) {
            temp2 = data[j] * 2;
            fp2 = sin(temp2) * cos(temp2);
            
            for (k = j; k < size / 2; k++) {
                temp3 = data[k] + temp1 + temp2;
                fp3 = exp(fp1 + fp2) * log(fabs(temp3) + 1.0);
                
                for (l = k; l < size; l++) {
                    temp4 = data[l] - temp3;
                    fp4 = fp1 * fp2 * fp3 * tan(temp4);
                    
                    sum1 += (int)(fp1 * 1000);
                    sum2 += (int)(fp2 * 1000);
                    sum3 += (int)(fp3 * 1000);
                    sum4 += (int)(fp4 * 1000);
                    
                    /* Complex expression with many intermediates */
                    data[l] = (temp1 * temp2 + temp3 * temp4) ^ 
                              (sum1 & sum2) | (sum3 ^ sum4);
                }
                
                /* Early continue to create complex CFG */
                if (temp3 % 7 == 0) continue;
                
                data[k] = sum1 ^ sum2 ^ sum3 ^ sum4;
            }
            
            /* Nested if-else chain */
            if (temp2 > 1000) {
                sum1 += 1000;
            } else if (temp2 > 500) {
                sum2 += 500;
            } else if (temp2 > 250) {
                sum3 += 250;
            } else if (temp2 > 125) {
                sum4 += 125;
            } else {
                sum1 = sum2 + sum3 + sum4;
            }
        }
        
        /* Multiple function calls within loop */
        global_counter += sum1;
        global_accumulator += fp1;
    }
}

/* Complex switch statement with fall-through cases */
int test_complex_switch(int value, int *data, int size) {
    int result = 0;
    int i;
    
    switch (value % 15) {
        case 0:
            result = data[0];
            /* Fall through */
        case 1:
            result += data[1];
            for (i = 2; i < size; i += 2) {
                result ^= data[i];
            }
            break;
        case 2:
            result = data[2] * 3;
            /* Fall through */
        case 3:
            result -= data[3];
            /* Fall through */
        case 4:
            result |= data[4];
            break;
        case 5:
            for (i = 0; i < size; i++) {
                result += data[i] * i;
            }
            break;
        case 6:
            result = data[6] << 2;
            /* Fall through */
        case 7:
            result >>= 1;
            /* Fall through */
        case 8:
            result &= 0xFF;
            break;
        case 9:
            result = ~data[9];
            /* Fall through */
        case 10:
            result = -result;
            break;
        case 11:
            result = data[11] / 2;
            /* Fall through */
        case 12:
            result %= 13;
            break;
        case 13:
            result = abs(data[13]);
            /* Fall through */
        case 14:
            result = result * result;
            break;
        default:
            result = -1;
    }
    
    /* Multiple early returns */
    if (result < 0) return -result;
    if (result > 1000) return result / 1000;
    if (result == 0) return 1;
    
    return result;
}

/* Function with inline assembly and register constraints */
void test_asm_register_pressure(int *data, int size) {
    int i;
    int a, b, c, d, e, f, g, h;
    long la, lb, lc, ld;
    double da, db, dc;
    float fa, fb, fc;
    
    /* Multiple asm statements with fixed register constraints */
    for (i = 0; i < size; i++) {
        a = data[i];
        b = data[(i + 1) % size];
        c = data[(i + 2) % size];
        d = data[(i + 3) % size];
        
        /* Compete for EAX/RAX register */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (e)
            : "r" (a), "r" (b)
            : "%eax", "memory"
        );
        
        /* Compete for EBX/RBX register */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (f)
            : "r" (c), "r" (d)
            : "%ebx", "memory"
        );
        
        /* Use multiple register classes */
        la = (long)a * b;
        lb = (long)c * d;
        
        asm volatile (
            "addq %1, %0\n\t"
            : "+r" (la)
            : "r" (lb)
            : "cc", "memory"
        );
        
        /* Floating point operations */
        da = (double)a / (b + 1);
        db = (double)c / (d + 1);
        
        asm volatile (
            "addsd %1, %0\n\t"
            : "+x" (da)
            : "x" (db)
            : "memory"
        );
        
        /* Mixed types creating register class pressure */
        fa = (float)a;
        fb = (float)b;
        
        asm volatile (
            "mulss %1, %0\n\t"
            : "+x" (fa)
            : "x" (fb)
            : "memory"
        );
        
        /* Store results back, creating store-load dependencies */
        data[i] = e + f + (int)la + (int)da + (int)fa;
    }
}

/* Function with many arguments to stress calling convention */
int test_many_arguments(int a1, int a2, int a3, int a4, int a5,
                        int a6, int a7, int a8, int a9, int a10,
                        double f1, double f2, double f3, double f4,
                        float f5, float f6, float f7, float f8) {
    /* Complex expression using all arguments */
    int result = (a1 * a2 + a3 * a4 - a5 * a6) / (a7 + a8 - a9 + a10);
    double fp_result = (f1 * f2 + f3 * f4) / (f5 * f6 + f7 * f8);
    
    /* Keep all variables alive across complex computation */
    volatile int keep_alive1 = a1 + a2 + a3 + a4;
    volatile int keep_alive2 = a5 + a6 + a7 + a8;
    volatile double keep_alive3 = f1 + f2 + f3;
    volatile float keep_alive4 = f4 + f5 + f6;
    
    /* Pointer aliasing to prevent optimization */
    int *ptr1 = &keep_alive1;
    int *ptr2 = &keep_alive2;
    *ptr1 ^= *ptr2;
    *ptr2 ^= *ptr1;
    *ptr1 ^= *ptr2;
    
    return result + (int)fp_result + keep_alive1 + keep_alive2 + (int)keep_alive3 + (int)keep_alive4;
}

/* Function with computed goto for irreducible control flow */
void test_computed_goto(int *data, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int i = 0;
    int sum = 0;
    
    /* Irreducible control flow */
    goto *labels[data[0] % 10];
    
label0:
    sum += data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 1) % 10];
    
label1:
    sum -= data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 2) % 10];
    
label2:
    sum *= data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 3) % 10];
    
label3:
    sum ^= data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 4) % 10];
    
label4:
    sum |= data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 5) % 10];
    
label5:
    sum &= data[i++];
    if (i >= size) goto end;
    goto *labels[(data[i] + 6) % 10];
    
label6:
    sum <<= (data[i++] & 3);
    if (i >= size) goto end;
    goto *labels[(data[i] + 7) % 10];
    
label7:
    sum >>= (data[i++] & 3);
    if (i >= size) goto end;
    goto *labels[(data[i] + 8) % 10];
    
label8:
    sum = ~sum;
    i++;
    if (i >= size) goto end;
    goto *labels[(data[i] + 9) % 10];
    
label9:
    sum = -sum;
    i++;
    if (i >= size) goto end;
    goto *labels[data[i] % 10];
    
end:
    global_counter += sum;
}

/* Vector operations using multiple SIMD registers */
void test_vector_operations(int *data, int size) {
    int i;
    /* Use multiple vector-like operations */
    for (i = 0; i < size - 3; i += 4) {
        int v1 = data[i];
        int v2 = data[i + 1];
        int v3 = data[i + 2];
        int v4 = data[i + 3];
        
        /* SIMD-like operations */
        int add1 = v1 + v2;
        int add2 = v3 + v4;
        int mul1 = v1 * v2;
        int mul2 = v3 * v4;
        int shl1 = v1 << 1;
        int shl2 = v2 << 2;
        int shl3 = v3 << 3;
        int shl4 = v4 << 4;
        
        /* Cross dependencies */
        data[i] = add1 + mul1 + shl1;
        data[i + 1] = add2 + mul2 + shl2;
        data[i + 2] = add1 - mul1 + shl3;
        data[i + 3] = add2 - mul2 + shl4;
    }
}

/* Main test driver */
int main() {
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int i, j;
    long total_checksum = 0;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    printf("Starting MCF coverage test...\n");
    
    /* Warm-up iterations for profile feedback */
    for (j = 0; j < WARMUP_ITERATIONS; j++) {
        test_deep_nesting(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    /* Main test iterations */
    for (j = 0; j < ITERATIONS; j++) {
        /* Test 1: Deep nesting and register pressure */
        test_deep_nesting(data, ARRAY_SIZE / 20);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex control flow */
        for (i = 0; i < 100; i++) {
            total_checksum += test_complex_switch(data[i], data, 100);
        }
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly with register constraints */
        test_asm_register_pressure(data2, ARRAY_SIZE / 50);
        asm volatile("" ::: "memory");
        
        /* Test 4: Many arguments */
        total_checksum += test_many_arguments(
            data[0], data[1], data[2], data[3], data[4],
            data[5], data[6], data[7], data[8], data[9],
            (double)data[10] / 1000.0, (double)data[11] / 1000.0,
            (double)data[12] / 1000.0, (double)data[13] / 1000.0,
            (float)data[14] / 1000.0f, (float)data[15] / 1000.0f,
            (float)data[16] / 1000.0f, (float)data[17] / 1000.0f
        );
        asm volatile("" ::: "memory");
        
        /* Test 5: Computed goto */
        test_computed_goto(data, 100);
        asm volatile("" ::: "memory");
        
        /* Test 6: Vector operations */
        test_vector_operations(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Mix data arrays to create aliasing concerns */
        if (j % 2 == 0) {
            memcpy(data, data2, (ARRAY_SIZE / 2) * sizeof(int));
        } else {
            memcpy(data2, data, (ARRAY_SIZE / 2) * sizeof(int));
        }
    }
    
    /* Final checksum calculation */
    for (i = 0; i < ARRAY_SIZE; i++) {
        total_checksum += data[i] + data2[i];
    }
    
    printf("Test completed. Checksum: %ld\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    printf("Global accumulator: %f\n", global_accumulator);
    
    free(data);
    free(data2);
    
    return 0;
}
