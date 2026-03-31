#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep RTL complex */
__attribute__((noinline))
static unsigned int high_pressure_computation(unsigned int *inputs) {
    /* Many distinct variables to create high register pressure */
    register unsigned int v0, v1, v2, v3, v4, v5, v6, v7;
    register unsigned int v8, v9, v10, v11, v12, v13, v14, v15;
    register unsigned int v16, v17, v18, v19, v20, v21, v22, v23;
    register unsigned int v24, v25, v26, v27, v28, v29, v30, v31;
    
    /* Initialize from inputs to avoid constant propagation */
    v0 = inputs[0] ^ 0x5A5A5A5A;
    v1 = inputs[1] + 0x12345678;
    v2 = inputs[2] * 3;
    v3 = inputs[3] | 0xF0F0F0F0;
    v4 = inputs[4] << 2;
    v5 = inputs[5] >> 1;
    v6 = inputs[6] & 0xCCCCCCCC;
    v7 = inputs[7] - 0x11111111;
    
    /* Create rematerialization candidates - pure functions of inputs */
    unsigned int r0 = v0 + 0x1000;      /* Cheap: v0 + constant */
    unsigned int r1 = v1 & 0x00FFFFFF;  /* Cheap: mask operation */
    unsigned int r2 = v2 << 3;          /* Cheap: shift */
    unsigned int r3 = v3 ^ 0xAAAAAAAA;  /* Cheap: xor with constant */
    unsigned int r4 = v4 | 0x0000FFFF;  /* Cheap: or with constant */
    unsigned int r5 = v5 * 5;           /* Cheap: multiplication by small constant */
    unsigned int r6 = v6 + v7;          /* Cheap: addition */
    unsigned int r7 = v0 ^ v1;          /* Cheap: xor of two values */
    
    /* Keep these results live across many operations */
    /* Use them immediately but also keep them for later */
    v8 = r0 * 2;
    v9 = r1 | 0xFF00;
    v10 = r2 + 0x100;
    v11 = r3 & 0xFFFF0000;
    
    /* More computations creating many live values */
    v12 = inputs[8] + v0;
    v13 = inputs[9] * v1;
    v14 = inputs[10] ^ v2;
    v15 = inputs[11] | v3;
    v16 = inputs[12] << v4;
    v17 = inputs[13] >> v5;
    v18 = inputs[14] & v6;
    v19 = inputs[15] - v7;
    
    /* Use remat candidates again, forcing them to stay live */
    v20 = r4 + v8;
    v21 = r5 ^ v9;
    v22 = r6 | v10;
    v23 = r7 & v11;
    
    /* Complex control flow to create challenging liveness patterns */
    unsigned int sum = 0;
    
    /* Outer loop - remat candidates defined outside, used inside */
    for (unsigned int i = 0; i < 4; i++) {
        /* Use rematerialization candidates inside loop */
        unsigned int t0 = r0 + i;
        unsigned int t1 = r1 * i;
        unsigned int t2 = r2 ^ i;
        unsigned int t3 = r3 | i;
        
        /* Inner loop with more computations */
        for (unsigned int j = 0; j < 3; j++) {
            /* Conditional branch using different variable sets */
            if (j & 1) {
                /* Use one set of variables */
                sum += t0 + v12 + v13 + v14;
                sum ^= t1 + v15 + v16 + v17;
            } else {
                /* Use another set of variables */
                sum |= t2 + v18 + v19 + v20;
                sum &= t3 + v21 + v22 + v23;
            }
            
            /* More computations keeping many values live */
            v24 = v12 * j + v13;
            v25 = v14 ^ j | v15;
            v26 = v16 << (j & 3);
            v27 = v17 >> (j & 7);
            v28 = v18 + j * v19;
            v29 = v20 - j + v21;
            v30 = v22 & (0xFF << j);
            v31 = v23 | (j * 0x01010101);
            
            /* Use all variables to keep them live */
            sum += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
        }
        
        /* Use remat candidates again after inner loop */
        sum += r4 * i + r5 + r6 + r7;
    }
    
    /* Final computation using all rematerialization candidates */
    unsigned int final = 
        (r0 * 0xABCD) ^ 
        (r1 + 0x1234) | 
        (r2 & 0xFEDCBA98) + 
        (r3 << 4) - 
        (r4 >> 2) * 
        (r5 ^ 0x87654321) + 
        (r6 | 0x33333333) & 
        (r7 + 0x44444444);
    
    return sum + final;
}

/* Another high-pressure function to increase complexity */
__attribute__((noinline))
static unsigned int nested_pressure(unsigned int seed) {
    unsigned int a = seed * 0x9E3779B9;
    unsigned int b = seed + 0x6A09E667;
    unsigned int c = seed ^ 0xBB67AE85;
    unsigned int d = seed | 0x3C6EF372;
    
    /* Remat candidates */
    unsigned int ra = a + 0x100;
    unsigned int rb = b & 0xFFFF;
    unsigned int rc = c << 2;
    unsigned int rd = d ^ 0x5555;
    
    /* Many intermediate computations */
    unsigned int results[16];
    for (int i = 0; i < 16; i++) {
        /* Use remat candidates in loop */
        results[i] = ra * i + rb * (i + 1) + rc * (i + 2) + rd * (i + 3);
        
        /* Complex expression with many temporaries */
        unsigned int t1 = a * b + c;
        unsigned int t2 = b * c + d;
        unsigned int t3 = c * d + a;
        unsigned int t4 = d * a + b;
        unsigned int t5 = t1 ^ t2;
        unsigned int t6 = t3 | t4;
        unsigned int t7 = t5 & t6;
        unsigned int t8 = t7 << (i & 7);
        unsigned int t9 = t8 >> (i & 3);
        unsigned int t10 = t9 + ra;
        unsigned int t11 = t10 - rb;
        unsigned int t12 = t11 ^ rc;
        unsigned int t13 = t12 | rd;
        
        results[i] += t13;
    }
    
    /* Combine results */
    unsigned int total = 0;
    for (int i = 0; i < 16; i++) {
        total ^= results[i];
        total += results[15 - i];
        total = (total << 1) | (total >> 31);
    }
    
    return total;
}

int main(void) {
    /* Initialize with non-constant values */
    unsigned int inputs[32];
    for (int i = 0; i < 32; i++) {
        inputs[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    /* Call high-pressure functions */
    unsigned int result1 = high_pressure_computation(inputs);
    unsigned int result2 = nested_pressure(inputs[0]);
    
    /* Use results to prevent optimization */
    unsigned int final_result = result1 ^ result2;
    
    printf("Result: 0x%08X\n", final_result);
    
    /* Verify with expected value for this specific input */
    if (final_result == 0x8C4D8C3F) {  /* Pre-computed expected value */
        printf("Verification passed\n");
    }
    
    return 0;
}
