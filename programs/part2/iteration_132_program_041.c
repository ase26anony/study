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
volatile int global_barrier = 0;

/* Function 1: Deeply nested loops with many live ranges */
int test_nested_loops(int *data, int size) {
    int sum = 0;
    int i, j, k, l, m;
    
    /* Declare many variables at function scope to extend live ranges */
    int temp1, temp2, temp3, temp4, temp5;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    
    /* Complex nested loops */
    for (i = 0; i < size / 10; i++) {
        temp1 = data[i];
        for (j = 0; j < 5; j++) {
            temp2 = temp1 * j;
            for (k = 0; k < 3; k++) {
                temp3 = temp2 + k * 7;
                for (l = 0; l < 2; l++) {
                    temp4 = temp3 - l * 11;
                    for (m = 0; m < 2; m++) {
                        temp5 = temp4 ^ (m * 13);
                        acc1 += temp5;
                        acc2 += temp5 * 2;
                        acc3 += temp5 * 3;
                        acc4 += temp5 * 4;
                        acc5 += temp5 * 5;
                    }
                }
            }
        }
        sum += acc1 + acc2 + acc3 + acc4 + acc5;
        
        /* Reset accumulators but keep them alive */
        asm volatile("" : "+r"(acc1), "+r"(acc2), "+r"(acc3), "+r"(acc4), "+r"(acc5));
        acc1 = acc2 = acc3 = acc4 = acc5 = 0;
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
            if (x < -100) return -1;  /* Early return */
            x = -x;
        } else if (x == 0) {
            continue;  /* Skip to next iteration */
        } else if (x > 1000) {
            break;  /* Exit loop */
        }
        
        /* Switch with many cases and fall-through */
        switch (x % 15) {
            case 0: result += x * 2; break;
            case 1: result += x * 3; /* fall through */
            case 2: result += x * 4; break;
            case 3: result += x * 5; /* fall through */
            case 4: result += x * 6; /* fall through */
            case 5: result += x * 7; break;
            case 6: result += x * 8; break;
            case 7: result += x * 9; /* fall through */
            case 8: result += x * 10; break;
            case 9: result += x * 11; /* fall through */
            case 10: result += x * 12; /* fall through */
            case 11: result += x * 13; break;
            case 12: result += x * 14; break;
            case 13: result += x * 15; /* fall through */
            case 14: result += x * 16; break;
            default: result += x;
        }
        
        /* Nested loop with break at different level */
        for (int j = 0; j < 3; j++) {
            if (result > 1000000) {
                goto done;  /* Jump out of nested loops */
            }
            for (int k = 0; k < 2; k++) {
                if (result < 0) {
                    break;  /* Break inner loop only */
                }
                result += j * k;
            }
        }
    }
    
done:
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_inline_asm(int *data, int size, int *result) {
    int i;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    /* Multiple asm statements competing for registers */
    for (i = 0; i < size; i += 8) {
        /* Force use of specific registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $7, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(r1)
            : "r"(data[i])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "addl $13, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r"(r2)
            : "r"(data[i+1])
            : "%ebx", "memory"
        );
        
        asm volatile (
            "movl %1, %%ecx\n\t"
            "xorl $42, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r"(r3)
            : "r"(data[i+2])
            : "%ecx", "memory"
        );
        
        asm volatile (
            "movl %1, %%edx\n\t"
            "subl $23, %%edx\n\t"
            "movl %%edx, %0\n\t"
            : "=r"(r4)
            : "r"(data[i+3])
            : "%edx", "memory"
        );
        
        /* More register pressure */
        asm volatile (
            "movl %1, %%esi\n\t"
            "shll $2, %%esi\n\t"
            "movl %%esi, %0\n\t"
            : "=r"(r5)
            : "r"(data[i+4])
            : "%esi", "memory"
        );
        
        asm volatile (
            "movl %1, %%edi\n\t"
            "shrl $1, %%edi\n\t"
            "movl %%edi, %0\n\t"
            : "=r"(r6)
            : "r"(data[i+5])
            : "%edi", "memory"
        );
        
        /* Compete for same registers */
        asm volatile (
            "movl %1, %%eax\n\t"  /* Reuse EAX */
            "addl %%eax, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r"(r7)
            : "r"(data[i+6])
            : "%eax", "memory"
        );
        
        asm volatile (
            "movl %1, %%ebx\n\t"  /* Reuse EBX */
            "orl $0xFF, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r"(r8)
            : "r"(data[i+7])
            : "%ebx", "memory"
        );
        
        /* Use all results */
        result[0] += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types(char *cdata, short *sdata, int *idata, 
                       long *ldata, float *fdata, double *ddata,
                       int size, int offset1, int offset2, int offset3,
                       int offset4, int offset5, int offset6) {
    /* Many arguments stress register/stack passing */
    double total = 0.0;
    
    for (int i = 0; i < size; i++) {
        /* Mixed type computations */
        char c = cdata[(i + offset1) % size];
        short s = sdata[(i + offset2) % size];
        int n = idata[(i + offset3) % size];
        long l = ldata[(i + offset4) % size];
        float f = fdata[(i + offset5) % size];
        double d = ddata[(i + offset6) % size];
        
        /* Complex expression with many intermediates */
        double temp = (double)c * 1.5 +
                     (double)s * 2.5 +
                     (double)n * 3.5 +
                     (double)l * 0.5 +
                     (double)f * 4.5 +
                     d * 5.5;
        
        /* Trigonometric functions create more register pressure */
        temp = sin(temp) * cos(temp) + tan(temp * 0.01);
        
        /* Power and log operations */
        if (temp > 0) {
            temp = pow(temp, 1.5) + log(temp + 1.0);
        }
        
        total += temp;
        
        /* Pointer aliasing to prevent optimization */
        volatile double *alias = &total;
        *alias = *alias + 0.000001;
    }
    
    return total;
}

/* Function 5: Vector-like operations using multiple accumulators */
long test_vector_ops(int *data, int size) {
    long sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    long sum5 = 0, sum6 = 0, sum7 = 0, sum8 = 0;
    
    /* Unrolled loop with multiple accumulators */
    for (int i = 0; i < size - 7; i += 8) {
        sum1 += data[i] * 3;
        sum2 += data[i+1] * 5;
        sum3 += data[i+2] * 7;
        sum4 += data[i+3] * 11;
        sum5 += data[i+4] * 13;
        sum6 += data[i+5] * 17;
        sum7 += data[i+6] * 19;
        sum8 += data[i+7] * 23;
        
        /* Cross-accumulator dependencies */
        sum1 += sum2 % 100;
        sum3 += sum4 % 200;
        sum5 += sum6 % 300;
        sum7 += sum8 % 400;
        
        /* More complex dependencies */
        sum2 = sum1 ^ sum3;
        sum4 = sum3 | sum5;
        sum6 = sum5 & sum7;
        sum8 = sum7 + sum1;
    }
    
    /* Final reduction */
    return sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
}

/* Function 6: Irreducible control flow using computed goto */
int test_computed_goto(int *data, int size) {
    static void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    int result = 0;
    int i = 0;
    
    /* Create irreducible control flow */
    goto *labels[data[0] % 10];
    
label0:
    result += data[i++] * 2;
    if (i >= size) goto end;
    goto *labels[data[i] % 10];
    
label1:
    result += data[i++] * 3;
    if (i >= size) goto end;
    goto *labels[(data[i] + 1) % 10];
    
label2:
    result += data[i++] * 5;
    if (i >= size) goto end;
    goto *labels[(data[i] + 2) % 10];
    
label3:
    result += data[i++] * 7;
    if (i >= size) goto end;
    goto *labels[(data[i] + 3) % 10];
    
label4:
    result += data[i++] * 11;
    if (i >= size) goto end;
    goto *labels[(data[i] + 4) % 10];
    
label5:
    result += data[i++] * 13;
    if (i >= size) goto end;
    goto *labels[(data[i] + 5) % 10];
    
label6:
    result += data[i++] * 17;
    if (i >= size) goto end;
    goto *labels[(data[i] + 6) % 10];
    
label7:
    result += data[i++] * 19;
    if (i >= size) goto end;
    goto *labels[(data[i] + 7) % 10];
    
label8:
    result += data[i++] * 23;
    if (i >= size) goto end;
    goto *labels[(data[i] + 8) % 10];
    
label9:
    result += data[i++] * 29;
    if (i >= size) goto end;
    goto *labels[(data[i] + 9) % 10];
    
end:
    return result;
}

/* Main function that runs all tests */
int main() {
    /* Allocate and initialize data */
    int *int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *char_data = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long *long_data = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 1000;
        long_data[i] = rand() % 10000;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    int asm_result[1] = {0};
    long total_result = 0;
    
    printf("Starting register pressure stress test...\n");
    
    /* Warm-up iterations for profile feedback */
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        total_result += test_nested_loops(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");  /* Memory barrier */
    }
    
    /* Main test iterations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Nested loops */
        total_result += test_nested_loops(int_data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");
        
        /* Test 2: Complex CFG */
        total_result += test_complex_cfg(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 3: Inline assembly */
        test_inline_asm(int_data, ARRAY_SIZE, asm_result);
        total_result += asm_result[0];
        asm_result[0] = 0;
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed types */
        double mixed_result = test_mixed_types(
            char_data, short_data, int_data, long_data, 
            float_data, double_data, ARRAY_SIZE / 10,
            iter % 10, (iter + 1) % 10, (iter + 2) % 10,
            (iter + 3) % 10, (iter + 4) % 10, (iter + 5) % 10);
        total_result += (long)mixed_result;
        asm volatile("" ::: "memory");
        
        /* Test 5: Vector operations */
        total_result += test_vector_ops(int_data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        /* Test 6: Computed goto */
        total_result += test_computed_goto(int_data, ARRAY_SIZE / 100);
        asm volatile("" ::: "memory");
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 100) {
            int_data[i] = (int_data[i] * 13 + 17) % 1000;
        }
    }
    
    /* Clean up */
    free(int_data);
    free(char_data);
    free(short_data);
    free(long_data);
    free(float_data);
    free(double_data);
    
    printf("Test completed. Final checksum: %ld\n", total_result);
    printf("Expected range: 0 to ~2^31 (implementation dependent)\n");
    
    return 0;
}
