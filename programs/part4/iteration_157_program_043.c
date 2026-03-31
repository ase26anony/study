/* test_sched_context.c - Coverage test for haifa-sched.cc free_sched_context */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive(int a, int b, int c, int d, int e) {
    /* Create register pressure with many variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
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
    
    /* Anti-dependencies (WAR) */
    v11 = v4 + 1;  /* Reads v4 */
    v4 = v11 * 2;  /* Writes v4 after read */
    
    /* Output dependencies (WAW) */
    v12 = v5 + v6;
    v12 = v7 * v8; /* Overwrites v12 */
    
    /* More operations to increase block size */
    v13 = (v1 + v2) * (v3 - v4);
    v14 = v13 % (v5 + 1);
    v15 = v14 | v6;
    v16 = v15 ^ v7;
    v17 = v16 << v8;
    v18 = v17 >> 1;
    v19 = v18 + v9;
    v20 = v19 - v10;
    
    /* Use inline assembly to create opaque dependencies */
    asm volatile("" : "+r"(v20));
    
    v21 = v20 * 3;
    v22 = v21 / 2;
    v23 = v22 + v11;
    v24 = v23 - v12;
    v25 = v24 | v13;
    v26 = v25 & v14;
    v27 = v26 ^ v15;
    v28 = v27 << 2;
    v29 = v28 >> 1;
    v30 = v29 + v16;
    
    /* Control flow to create multiple basic blocks */
    if (v30 > 1000) {
        /* Branch with its own dependencies */
        v17 = v30 * 2;
        v18 = v17 - 50;
        v19 = v18 / 3;
        v20 = v19 + v30;
    } else {
        /* Alternative branch */
        v17 = v30 / 2;
        v18 = v17 + 100;
        v19 = v18 * 3;
        v20 = v19 - v30;
    }
    
    /* Final computation using many variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function 2: Floating-point array processing with loops */
double func2_fp_loop(double base, int iterations) {
    double arr[20];
    double sum = 0.0;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1;
    for (int i = 2; i < 20; i++) {
        /* RAW dependency through array */
        arr[i] = arr[i-1] + arr[i-2] * 0.5;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double temp = 0.0;
        
        /* Multiple FP operations with dependencies */
        temp = sin(arr[i % 20]) * cos(arr[(i+1) % 20]);
        temp = temp + tan(arr[(i+2) % 20]);
        temp = exp(temp * 0.01);
        
        /* Conditional inside loop */
        if (temp > 1.0) {
            temp = log(temp);
            arr[i % 20] = temp * 0.9;
        } else {
            temp = sqrt(temp + 1.0);
            arr[i % 20] = temp * 1.1;
        }
        
        sum += temp;
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(i));
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many locals */
long func3_mixed_control(int x, int y, double z) {
    /* Many local variables for register pressure */
    long l1 = x * 100L, l2 = y * 200L, l3 = 0, l4 = 0, l5 = 0;
    double d1 = z, d2 = z * 2.0, d3 = 0.0, d4 = 0.0;
    int i1 = x, i2 = y, i3 = 0, i4 = 0, i5 = 0, i6 = 0;
    
    /* Complex if-else chain creating multiple basic blocks */
    if (x > y) {
        /* Block A: Integer operations */
        i3 = i1 * i2;
        i4 = i3 << 3;
        i5 = i4 / (i1 + 1);
        d3 = d1 * d2;
        l3 = (long)(d3 * 1000.0);
    } else if (x < y) {
        /* Block B: Different operations */
        i3 = i2 - i1;
        i4 = i3 * 7;
        i5 = i4 >> 2;
        d3 = d2 / d1;
        l3 = (long)(d3 * 500.0);
    } else {
        /* Block C: Mixed operations */
        i3 = i1 + i2;
        i4 = i3 * i3;
        i5 = i4 % 17;
        d3 = sqrt(d1 * d2);
        l3 = (long)(d3 * 250.0);
    }
    
    /* Switch statement for more control flow */
    switch (i5 % 4) {
        case 0:
            l4 = l1 + l2 + l3;
            d4 = d3 * 3.14159;
            i6 = i4 * 2;
            break;
        case 1:
            l4 = l1 - l2 + l3;
            d4 = d3 / 2.71828;
            i6 = i4 / 2;
            break;
        case 2:
            l4 = l1 * l2 - l3;
            d4 = d3 + 1.41421;
            i6 = i4 + 100;
            break;
        case 3:
            l4 = l2 / (l1 + 1) + l3;
            d4 = d3 - 0.57721;
            i6 = i4 - 50;
            break;
    }
    
    /* Final computation using all variables */
    l5 = l3 + l4 + (long)d4 + i6;
    
    /* More operations to increase block size */
    for (int i = 0; i < 5; i++) {
        l5 = l5 * 2 - i;
        d4 = d4 * 1.1;
        i6 = i6 + (i5 % 3);
    }
    
    return l5;
}

/* Function 4: Switch with different operation blocks */
int func4_switch_blocks(int mode, int val) {
    int result = val;
    
    switch (mode % 5) {
        case 0: {
            /* Block with arithmetic sequence */
            int a = val, b = val * 2, c = val * 3;
            for (int i = 0; i < 10; i++) {
                a = a + b;
                b = b - c;
                c = c * 2;
                result += a + b + c;
            }
            break;
        }
        case 1: {
            /* Block with bit operations */
            unsigned int u1 = val, u2 = ~val, u3 = 0;
            for (int i = 0; i < 8; i++) {
                u3 = (u1 << i) | (u2 >> (32 - i));
                u1 = u1 ^ u3;
                u2 = u2 & u3;
                result ^= u1 + u2 + u3;
            }
            break;
        }
        case 2: {
            /* Block with mixed int/float */
            double d = val * 1.5;
            int accum = 0;
            for (int i = 0; i < 6; i++) {
                d = d * 1.1 + i;
                accum += (int)d;
                result += accum * i;
            }
            break;
        }
        case 3: {
            /* Block with memory access pattern */
            int arr[10];
            for (int i = 0; i < 10; i++) {
                arr[i] = val + i;
            }
            for (int i = 1; i < 10; i++) {
                arr[i] = arr[i] + arr[i-1];
                result += arr[i];
            }
            break;
        }
        case 4: {
            /* Block with nested conditionals */
            int tmp = val;
            for (int i = 0; i < 12; i++) {
                if (tmp % 2 == 0) {
                    tmp = tmp / 2 + i;
                } else {
                    tmp = tmp * 3 + 1;
                }
                if (tmp > 1000) {
                    tmp = tmp % 100;
                }
                result += tmp;
            }
            break;
        }
    }
    
    return result;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int input1, input2, input3;
    volatile double input4;
    
    /* Read inputs to make values dynamic */
    if (argc > 3) {
        input1 = atoi(argv[1]);
        input2 = atoi(argv[2]);
        input3 = atoi(argv[3]);
        input4 = atof(argv[4]);
    } else {
        /* Default values if no args */
        input1 = 42;
        input2 = 17;
        input3 = 100;
        input4 = 3.14159;
    }
    
    /* Call all functions to trigger scheduling */
    int result1 = func1_intensive(input1, input2, input3, input1 + input2, input2 + input3);
    double result2 = func2_fp_loop(input4, 25);
    long result3 = func3_mixed_control(input1, input2, input4);
    int result4 = func4_switch_blocks(input1, input3);
    
    /* Aggregate results to prevent dead code elimination */
    volatile long final_result = 0;
    final_result = result1 + (long)result2 + result3 + result4;
    
    /* Print to ensure code isn't optimized away */
    printf("Results: %d, %.2f, %ld, %d\n", result1, result2, result3, result4);
    printf("Final aggregate: %ld\n", final_result);
    
    return 0;
}
