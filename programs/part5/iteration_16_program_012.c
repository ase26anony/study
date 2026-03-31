#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to keep RTL complex */
static __attribute__((noinline)) 
unsigned long high_pressure_computation(int *data, int n) {
    /* Many distinct variables to create register pressure */
    register int a0, a1, a2, a3, a4, a5, a6, a7;
    register int b0, b1, b2, b3, b4, b5, b6, b7;
    register int c0, c1, c2, c3, c4, c5, c6, c7;
    register int d0, d1, d2, d3, d4, d5, d6, d7;
    register int e0, e1, e2, e3, e4, e5, e6, e7;
    
    /* Initialize from input data - creates many live ranges */
    a0 = data[0] + 1;   /* Remat candidate: cheap to recompute */
    a1 = data[1] * 2;   /* Another candidate */
    a2 = data[2] & 0xFF;
    a3 = data[3] | 0x80;
    a4 = data[4] << 1;
    a5 = data[5] >> 2;
    a6 = data[6] ^ 0x55;
    a7 = data[7] + 0x10;
    
    b0 = data[8] - 5;
    b1 = data[9] * 3;
    b2 = data[10] & 0xF0;
    b3 = data[11] | 0x0F;
    b4 = data[12] << 2;
    b5 = data[13] >> 1;
    b6 = data[14] ^ 0xAA;
    b7 = data[15] - 0x20;
    
    /* More variables to increase pressure */
    c0 = a0 + b0;
    c1 = a1 - b1;
    c2 = a2 & b2;
    c3 = a3 | b3;
    c4 = a4 ^ b4;
    c5 = a5 + b5;
    c6 = a6 - b6;
    c7 = a7 & b7;
    
    d0 = c0 * 2;
    d1 = c1 / 3;
    d2 = c2 << 1;
    d3 = c3 >> 2;
    d4 = c4 + 0x10;
    d5 = c5 - 0x20;
    d6 = c6 & 0x3F;
    d7 = c7 | 0xC0;
    
    /* Complex control flow to create merging points */
    unsigned long result = 0;
    for (int i = 0; i < n; i++) {
        /* Use many variables inside loop - keeps them live */
        if (i % 2 == 0) {
            e0 = d0 + i;    /* Uses variable defined outside loop */
            e1 = d1 - i;
            e2 = d2 & i;
            e3 = d3 | i;
            result += e0 + e1 + e2 + e3;
        } else {
            e4 = d4 ^ i;    /* Uses different set of variables */
            e5 = d5 + i;
            e6 = d6 - i;
            e7 = d7 & i;
            result += e4 + e5 + e6 + e7;
        }
        
        /* More computations to increase pressure in loop */
        for (int j = 0; j < 3; j++) {
            /* Use even more variables in nested loop */
            int t0 = a0 + j;  /* a0 defined outside both loops */
            int t1 = b0 - j;  /* b0 defined outside both loops */
            int t2 = c0 & j;
            int t3 = d0 | j;
            result += t0 + t1 + t2 + t3;
            
            /* Conditional inside nested loop */
            if (j % 2) {
                int t4 = a1 * j;
                int t5 = b1 / (j + 1);
                result += t4 + t5;
            }
        }
    }
    
    /* Final computation using all variables */
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    result += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
    result += d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
    
    return result;
}

/* Another function to create more pressure */
static __attribute__((noinline))
unsigned long another_high_pressure_function(int *data) {
    /* Different computations to avoid CSE */
    register int x0 = data[0] * 7 + 11;
    register int x1 = data[1] / 3 - 5;
    register int x2 = data[2] << 3;
    register int x3 = data[3] >> 2;
    register int x4 = data[4] & 0x7F;
    register int x5 = data[5] | 0x80;
    register int x6 = data[6] ^ 0xFF;
    register int x7 = data[7] + 100;
    
    /* Chain computations */
    register int y0 = x0 + x1;
    register int y1 = x2 - x3;
    register int y2 = x4 & x5;
    register int y3 = x6 | x7;
    register int y4 = x0 ^ x7;
    register int y5 = x1 + x6;
    register int y6 = x2 - x5;
    register int y7 = x3 & x4;
    
    /* Loop with complex liveness */
    unsigned long sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use variables defined outside loop */
        if (i < 50) {
            sum += x0 + y0 + (i * 2);
        } else {
            sum += x1 + y1 + (i / 2);
        }
        
        /* More computations */
        for (int j = 0; j < 2; j++) {
            int z0 = x2 + j + y2;
            int z1 = x3 - j + y3;
            sum += z0 + z1;
        }
    }
    
    return sum;
}

int main() {
    /* Initialize test data */
    int data[32];
    for (int i = 0; i < 32; i++) {
        data[i] = i * 3 + 7;  /* Non-trivial pattern */
    }
    
    /* Call high-pressure functions */
    unsigned long result1 = high_pressure_computation(data, 10);
    unsigned long result2 = another_high_pressure_function(data + 8);
    
    /* Combine results to ensure nothing is optimized away */
    unsigned long final_result = result1 + result2;
    
    printf("Result: %lu\n", final_result);
    
    /* Verify with expected value for testing */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
