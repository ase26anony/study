/* test_scheduler_context.c
 * Complex program to trigger scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_input1 = 42;
volatile int g_input2 = 73;
volatile float g_input3 = 3.14159f;
volatile float g_input4 = 2.71828f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int func1_intensive_deps(int a, int b, int c, int d, int e, int f) {
    /* Many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Long dependency chain with RAW dependencies */
    v1 = a + b;          /* Start of chain */
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 ^ f;
    v6 = v5 | a;
    v7 = v6 & b;
    v8 = v7 << 2;
    v9 = v8 >> 1;
    v10 = v9 + c;
    
    /* Anti-dependencies (WAR) */
    v11 = v10 * 2;       /* Read v10, write v11 */
    v10 = v11 - 1;       /* Read v11, write v10 (anti-dep) */
    
    /* Output dependencies (WAW) */
    v12 = v10 + 5;       /* Write v12 */
    v12 = v12 * 3;       /* Write v12 again (output dep) */
    
    /* More operations to increase block size */
    v13 = v12 % 7;
    v14 = v13 * v1;
    v15 = v14 - v2;
    v16 = v15 + v3;
    v17 = v16 ^ v4;
    v18 = v17 | v5;
    v19 = v18 & v6;
    v20 = v19 << v7;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        /* True path with more dependencies */
        v21 = v20 / 2;
        v22 = v21 * 3;
        v23 = v22 - 4;
        v24 = v23 + 5;
        
        /* Inline assembly to create artificial dependencies */
        asm volatile("" : "+r"(v24));
        
        v25 = v24 ^ 0xFF;
        v26 = v25 | 0xAA;
        v27 = v26 & 0x55;
    } else {
        /* False path with different operations */
        v21 = v20 * 2;
        v22 = v21 / 3;
        v23 = v22 + 4;
        v24 = v23 - 5;
        
        /* Another inline assembly barrier */
        asm volatile("" : "+r"(v24));
        
        v25 = v24 << 2;
        v26 = v25 >> 1;
        v27 = v26 % 13;
    }
    
    /* Merge point - more operations */
    v28 = v27 + v8;
    v29 = v28 * v9;
    v30 = v29 - v10;
    
    /* Final computation using all variables */
    return v30 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
}

/* Function 2: Floating-point array processing with loops */
float func2_fp_arrays(float base, int iterations) {
    /* Many local FP variables for register pressure */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    float f11, f12, f13, f14, f15, f16, f17, f18, f19, f20;
    float arr[20];
    float result = 0.0f;
    
    /* Initialize array with dependencies */
    f1 = base;
    for (int i = 0; i < 20; i++) {
        arr[i] = f1;
        f1 = f1 * 1.1f - 0.5f;
    }
    
    /* Complex FP operations with dependencies */
    f2 = arr[0] + arr[1];
    f3 = f2 * arr[2];
    f4 = f3 - arr[3];
    f5 = f4 / arr[4];
    f6 = f5 + arr[5];
    f7 = f6 * arr[6];
    f8 = f7 - arr[7];
    f9 = f8 / arr[8];
    f10 = f9 + arr[9];
    
    /* Loop with internal dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Conditional inside loop creates basic blocks */
        if (i % 3 == 0) {
            f11 = f10 * 2.0f;
            f12 = f11 - 1.0f;
            asm volatile("" : "+r"(f12));
        } else if (i % 3 == 1) {
            f11 = f10 / 2.0f;
            f12 = f11 + 1.0f;
            asm volatile("" : "+r"(f12));
        } else {
            f11 = f10 + 2.0f;
            f12 = f11 * 1.5f;
            asm volatile("" : "+r"(f12));
        }
        
        /* Update result with dependency chain */
        result = result + f12;
        f10 = f12 * 0.9f;
    }
    
    /* More FP operations after loop */
    f13 = result * 0.01f;
    f14 = f13 + arr[10];
    f15 = f14 - arr[11];
    f16 = f15 * arr[12];
    f17 = f16 / arr[13];
    f18 = f17 + arr[14];
    f19 = f18 - arr[15];
    f20 = f19 * arr[16];
    
    return f20 + result;
}

/* Function 3: Mixed operations with control flow and many variables */
long func3_mixed_ops(int a, int b, float c, float d) {
    /* Mixed type variables */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long l1, l2, l3, l4, l5;
    
    /* Integer operations */
    i1 = a + b;
    i2 = i1 * 2;
    i3 = i2 - b;
    i4 = i3 / a;
    i5 = i4 ^ 0xFFFF;
    
    /* Floating point operations */
    f1 = c + d;
    f2 = f1 * 2.0f;
    f3 = f2 - d;
    f4 = f3 / c;
    
    /* Type conversions create interesting dependencies */
    i6 = (int)f1;
    i7 = i6 + i5;
    f5 = (float)i7;
    f6 = f5 * f4;
    
    /* Switch statement creates multiple basic blocks */
    switch (i7 % 4) {
        case 0:
            i8 = i7 * 3;
            f7 = f6 * 3.0f;
            asm volatile("" : "+r"(i8), "+r"(f7));
            break;
        case 1:
            i8 = i7 / 3;
            f7 = f6 / 3.0f;
            asm volatile("" : "+r"(i8), "+r"(f7));
            break;
        case 2:
            i8 = i7 + 3;
            f7 = f6 + 3.0f;
            asm volatile("" : "+r"(i8), "+r"(f7));
            break;
        default:
            i8 = i7 - 3;
            f7 = f6 - 3.0f;
            asm volatile("" : "+r"(i8), "+r"(f7));
            break;
    }
    
    /* More mixed operations */
    i9 = i8 + (int)f7;
    f8 = f7 + (float)i9;
    i10 = i9 * 2;
    f9 = f8 * 2.0f;
    f10 = f9 - 1.0f;
    
    /* Long operations */
    l1 = (long)i10 * 1000L;
    l2 = l1 + (long)f10;
    l3 = l2 * 37L;
    l4 = l3 / 13L;
    l5 = l4 ^ 0xAAAAAAAA;
    
    return l5;
}

/* Function 4: Complex nested loops with memory accesses */
double func4_memory_access(int size, double* data) {
    double temp[50];
    double sum = 0.0;
    
    /* Initialize temp array with computation */
    for (int i = 0; i < 50; i++) {
        temp[i] = (double)i * 1.5;
    }
    
    /* Nested loops with memory dependencies */
    for (int i = 1; i < size && i < 49; i++) {
        /* True memory dependency (RAW) */
        temp[i] = temp[i-1] + data[i];
        
        /* Anti-dependency (WAR) through array */
        double old_val = temp[i];
        temp[i] = temp[i] * 2.0 - 1.0;
        
        /* Output dependency (WAW) */
        if (old_val > 10.0) {
            temp[i] = old_val / 2.0;
        }
        
        /* Inner loop with more computations */
        for (int j = 0; j < 5; j++) {
            sum += temp[i] * j;
            
            /* Conditional in inner loop */
            if ((i + j) % 2 == 0) {
                sum = sum * 1.1;
                asm volatile("" : "+r"(sum));
            } else {
                sum = sum / 1.1;
                asm volatile("" : "+r"(sum));
            }
        }
    }
    
    /* Final computation */
    double result = 0.0;
    for (int i = 0; i < 50; i++) {
        result += temp[i] * i;
    }
    
    return result + sum;
}

/* Main function to drive execution */
int main(int argc, char** argv) {
    /* Use command line args or defaults to prevent constant propagation */
    int input1 = (argc > 1) ? atoi(argv[1]) : g_input1;
    int input2 = (argc > 2) ? atoi(argv[2]) : g_input2;
    float input3 = (argc > 3) ? atof(argv[3]) : g_input3;
    float input4 = (argc > 4) ? atof(argv[4]) : g_input4;
    
    /* Volatile sink to prevent dead code elimination */
    volatile long total_result = 0;
    
    /* Call all complex functions */
    int r1 = func1_intensive_deps(input1, input2, input1+1, input2-1, 
                                  input1*2, input2/2);
    total_result += r1;
    
    float r2 = func2_fp_arrays(input3, 10);
    total_result += (long)r2;
    
    long r3 = func3_mixed_ops(input1, input2, input3, input4);
    total_result += r3;
    
    /* Prepare data for memory function */
    double data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = (double)i * 0.1;
    }
    
    double r4 = func4_memory_access(40, data);
    total_result += (long)r4;
    
    /* Print checksum to ensure all computations are used */
    printf("Result checksum: %ld\n", total_result);
    
    return 0;
}
