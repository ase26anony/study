/* mcf_test.c - Test program to trigger min-cost flow debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define ITERATIONS 100
#define NUM_CASES 15

/* Complex control flow with many live ranges */
int test_complex_cfg(int *data, int size) {
    volatile int keep_alive = 0;  /* Force register preservation */
    int sum = 0;
    int i, j, k;
    
    /* Variables declared at function scope to extend live ranges */
    int temp1, temp2, temp3, temp4, temp5;
    int *ptr1 = &temp1;
    int *ptr2 = &temp2;
    
    /* Deeply nested loops creating register pressure */
    for (i = 0; i < size; i++) {
        temp1 = data[i];
        
        /* Complex expression with many intermediates */
        temp2 = temp1 * temp1;
        temp3 = temp2 + (temp1 << 3);
        temp4 = temp3 / (temp1 + 1);
        temp5 = temp4 - (temp2 >> 2);
        
        /* Multiple nested conditionals */
        if (temp1 > 1000) {
            if (temp2 < 500000) {
                temp3 = temp3 * 2;
                if (temp4 > 100) {
                    temp5 = temp5 + temp1;
                } else {
                    temp5 = temp5 - temp1;
                    if (temp5 < 0) {
                        temp5 = -temp5;
                        goto early_exit;
                    }
                }
            } else if (temp2 > 1000000) {
                for (j = 0; j < 10; j++) {
                    temp3 += j;
                    for (k = 0; k < 5; k++) {
                        temp4 += (temp3 * k) >> 1;
                    }
                }
            }
        } else if (temp1 < -1000) {
            switch (temp1 % 10) {
                case 0: temp2 = -temp2; break;
                case 1: temp2 = temp2 >> 1; break;
                case 2: temp2 = temp2 << 1; break;
                case 3: temp2 = temp2 * 3; break;
                case 4: temp2 = temp2 / 2; break;
                case 5: temp2 = temp2 + 100; break;
                case 6: temp2 = temp2 - 100; break;
                case 7: temp2 = temp2 ^ 0xFF; break;
                case 8: temp2 = temp2 | 0xAA; break;
                case 9: temp2 = temp2 & 0x55; break;
                default: temp2 = 0; break;
            }
        }
        
        early_exit:
        sum += temp5;
        
        /* Inline assembly with register constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (keep_alive)
            : "r" (temp1)
            : "%eax", "memory"
        );
    }
    
    /* Force use of all temporaries before return */
    *ptr1 = temp1;
    *ptr2 = temp2;
    return sum + keep_alive + *ptr1 + *ptr2;
}

/* Function with switch statement creating complex CFG */
double test_switch_cfg(double *data, int size) {
    double result = 0.0;
    int i;
    
    for (i = 0; i < size; i++) {
        double x = data[i];
        double y = 0.0;
        
        /* Large switch with fall-through cases */
        switch ((int)x % NUM_CASES) {
            case 0:
                y = sin(x);
                /* fall through */
            case 1:
                y += cos(x);
                break;
            case 2:
                y = tan(x);
                if (y > 1.0) goto case_3;
                break;
            case 3:
            case_3:
                y = exp(x);
                break;
            case 4:
                y = log(fabs(x) + 1.0);
                break;
            case 5:
                y = sqrt(fabs(x));
                break;
            case 6:
                y = x * x;
                break;
            case 7:
                y = x * x * x;
                break;
            case 8:
                y = 1.0 / (x + 1.0);
                break;
            case 9:
                y = sin(x) * cos(x);
                break;
            case 10:
                y = tanh(x);
                break;
            case 11:
                y = asin(fmod(x, 1.0));
                break;
            case 12:
                y = acos(fmod(x, 1.0));
                break;
            case 13:
                y = atan(x);
                break;
            case 14:
                y = sinh(x) + cosh(x);
                break;
            default:
                y = x;
        }
        
        /* Complex expression using multiple FP registers */
        result += y * y - 2.0 * x * y + x * x;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Function with many function calls in loops */
long test_function_calls(int *data, int size) {
    long total = 0;
    int i, j;
    
    /* Helper functions that get inlined or not */
    auto int helper1(int a, int b) { return a * b + (a >> b); }
    auto int helper2(int a, int b) { return (a & b) | (a ^ b); }
    auto int helper3(int a, int b) { return a + b * 3 - (a % (b + 1)); }
    auto int helper4(int a, int b) { return (a << 3) | (b >> 2); }
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        int acc = val;
        
        /* Many function calls creating register pressure */
        for (j = 0; j < 8; j++) {
            acc = helper1(acc, j);
            acc = helper2(acc, val);
            acc = helper3(acc, i);
            acc = helper4(acc, size - i);
            
            /* Inline assembly with fixed registers */
            asm volatile (
                "movl %1, %%ebx\n\t"
                "imull %%ebx, %%eax\n\t"
                : "=a" (acc)
                : "r" (j), "0" (acc)
                : "%ebx", "cc"
            );
        }
        
        total += acc;
    }
    
    return total;
}

/* Mixed data types stressing different register classes */
float test_mixed_types(char *cdata, short *sdata, int *idata, 
                       float *fdata, double *ddata, int size) {
    float fsum = 0.0f;
    double dsum = 0.0;
    int isum = 0;
    short ssum = 0;
    char csum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Operations on different data types */
        char c = cdata[i];
        short s = sdata[i];
        int i_val = idata[i];
        float f = fdata[i];
        double d = ddata[i];
        
        /* Mixed type computations */
        fsum += f + (float)c + (float)s + (float)i_val;
        dsum += d + (double)f + (double)i_val;
        isum += i_val + (int)c + (int)s + (int)f;
        ssum += s + (short)c + (short)(i_val & 0xFFFF);
        csum += c + (char)(i_val & 0xFF);
        
        /* Complex address calculations */
        float *fptr = &fdata[(i * 13) % size];
        double *dptr = &ddata[(i * 17) % size];
        int *iptr = &idata[(i * 19) % size];
        
        *fptr = *fptr * 0.99f;
        *dptr = *dptr * 0.999;
        *iptr = *iptr + 1;
        
        /* Vector-like operations */
        asm volatile (
            "movss %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m" (*fptr)
            : "m" (*fptr), "m" (fsum)
            : "%xmm0"
        );
    }
    
    return fsum + (float)dsum + (float)isum + (float)ssum + (float)csum;
}

/* Function with many arguments passed in registers and on stack */
long test_many_args(int a1, int a2, int a3, int a4, int a5,
                    int a6, int a7, int a8, int a9, int a10,
                    int a11, int a12, int a13, int a14, int a15) {
    /* Use all arguments in complex ways */
    int t1 = a1 * a2 + a3;
    int t2 = a4 - a5 * a6;
    int t3 = a7 & a8 | a9;
    int t4 = a10 ^ a11 << a12;
    int t5 = a13 + a14 - a15;
    
    /* Create register pressure with many live values */
    int r1 = t1 + t2;
    int r2 = t3 - t4;
    int r3 = t5 * t1;
    int r4 = t2 / (t3 + 1);
    int r5 = t4 | t5;
    int r6 = t1 & t2;
    int r7 = t3 ^ t4;
    int r8 = t5 << 2;
    int r9 = t1 >> 1;
    int r10 = t2 * t3;
    
    /* All values must stay alive */
    volatile int preserve __attribute__((unused));
    preserve = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Complex control flow with early returns */
    if (r1 > 1000) {
        if (r2 < 0) {
            return r3 + r4;
        } else if (r5 > 500) {
            return r6 - r7;
        }
    } else if (r8 < 100) {
        for (int i = 0; i < 10; i++) {
            r9 += i;
            if (r9 > 50) break;
            r10 -= i;
        }
        return r9 * r10;
    }
    
    /* Default return with all values used */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

/* Main test driver */
int main() {
    int i;
    long total_result = 0;
    
    /* Initialize large arrays with random data */
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    double *double_data = malloc(ARRAY_SIZE * sizeof(double));
    char *char_data = malloc(ARRAY_SIZE * sizeof(char));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 10000 - 5000;
        double_data[i] = (rand() % 10000 - 5000) / 100.0;
        char_data[i] = rand() % 256 - 128;
        short_data[i] = rand() % 65536 - 32768;
        float_data[i] = (rand() % 10000 - 5000) / 100.0f;
    }
    
    printf("Starting register pressure tests...\n");
    
    /* Warm-up iterations for profile feedback */
    for (i = 0; i < 10; i++) {
        int warmup = test_complex_cfg(int_data, 100);
        asm volatile("" ::: "memory");  /* Memory barrier */
        total_result += warmup;
    }
    
    /* Run tests that stress different aspects */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Complex CFG with many live ranges */
        int result1 = test_complex_cfg(int_data, ARRAY_SIZE / 10);
        total_result += result1;
        
        /* Memory barrier between tests */
        asm volatile("" ::: "memory");
        
        /* Test 2: Switch-based CFG */
        double result2 = test_switch_cfg(double_data, ARRAY_SIZE / 20);
        total_result += (long)result2;
        
        asm volatile("" ::: "memory");
        
        /* Test 3: Many function calls */
        long result3 = test_function_calls(int_data, ARRAY_SIZE / 30);
        total_result += result3;
        
        asm volatile("" ::: "memory");
        
        /* Test 4: Mixed data types */
        float result4 = test_mixed_types(char_data, short_data, int_data,
                                        float_data, double_data, ARRAY_SIZE / 40);
        total_result += (long)result4;
        
        asm volatile("" ::: "memory");
        
        /* Test 5: Many arguments */
        long result5 = test_many_args(
            i, i*2, i*3, i*4, i*5,
            i*6, i*7, i*8, i*9, i*10,
            i*11, i*12, i*13, i*14, i*15
        );
        total_result += result5;
        
        /* Progress indicator */
        if (i % 10 == 0) {
            printf("Iteration %d/%d, running total: %ld\n", 
                   i, ITERATIONS, total_result);
        }
    }
    
    /* Clean up */
    free(int_data);
    free(double_data);
    free(char_data);
    free(short_data);
    free(float_data);
    
    printf("Final result: %ld\n", total_result);
    printf("Test completed successfully.\n");
    
    return 0;
}
