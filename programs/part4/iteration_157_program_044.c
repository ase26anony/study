/* test_scheduler_context.c
 * Designed to trigger GCC's scheduler context allocation and cleanup
 * Compile with: gcc -O3 -fschedule-insns2 -fdump-rtl-sched2 test_scheduler_context.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Volatile variables to prevent constant propagation */
volatile int g_input1 = 42;
volatile int g_input2 = 17;
volatile float g_input3 = 3.14159f;

/* Function 1: Integer-heavy computation with many serial dependencies */
int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Create register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* True data dependencies (RAW) */
    v1 = a + b;
    v2 = v1 * c;
    v3 = v2 - d;
    v4 = v3 / e;
    v5 = v4 << 2;
    v6 = v5 ^ v1;
    
    /* Anti-dependencies (WAR) */
    v7 = v6 + 1;
    v6 = v7 * 2;  /* WAR: v6 written after v7 read */
    
    /* Output dependencies (WAW) */
    v8 = v7 + v3;
    v8 = v8 * 3;   /* WAW: v8 written twice */
    
    /* More complex dependency chain */
    v9 = v8 & 0xFF;
    v10 = v9 | v4;
    v11 = v10 << v5;
    v12 = v11 >> 1;
    v13 = v12 + v6;
    v14 = v13 - v7;
    v15 = v14 * v8;
    v16 = v15 / v9;
    v17 = v16 ^ v10;
    v18 = v17 & 0xFFFF;
    v19 = v18 | v11;
    v20 = v19 << 3;
    
    /* Control flow to create multiple basic blocks */
    if (v20 > 1000) {
        v21 = v20 * 2;
        v22 = v21 + v12;
        v23 = v22 - v13;
        
        /* Inline assembly to create artificial dependencies */
        asm volatile("" : "+r"(v23));
        
        v24 = v23 * 3;
        v25 = v24 / 4;
    } else {
        v21 = v20 / 2;
        v22 = v21 - v12;
        v23 = v22 + v13;
        
        asm volatile("" : "+r"(v23));
        
        v24 = v23 + 5;
        v25 = v24 * 6;
    }
    
    /* Continue dependency chain */
    v26 = v25 + v14;
    v27 = v26 - v15;
    v28 = v27 * v16;
    v29 = v28 / v17;
    v30 = v29 ^ v18;
    
    /* Final computation using all variables to prevent dead store elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return result;
}

/* Function 2: Floating-point array processing with loops */
float fp_array_processing(float base, int iterations) {
    float arr[32];
    float sum = 0.0f;
    
    /* Initialize array with dependencies */
    arr[0] = base;
    arr[1] = base * 1.1f;
    
    for (int i = 2; i < 32; i++) {
        /* Memory dependencies with address calculations */
        arr[i] = arr[i-1] + arr[i-2] * 0.5f;
        
        /* Mixed operations */
        if (i % 3 == 0) {
            arr[i] = sinf(arr[i]) * cosf(arr[i]);
        } else if (i % 3 == 1) {
            arr[i] = sqrtf(fabsf(arr[i]));
        }
        
        /* Inline assembly barrier */
        asm volatile("" : "+m"(arr[i]));
    }
    
    /* Process array with loop-carried dependencies */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 32;
        sum += arr[idx] * (i + 1);
        
        /* Update array element creating output dependency */
        arr[idx] = sum * 0.01f;
        
        /* More complex floating point operations */
        if (sum > 100.0f) {
            sum = logf(fabsf(sum) + 1.0f);
        }
    }
    
    return sum;
}

/* Function 3: Mixed operations with control flow and many local variables */
double mixed_operations(int seed, float factor) {
    /* Many local variables for register pressure */
    double d1 = seed * 1.1, d2 = seed * 2.2, d3 = seed * 3.3;
    double d4, d5, d6, d7, d8, d9, d10;
    float f1 = factor, f2, f3, f4, f5;
    int i1 = seed, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Complex dependency network */
    d4 = d1 * d2 + d3;
    f2 = f1 * 2.0f;
    i2 = i1 << 2;
    
    /* Control flow with dependencies across branches */
    if (d4 > 50.0) {
        d5 = sin(d4) * cos(d4);
        f3 = f2 / 3.0f;
        i3 = i2 ^ 0xAA;
        
        /* Memory operation with address calculation */
        double* ptr = &d5;
        *ptr = *ptr + 1.0;
    } else {
        d5 = exp(d4 * 0.1);
        f3 = f2 * 3.0f;
        i3 = i2 | 0x55;
    }
    
    /* Switch statement creating multiple basic blocks */
    switch (seed % 4) {
        case 0:
            d6 = d5 * 2.0;
            f4 = f3 + 10.0f;
            i4 = i3 * 3;
            break;
        case 1:
            d6 = d5 / 2.0;
            f4 = f3 - 10.0f;
            i4 = i3 / 3;
            break;
        case 2:
            d6 = d5 + 100.0;
            f4 = f3 * 1.5f;
            i4 = i3 + 777;
            break;
        default:
            d6 = d5 - 100.0;
            f4 = f3 / 1.5f;
            i4 = i3 - 777;
            break;
    }
    
    /* Continue dependency chain */
    d7 = d6 * f4;
    i5 = i4 + (int)d7;
    f5 = f4 * i5;
    d8 = d7 / f5;
    i6 = i5 ^ (int)d8;
    d9 = d8 * i6;
    i7 = i6 + (int)d9;
    d10 = d9 / (i7 + 1);
    
    /* Use all variables to prevent optimization */
    double result = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
                    f1 + f2 + f3 + f4 + f5 +
                    i1 + i2 + i3 + i4 + i5 + i6 + i7;
    
    return result;
}

/* Function 4: Nested loops with complex index calculations */
long nested_loop_computation(int size) {
    long matrix[8][8];
    long total = 0;
    
    /* Initialize matrix with dependencies */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == 0 && j == 0) {
                matrix[i][j] = size;
            } else if (j == 0) {
                matrix[i][j] = matrix[i-1][7] * 3 + 1;
            } else {
                matrix[i][j] = matrix[i][j-1] * 2 - 1;
            }
            
            /* Inline assembly to prevent reordering */
            asm volatile("" : "+m"(matrix[i][j]));
        }
    }
    
    /* Process matrix with multiple passes */
    for (int pass = 0; pass < 3; pass++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                /* Complex index calculations */
                int idx1 = (i * 3 + j * 7) % 8;
                int idx2 = (i * 5 + j * 11) % 8;
                
                /* Multiple memory accesses with dependencies */
                long val1 = matrix[i][j];
                long val2 = matrix[idx1][idx2];
                
                /* Conditional update */
                if ((val1 ^ val2) & 1) {
                    matrix[i][j] = val1 * val2 + pass;
                } else {
                    matrix[i][j] = val1 / (val2 + 1) - pass;
                }
                
                /* Accumulate with dependency */
                total += matrix[i][j] * (i + j + 1);
                
                /* Another asm barrier */
                asm volatile("" : "+r"(total));
            }
        }
    }
    
    return total;
}

int main(int argc, char** argv) {
    /* Read inputs to prevent constant propagation */
    int input1 = g_input1;
    int input2 = g_input2;
    float input3 = g_input3;
    
    if (argc > 1) {
        input1 = atoi(argv[1]);
        if (argc > 2) {
            input2 = atoi(argv[2]);
            if (argc > 3) {
                input3 = atof(argv[3]);
            }
        }
    }
    
    /* Call all complex functions to trigger scheduler */
    int result1 = complex_int_chain(input1, input2, input1+1, input2-1, 5);
    float result2 = fp_array_processing(input3, 50);
    double result3 = mixed_operations(input1 ^ input2, input3 * 2.0f);
    long result4 = nested_loop_computation(input1 + input2);
    
    /* Aggregate results to volatile sink to prevent dead code elimination */
    volatile long final_result = 0;
    final_result += result1;
    final_result += (long)result2;
    final_result += (long)result3;
    final_result += result4;
    
    /* Print checksum to ensure all computations are executed */
    printf("Checksum: %ld\n", final_result);
    
    return 0;
}
