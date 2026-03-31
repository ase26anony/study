#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31;
    
    /* Initialize from inputs - creates many live ranges */
    v0 = inputs[0];
    v1 = inputs[1];
    v2 = inputs[2];
    v3 = inputs[3];
    v4 = inputs[4];
    v5 = inputs[5];
    v6 = inputs[6];
    v7 = inputs[7];
    v8 = inputs[8];
    v9 = inputs[9];
    v10 = inputs[10];
    v11 = inputs[11];
    v12 = inputs[12];
    v13 = inputs[13];
    v14 = inputs[14];
    v15 = inputs[15];
    v16 = inputs[16];
    v17 = inputs[17];
    v18 = inputs[18];
    v19 = inputs[19];
    v20 = inputs[20];
    v21 = inputs[21];
    v22 = inputs[22];
    v23 = inputs[23];
    v24 = inputs[24];
    v25 = inputs[25];
    v26 = inputs[26];
    v27 = inputs[27];
    v28 = inputs[28];
    v29 = inputs[29];
    v30 = inputs[30];
    v31 = inputs[31];
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loop */
    int r0 = v0 + 12345;      /* Candidate 1: v0 + constant */
    int r1 = v1 & 0xFFFF00FF; /* Candidate 2: bitwise operation */
    int r2 = v2 << 3;         /* Candidate 3: shift operation */
    int r3 = v3 * 7;          /* Candidate 4: multiplication */
    int r4 = v4 ^ 0xAA55AA55; /* Candidate 5: xor with constant */
    int r5 = v5 | 0x00FF00FF; /* Candidate 6: or with constant */
    int r6 = v6 - 54321;      /* Candidate 7: subtraction */
    int r7 = ~v7;             /* Candidate 8: bitwise not */
    
    /* Complex loop with many live values */
    int sum = 0;
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use the remat candidates inside the loop - they're defined outside */
        /* This creates cross-block liveness */
        int t0 = r0 + i;
        int t1 = r1 - i;
        int t2 = r2 | i;
        int t3 = r3 ^ i;
        int t4 = r4 & i;
        int t5 = r5 << (i & 3);
        int t6 = r6 >> (i & 3);
        int t7 = r7 + i;
        
        /* More computations to increase pressure */
        int a = v8 * v9 + v10;
        int b = v11 / (v12 + 1) - v13;
        int c = v14 << (v15 & 7);
        int d = v16 ^ v17 ^ v18;
        int e = v19 | v20 | v21;
        int f = v22 & v23 & v24;
        int g = v25 - v26 + v27;
        int h = v28 * 3 + v29 * 5;
        
        /* Conditional inside loop creates merging points */
        if (i & 1) {
            /* Use different combinations in each branch */
            sum += t0 + a + c + e + g;
            sum += v30 * i;
        } else {
            sum += t1 + b + d + f + h;
            sum += v31 * i;
        }
        
        /* Use all remat candidates to keep them live */
        sum += t2 + t3 + t4 + t5 + t6 + t7;
        
        /* More independent computations */
        int x1 = v0 * i + v1;
        int x2 = v2 * i - v3;
        int x3 = v4 << (i & 7);
        int x4 = v5 >> (i & 7);
        int x5 = v6 & (0xFF << (i & 7));
        int x6 = v7 | (0xAA << (i & 7));
        int x7 = v8 ^ i;
        int x8 = v9 + (i * 13);
        int x9 = v10 - (i * 17);
        int x10 = v11 * (i + 1);
        int x11 = v12 / ((i & 15) + 1);
        
        /* Use these to prevent dead code elimination */
        sum += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11;
    }
    
    /* Final use of remat candidates outside loop */
    sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    return sum;
}

/* Another function with different pattern to increase complexity */
static int __attribute__((noinline))
nested_pressure(int a, int b, int c, int d, int e, int f, int g, int h)
{
    /* Create many intermediate values */
    int t1 = a * b + 123;
    int t2 = c * d - 456;
    int t3 = e << 2;
    int t4 = f >> 2;
    int t5 = g & 0xFF;
    int t6 = h | 0xFF00;
    int t7 = a ^ b ^ c;
    int t8 = d + e + f;
    int t9 = g * 3 + h * 7;
    int t10 = a & 0x55555555;
    int t11 = b | 0xAAAAAAAA;
    int t12 = c ^ 0x33333333;
    int t13 = d - 1000;
    int t14 = e + 2000;
    int t15 = f * 11;
    int t16 = g / 3;
    
    /* Nested loops with different live sets */
    int result = 0;
    for (int i = 0; i < 50; i++) {
        /* Outer loop uses some values */
        int outer = t1 + t2 + t3 + i;
        
        for (int j = 0; j < 20; j++) {
            /* Inner loop uses different combinations */
            int inner = t4 + t5 + t6 + j;
            
            /* Conditional with many live values */
            if ((i + j) & 1) {
                result += outer + inner + t7 + t8 + t9;
            } else {
                result += outer + inner + t10 + t11 + t12;
            }
            
            /* More computations */
            int tmp1 = t13 * j + t14;
            int tmp2 = t15 / (j + 1) + t16;
            int tmp3 = (a + j) & (b - j);
            int tmp4 = (c * j) | (d << (j & 3));
            int tmp5 = (e >> (j & 3)) ^ (f & j);
            
            result += tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
        }
        
        /* Use remaining values outside inner loop */
        result += t1 * i + t2 - i + t3 ^ i;
    }
    
    return result;
}

int main(void) 
{
    /* Initialize input array with deterministic values */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = i * 3 + 7;
    }
    
    /* Call high-pressure functions */
    int result1 = high_pressure_computation(inputs);
    int result2 = nested_pressure(
        inputs[0], inputs[1], inputs[2], inputs[3],
        inputs[4], inputs[5], inputs[6], inputs[7]
    );
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with expected value for correctness */
    if (final_result == 2147483647) {
        printf("Unexpected exact match - this is unlikely\n");
    }
    
    return 0;
}
