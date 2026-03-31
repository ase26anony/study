/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int global_seed = 42;
volatile double global_acc = 0.0;

/* Function 1: Deeply nested loops with many live ranges */
void test_nested_loops(int *data, int size) {
    int i, j, k, l;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    double f1, f2, f3, f4;
    
    /* Outer loops create many overlapping live ranges */
    for (i = 0; i < size / 4; i++) {
        tmp1 = data[i] * 3;
        for (j = 0; j < size / 8; j++) {
            tmp2 = data[j] + tmp1;  /* tmp1 live across inner loop */
            f1 = sin(tmp2 * 0.01);
            for (k = 0; k < size / 16; k++) {
                tmp3 = data[k] ^ tmp2;  /* tmp2 live across deeper loop */
                f2 = cos(tmp3 * 0.02);
                for (l = 0; l < size / 32; l++) {
                    tmp4 = data[l] | tmp3;  /* tmp3 live across innermost */
                    f3 = tan(tmp4 * 0.03);
                    acc1 += (int)(f1 * 100);
                    acc2 += (int)(f2 * 100);
                    acc3 += (int)(f3 * 100);
                    f4 = f1 + f2 + f3;  /* All floats live here */
                    acc4 += (int)(f4 * 100);
                }
                /* Force all variables to stay alive */
                asm volatile("" : "+r"(tmp3), "+r"(f2) : : "memory");
            }
            asm volatile("" : "+r"(tmp2), "+r"(f1) : : "memory");
        }
        asm volatile("" : "+r"(tmp1) : : "memory");
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Function 2: Complex control flow with switch and early returns */
int test_complex_cfg(int *data, int size) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Nested if-else chains */
        if (data[i] < 100) {
            if (data[i] < 50) {
                if (data[i] < 25) {
                    result += data[i] * 2;
                    if (data[i] < 10) return result;  /* Early return */
                } else {
                    result += data[i] * 3;
                }
            } else {
                result += data[i] * 4;
            }
        } else if (data[i] < 200) {
            result += data[i] * 5;
        } else if (data[i] < 300) {
            result += data[i] * 6;
        } else {
            result += data[i] * 7;
        }
        
        /* Large switch statement with fall-through */
        switch (data[i] % SWITCH_CASES) {
            case 0: result += 1; /* Fall through */
            case 1: result += 2; /* Fall through */
            case 2: result += 3; break;
            case 3: result += 4; /* Fall through */
            case 4: result += 5; /* Fall through */
            case 5: result += 6; /* Fall through */
            case 6: result += 7; break;
            case 7: result += 8; /* Fall through */
            case 8: result += 9; /* Fall through */
            case 9: result += 10; break;
            case 10: result += 11; /* Fall through */
            case 11: result += 12; /* Fall through */
            case 12: result += 13; break;
            case 13: result += 14;
            case 14: result += 15; break;
        }
        
        /* Loop with break/continue at different levels */
        for (j = 0; j < 10; j++) {
            if (data[i] % 3 == 0) {
                result += j;
                if (j == 5) break;
            } else if (data[i] % 3 == 1) {
                result -= j;
                if (j == 3) continue;
            } else {
                result *= (j + 1);
            }
        }
    }
    
    return result;
}

/* Function 3: Inline assembly with register constraints */
void test_asm_constraints(int *data, int size) {
    int i;
    int a, b, c, d, e, f;
    long long la, lb, lc;
    double da, db, dc;
    
    for (i = 0; i < size; i += 8) {
        /* Compete for EAX/RAX register */
        a = data[i];
        b = data[i + 1];
        c = data[i + 2];
        d = data[i + 3];
        
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "imull %3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (a)
            : "r" (b), "r" (c), "r" (d)
            : "%eax", "memory"
        );
        
        /* Compete for multiple registers */
        e = data[i + 4];
        f = data[i + 5];
        
        asm volatile (
            "movl %1, %%ebx\n\t"
            "movl %2, %%ecx\n\t"
            "addl %%ebx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (e)
            : "r" (e), "r" (f)
            : "%ebx", "%ecx", "memory"
        );
        
        /* 64-bit operations */
        la = (long long)data[i] * data[i + 1];
        lb = (long long)data[i + 2] * data[i + 3];
        
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (lc)
            : "r" (la), "r" (lb)
            : "%rax", "memory"
        );
        
        /* Floating point with XMM registers */
        da = (double)data[i] * 0.1;
        db = (double)data[i + 1] * 0.2;
        
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=x" (dc)
            : "x" (da), "x" (db)
            : "%xmm0", "memory"
        );
        
        data[i] = a + e + (int)lc + (int)dc;
    }
}

/* Function 4: Mixed data types and many function arguments */
double test_mixed_types(char c, short s, int i, long l, 
                        float f, double d, int *p, 
                        long long ll, double *dp, int count) {
    /* All parameters compete for registers/stack slots */
    double result = 0.0;
    int j;
    
    for (j = 0; j < count; j++) {
        /* Mixed type computations */
        double temp = (double)c + (double)s + (double)i + (double)l;
        temp += (double)f + d + (double)ll + (double)p[j] + dp[j];
        
        /* Complex expression with many intermediates */
        double a = temp * 1.1;
        double b = temp * 2.2;
        double c1 = temp * 3.3;
        double d1 = temp * 4.4;
        double e = temp * 5.5;
        double f1 = temp * 6.6;
        double g = temp * 7.7;
        double h = temp * 8.8;
        
        /* Keep all variables alive */
        result += a + b + c1 + d1 + e + f1 + g + h;
        
        /* Pointer aliasing to prevent optimization */
        *dp = result;
        volatile double *volatile_dp = dp;
        *volatile_dp = result * 0.5;
    }
    
    return result;
}

/* Function 5: Vector-like operations with many SIMD-sized variables */
void test_vector_ops(int *data, int size) {
    int i;
    /* Many variables to stress register allocation */
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int w0, w1, w2, w3, w4, w5, w6, w7;
    int x0, x1, x2, x3, x4, x5, x6, x7;
    int y0, y1, y2, y3, y4, y5, y6, y7;
    int z0, z1, z2, z3, z4, z5, z6, z7;
    
    for (i = 0; i < size; i += 16) {
        /* Load multiple values (simulating vector load) */
        v0 = data[i];     v1 = data[i + 1];  v2 = data[i + 2];  v3 = data[i + 3];
        v4 = data[i + 4]; v5 = data[i + 5];  v6 = data[i + 6];  v7 = data[i + 7];
        w0 = data[i + 8]; w1 = data[i + 9];  w2 = data[i + 10]; w3 = data[i + 11];
        w4 = data[i + 12]; w5 = data[i + 13]; w6 = data[i + 14]; w7 = data[i + 15];
        
        /* Vector-like operations keeping all variables alive */
        x0 = v0 + w0; x1 = v1 + w1; x2 = v2 + w2; x3 = v3 + w3;
        x4 = v4 + w4; x5 = v5 + w5; x6 = v6 + w6; x7 = v7 + w7;
        
        y0 = x0 * 2; y1 = x1 * 3; y2 = x2 * 4; y3 = x3 * 5;
        y4 = x4 * 6; y5 = x5 * 7; y6 = x6 * 8; y7 = x7 * 9;
        
        z0 = y0 >> 1; z1 = y1 >> 2; z2 = y2 >> 3; z3 = y3 >> 4;
        z4 = y4 >> 1; z5 = y5 >> 2; z6 = y6 >> 3; z7 = y7 >> 4;
        
        /* Store results */
        data[i] = z0 + z1 + z2 + z3 + z4 + z5 + z6 + z7;
        
        /* Memory barrier to force spills */
        asm volatile("" ::: "memory");
    }
}

/* Function 6: Computed goto for irreducible control flow */
void test_computed_goto(int *data, int size) {
    int i = 0;
    int result = 0;
    
    /* Labels for computed goto */
    void *labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    while (i < size) {
        int idx = data[i] % 10;
        goto *labels[idx];
        
    label0:
        result += data[i] * 1;
        i++;
        continue;
    label1:
        result += data[i] * 2;
        i++;
        continue;
    label2:
        result += data[i] * 3;
        i++;
        continue;
    label3:
        result += data[i] * 4;
        i++;
        continue;
    label4:
        result += data[i] * 5;
        i++;
        continue;
    label5:
        result += data[i] * 6;
        i++;
        continue;
    label6:
        result += data[i] * 7;
        i++;
        continue;
    label7:
        result += data[i] * 8;
        i++;
        continue;
    label8:
        result += data[i] * 9;
        i++;
        continue;
    label9:
        result += data[i] * 10;
        i++;
        continue;
    }
    
    global_acc += result;
}

/* Main function that runs all tests */
int main() {
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int i, iter;
    long long checksum = 0;
    clock_t start, end;
    
    if (!data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random-ish data */
    srand(global_seed);
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
        double_data[i] = (double)data[i] / 1000.0;
    }
    
    printf("Starting register pressure stress test...\n");
    start = clock();
    
    /* Warm-up iterations */
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        test_nested_loops(data, ARRAY_SIZE / 10);
        asm volatile("" ::: "memory");  /* Prevent optimization across calls */
    }
    
    /* Main test iterations */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Call all test functions in sequence */
        test_nested_loops(data + iter * 10, ARRAY_SIZE - iter * 10);
        asm volatile("" ::: "memory");
        
        int cfg_result = test_complex_cfg(data, ARRAY_SIZE / 2);
        checksum += cfg_result;
        asm volatile("" ::: "memory");
        
        test_asm_constraints(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        double mixed_result = test_mixed_types(
            iter & 0xFF,                    /* char */
            (iter * 2) & 0xFFFF,           /* short */
            iter * 3,                      /* int */
            (long)iter * 4,                /* long */
            (float)iter * 0.1f,            /* float */
            (double)iter * 0.2,            /* double */
            data,                          /* int* */
            (long long)iter * 5,           /* long long */
            double_data,                   /* double* */
            ARRAY_SIZE / 100               /* count */
        );
        global_acc += mixed_result;
        asm volatile("" ::: "memory");
        
        test_vector_ops(data, ARRAY_SIZE);
        asm volatile("" ::: "memory");
        
        test_computed_goto(data, ARRAY_SIZE / 5);
        asm volatile("" ::: "memory");
        
        /* Update data to create varying patterns */
        for (i = 0; i < ARRAY_SIZE; i++) {
            data[i] = (data[i] * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    end = clock();
    
    /* Compute final checksum */
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
    }
    checksum += (long long)global_acc;
    
    printf("Test completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Final checksum: %lld\n", checksum);
    printf("Global accumulator: %.2f\n", global_acc);
    
    free(data);
    free(double_data);
    
    return 0;
}
