/* test_scheduler_context.c
 * Designed to trigger GCC's scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive_deps(int a, int b, int c, int d, int e) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Create true data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 | 0xFF;
    v7 = v6 & 0x0F;
    v8 = v7 ^ v1;
    v9 = v8 + v2;
    v10 = v9 - v3;
    
    /* Anti-dependencies (WAR) and output dependencies (WAW) */
    v11 = v10 * 3;
    v12 = v11 + v4;
    v13 = v12;  /* WAW if v13 was used before */
    v14 = v13 - v5;
    v15 = v14 | v6;
    
    /* More complex dependency chain */
    v16 = (v15 >> 1) + v7;
    v17 = v16 * v8;
    v18 = v17 / (v9 + 1);
    v19 = v18 ^ v10;
    v20 = v19 & v11;
    
    /* Use inline assembly to create artificial dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 + v12;
    v22 = v21 * v13;
    v23 = v22 - v14;
    v24 = v23 | v15;
    v25 = v24 ^ v16;
    
    /* Control flow to create basic block boundaries */
    if (v25 > 1000) {
        v26 = v25 / 2;
        v27 = v26 * 3;
    } else {
        v26 = v25 * 2;
        v27 = v26 / 3;
    }
    
    v28 = v27 + v17;
    v29 = v28 - v18;
    v30 = v29 ^ v19;
    
    /* Final computation using most variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_loops(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    int i, j;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    for (i = 2; i < 32; i++) {
        /* True dependencies in array accesses */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
    }
    
    /* Nested loops with mixed operations */
    for (i = 0; i < iterations; i++) {
        float temp = 0.0f;
        for (j = 0; j < 16; j++) {
            /* Complex floating-point operations */
            temp += arr[j] * sinf(arr[31-j] * 0.01f * i);
        }
        
        /* Conditional inside loop */
        if (temp > 0) {
            sum += sqrtf(temp);
        } else {
            sum -= sqrtf(-temp);
        }
        
        /* Update array with anti-dependencies */
        for (j = 1; j < 32; j++) {
            arr[j-1] = arr[j] * 0.99f;  /* WAR: arr[j] read before arr[j-1] written */
        }
        arr[31] = temp * 0.1f;
    }
    
    /* Inline assembly barrier */
    asm volatile("" : "+r"(sum));
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
double func3_mixed_control(int mode, double x, double y) {
    /* Many local variables of different types */
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5;
    
    /* Initial computations */
    d1 = x + y;
    d2 = x - y;
    d3 = x * y;
    d4 = x / (y + 1.0);
    
    i1 = (int)x;
    i2 = (int)y;
    i3 = i1 * i2;
    i4 = i1 + i2;
    
    f1 = (float)d1;
    f2 = (float)d2;
    f3 = f1 * f2;
    
    /* Complex control flow */
    switch (mode % 5) {
        case 0:
            d5 = sin(d1) * cos(d2);
            i5 = i3 << 2;
            f4 = f3 * 2.0f;
            break;
        case 1:
            d5 = exp(d1) * log(fabs(d2) + 1.0);
            i5 = i4 >> 1;
            f4 = sqrtf(f3);
            break;
        case 2:
            d5 = d1 * d1 + d2 * d2;
            i5 = i3 ^ i4;
            f4 = f1 / f2;
            break;
        case 3:
            d5 = pow(d1, 2.5) + pow(d2, 1.5);
            i5 = i3 | i4;
            f4 = fabsf(f3);
            break;
        default:
            d5 = d3 + d4;
            i5 = i3 & i4;
            f4 = f3 + 1.0f;
            break;
    }
    
    /* More operations after switch */
    d6 = d5 * 1.1;
    i6 = i5 * 3;
    f5 = f4 * 1.5f;
    
    d7 = d6 + (double)i6;
    d8 = d7 * (double)f5;
    
    /* Another conditional */
    if (d8 > 100.0) {
        d9 = d8 / 10.0;
        i7 = i6 / 2;
    } else {
        d9 = d8 * 10.0;
        i7 = i6 * 2;
    }
    
    d10 = d9 + (double)(i7 % 100);
    
    /* Use inline assembly to prevent reordering */
    asm volatile("" : "+r"(d10));
    
    return d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
           i1 + i2 + i3 + i4 + i5 + i6 + i7 +
           f1 + f2 + f3 + f4 + f5;
}

/* Function 4: Complex switch with different operation blocks */
long func4_switch_blocks(int selector, long seed) {
    long result = seed;
    int i;
    
    /* Large switch statement creating multiple basic blocks */
    switch (selector & 0x7) {  /* 8 cases */
        case 0: {
            /* Block with integer dependencies */
            long a = seed * 3;
            long b = a + 17;
            long c = b ^ 0xABCD;
            long d = c << 3;
            result = a + b + c + d;
            break;
        }
        case 1: {
            /* Block with memory operations */
            long arr[8];
            for (i = 0; i < 8; i++) {
                arr[i] = seed * i;
            }
            for (i = 1; i < 8; i++) {
                arr[i] += arr[i-1];
            }
            result = arr[7];
            break;
        }
        case 2: {
            /* Block with mixed operations */
            double x = (double)seed;
            double y = x * 1.234;
            double z = y / 0.567;
            result = (long)(x + y + z);
            break;
        }
        case 3: {
            /* Block with bit operations */
            result = seed;
            for (i = 0; i < 16; i++) {
                result = (result << 1) | ((result >> 31) & 1);
                result ^= 0x9E3779B9;
            }
            break;
        }
        case 4: {
            /* Block with conditional chain */
            long tmp = seed;
            if (tmp & 1) tmp *= 3;
            if (tmp & 2) tmp += 11;
            if (tmp & 4) tmp ^= 0xFF;
            if (tmp & 8) tmp >>= 2;
            result = tmp;
            break;
        }
        case 5: {
            /* Block with small loop */
            result = 0;
            for (i = 0; i < 20; i++) {
                result += seed * i;
                if (i % 3 == 0) result -= i;
            }
            break;
        }
        case 6: {
            /* Block with function calls */
            result = labs(seed) * 2;
            result += (long)sqrt((double)labs(seed));
            break;
        }
        case 7: {
            /* Block with inline assembly */
            long a = seed;
            asm volatile(
                "mov %1, %%rax\n\t"
                "imul $137, %%rax\n\t"
                "mov %%rax, %0\n\t"
                : "=r"(result)
                : "r"(a)
                : "%rax"
            );
            result &= 0x7FFFFFFF;
            break;
        }
    }
    
    /* Post-switch operations */
    result = result * 1103515245 + 12345;
    
    /* Inline assembly barrier */
    asm volatile("" : "+r"(result));
    
    return result & 0x7FFFFFFF;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    int result1, result4;
    float result2;
    double result3;
    long result5;
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    /* Use command line arguments or defaults to create dynamic inputs */
    int input1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int input2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float input3 = (argc > 3) ? atof(argv[3]) : g_input3;
    float input4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    printf("Starting scheduler test with inputs: %d, %d, %f, %f\n",
           input1, input2, input3, input4);
    
    /* Call all functions to trigger scheduler in different contexts */
    result1 = func1_intensive_deps(input1, input2, input1+input2, 
                                   input1-input2, input1*input2);
    printf("func1 result: %d\n", result1);
    sink += result1;
    
    result2 = func2_fp_loops(input3, 5 + (input1 % 10));
    printf("func2 result: %f\n", result2);
    sink += (int)result2;
    
    result3 = func3_mixed_control(input1 % 10, (double)input3, 
                                  (double)input4);
    printf("func3 result: %f\n", result3);
    sink += (int)result3;
    
    result4 = (int)func4_switch_blocks(input1, input2);
    printf("func4 result: %d\n", result4);
    sink += result4;
    
    /* Call functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        sink += func1_intensive_deps(input1 + i, input2 - i, 
                                     input1 * i, input2 + i, i + 1);
        sink += (int)func2_fp_loops(input3 + i, 3);
        sink += (int)func3_mixed_control((input1 + i) % 5, 
                                         (double)i, (double)(i+1));
        sink += func4_switch_blocks(i, input1 + input2);
    }
    
    printf("Final checksum: %d\n", sink);
    
    /* Use sink to prevent optimization */
    asm volatile("" : : "r"(sink));
    
    return 0;
}
