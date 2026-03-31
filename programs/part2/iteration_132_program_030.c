/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 15000
#define ITERATIONS 100
#define SWITCH_CASES 15

/* Volatile variables to prevent optimization and extend live ranges */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Complex expression with many intermediate values */
int complex_expression_test(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Declare many variables at function scope to extend live ranges */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    double dt1, dt2, dt3, dt4, dt5;
    float ft1, ft2, ft3, ft4, ft5;
    
    /* Complex expression tree requiring many temporaries */
    t1 = a * b + c;
    t2 = d << 2;
    t3 = e & 0xFF;
    t4 = f | 0x7F;
    t5 = g ^ h;
    t6 = t1 - t2;
    t7 = t3 * t4;
    t8 = t5 / (t6 + 1);
    t9 = t7 % (t8 + 1);
    t10 = (t6 * t7) + (t8 * t9);
    
    /* Floating point operations mixing types */
    dt1 = (double)t1 * 1.5;
    dt2 = (double)t2 / 3.14159;
    dt3 = dt1 + dt2;
    ft1 = (float)dt3;
    ft2 = ft1 * 2.0f;
    
    /* Keep variables alive across complex control flow */
    if (t10 > 1000) {
        t11 = t10 >> 3;
        t12 = t11 * t1;
        dt4 = dt3 * 2.0;
    } else {
        t11 = t10 << 2;
        t12 = t11 / t2;
        dt4 = dt3 / 2.0;
    }
    
    /* Nested loops creating register pressure */
    for (int i = 0; i < 50; i++) {
        t13 = t12 + i;
        for (int j = 0; j < 20; j++) {
            t14 = t13 * j;
            for (int k = 0; k < 10; k++) {
                t15 = t14 - k;
                t16 = t15 & 0x3F;
                ft3 = ft2 + (float)k;
            }
        }
    }
    
    /* Mixed operations to stress different register classes */
    t17 = (int)dt4;
    t18 = (int)ft3;
    t19 = t17 ^ t18;
    t20 = t19 * t16;
    
    return t20;
}

/* Function with deeply nested control flow */
int complex_cfg_test(int x, int y, int z) {
    int result = 0;
    
    /* Multiple early returns creating complex CFG */
    if (x < 0) return -1;
    if (y == 0) return 0;
    
    /* Nested if-else chains */
    if (x > 100) {
        if (y < 50) {
            result = x + y;
        } else if (y < 100) {
            result = x - y;
        } else {
            if (z > 0) {
                result = x * y;
            } else {
                result = x / (y + 1);
            }
        }
    } else {
        switch (x % SWITCH_CASES) {
            case 0: result = y + z; break;
            case 1: result = y - z; break;
            case 2: result = y * z; break;
            case 3: result = y / (z + 1); break;
            case 4: result = y & z; break;
            case 5: result = y | z; break;
            case 6: result = y ^ z; break;
            case 7: result = y << (z & 3); break;
            case 8: result = y >> (z & 3); break;
            case 9: result = ~y; break;
            case 10: result = -y; break;
            case 11: result = abs(y); break;
            case 12: result = y * y; break;
            case 13: result = z * z; break;
            case 14: result = y * z * x; break;
            default: result = 1; break;
        }
    }
    
    /* Loop with break/continue at different levels */
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) continue;
        
        for (int j = 0; j < 50; j++) {
            if (j == result) break;
            
            for (int k = 0; k < 20; k++) {
                if (k % 7 == 0) continue;
                result += (i * j * k) % 97;
            }
            
            if (j % 13 == 0) {
                result -= 5;
                continue;
            }
        }
        
        if (i % 17 == 0) {
            result *= 2;
            break;
        }
    }
    
    return result;
}

/* Function with inline assembly creating register pressure */
void asm_register_pressure(int *data, int size) {
    int a, b, c, d, e, f, g, h;
    long la, lb, lc, ld;
    double da, db;
    float fa, fb;
    
    /* Multiple asm statements with fixed register constraints */
    for (int i = 0; i < size; i += 8) {
        /* Force use of specific registers */
        asm volatile (
            "mov %[val1], %%eax\n\t"
            "imul %%eax, %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            : [out1] "=r" (a)
            : [val1] "r" (data[i])
            : "eax", "cc"
        );
        
        asm volatile (
            "mov %[val2], %%ebx\n\t"
            "add $1, %%ebx\n\t"
            "mov %%ebx, %[out2]\n\t"
            : [out2] "=r" (b)
            : [val2] "r" (data[i+1])
            : "ebx", "cc"
        );
        
        asm volatile (
            "mov %[val3], %%ecx\n\t"
            "sub $2, %%ecx\n\t"
            "mov %%ecx, %[out3]\n\t"
            : [out3] "=r" (c)
            : [val3] "r" (data[i+2])
            : "ecx", "cc"
        );
        
        /* Memory clobber to force spills */
        asm volatile (
            "mov %[val4], %%edx\n\t"
            "xor $0xFF, %%edx\n\t"
            "mov %%edx, %[out4]\n\t"
            : [out4] "=r" (d)
            : [val4] "r" (data[i+3])
            : "edx", "cc", "memory"
        );
        
        /* Floating point asm */
        da = (double)data[i+4];
        db = (double)data[i+5];
        asm volatile (
            "fldl %[dbl1]\n\t"
            "fldl %[dbl2]\n\t"
            "faddp\n\t"
            "fstpl %[result]\n\t"
            : [result] "=m" (da)
            : [dbl1] "m" (da), [dbl2] "m" (db)
            : "st", "st(1)"
        );
        
        /* More register pressure */
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (f + 1);
        
        /* Use volatile to prevent dead store elimination */
        g_volatile_counter += h;
        g_volatile_double += da;
    }
}

/* Function with many arguments to stress calling convention */
long many_arguments_test(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10,
                         int a11, int a12, int a13, int a14, int a15,
                         double d1, double d2, double d3, double d4,
                         float f1, float f2, float f3) {
    /* Mix all arguments in complex ways */
    long result = 0;
    
    result += a1 * a2;
    result += a3 - a4;
    result += a5 & a6;
    result += a7 | a8;
    result += a9 ^ a10;
    result += a11 << (a12 & 3);
    result += a13 >> (a14 & 3);
    result += a15 * 2;
    
    /* Floating point operations */
    double dsum = d1 + d2 + d3 + d4;
    float fsum = f1 + f2 + f3;
    result += (long)(dsum * 100.0);
    result += (long)(fsum * 10.0f);
    
    /* Nested loops with the arguments */
    for (int i = 0; i < a1; i++) {
        for (int j = 0; j < a2; j++) {
            int temp = a3 + i - j;
            for (int k = 0; k < a4; k++) {
                temp += (a5 * k) % 97;
                if (temp > 1000) {
                    result += temp;
                    break;
                }
            }
            result += temp;
        }
    }
    
    return result;
}

/* Vector-like operations using multiple variables */
void vector_operations(int *in1, int *in2, int *out, int size) {
    /* Unrolled loop creating many simultaneous live values */
    for (int i = 0; i < size; i += 8) {
        int v0 = in1[i] + in2[i];
        int v1 = in1[i+1] - in2[i+1];
        int v2 = in1[i+2] * in2[i+2];
        int v3 = in1[i+3] & in2[i+3];
        int v4 = in1[i+4] | in2[i+4];
        int v5 = in1[i+5] ^ in2[i+5];
        int v6 = in1[i+6] << 1;
        int v7 = in1[i+7] >> 1;
        
        /* Cross dependencies */
        int t0 = v0 + v1;
        int t1 = v2 - v3;
        int t2 = v4 * v5;
        int t3 = v6 & v7;
        
        int u0 = t0 ^ t1;
        int u1 = t2 | t3;
        int u2 = t0 + t2;
        int u3 = t1 - t3;
        
        out[i] = u0;
        out[i+1] = u1;
        out[i+2] = u2;
        out[i+3] = u3;
        out[i+4] = u0 + u1;
        out[i+5] = u2 - u3;
        out[i+6] = u0 * u2;
        out[i+7] = u1 & u3;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* Main test driver */
int main() {
    /* Initialize with random data */
    int *data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = malloc(ARRAY_SIZE * sizeof(int));
    int *result = malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
    }
    
    long total_checksum = 0;
    
    /* Warm-up iterations */
    printf("Starting warm-up iterations...\n");
    for (int iter = 0; iter < ITERATIONS/10; iter++) {
        int temp = complex_expression_test(
            data1[iter] % 100, 
            data2[iter] % 100,
            (iter * 3) % 100,
            (iter * 5) % 100,
            (iter * 7) % 100,
            (iter * 11) % 100,
            (iter * 13) % 100,
            (iter * 17) % 100
        );
        total_checksum += temp;
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
    }
    
    /* Main test iterations */
    printf("Starting main test iterations...\n");
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Complex expressions */
        int r1 = complex_expression_test(
            data1[iter*10] % 256,
            data2[iter*10] % 256,
            iter % 128,
            (iter * 2) % 128,
            (iter * 3) % 128,
            (iter * 5) % 128,
            (iter * 7) % 128,
            (iter * 11) % 128
        );
        total_checksum += r1;
        
        /* Test 2: Complex CFG */
        int r2 = complex_cfg_test(
            data1[iter*20] % 200,
            data2[iter*20] % 200,
            iter % 150
        );
        total_checksum += r2;
        
        /* Test 3: Inline assembly register pressure */
        asm_register_pressure(&data1[iter*50], 100);
        total_checksum += g_volatile_counter;
        
        /* Test 4: Many arguments */
        long r4 = many_arguments_test(
            data1[iter] % 50, data2[iter] % 50,
            (iter+1) % 50, (iter+2) % 50, (iter+3) % 50,
            (iter+4) % 50, (iter+5) % 50, (iter+6) % 50,
            (iter+7) % 50, (iter+8) % 50, (iter+9) % 50,
            (iter+10) % 50, (iter+11) % 50, (iter+12) % 50,
            (iter+13) % 50,
            (double)(iter % 100) * 0.1,
            (double)((iter+1) % 100) * 0.2,
            (double)((iter+2) % 100) * 0.3,
            (double)((iter+3) % 100) * 0.4,
            (float)(iter % 50) * 0.5f,
            (float)((iter+1) % 50) * 0.6f,
            (float)((iter+2) % 50) * 0.7f
        );
        total_checksum += r4;
        
        /* Test 5: Vector operations */
        vector_operations(&data1[iter*100], &data2[iter*100], 
                         &result[iter*100], 100);
        
        /* Compute checksum from results */
        for (int i = 0; i < 100; i++) {
            total_checksum += result[iter*100 + i];
        }
        
        /* Progress indicator */
        if (iter % 10 == 0) {
            printf("Iteration %d, checksum: %ld\n", iter, total_checksum);
        }
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final verification */
    printf("Final checksum: %ld\n", total_checksum);
    printf("Test completed.\n");
    
    free(data1);
    free(data2);
    free(result);
    
    return 0;
}
