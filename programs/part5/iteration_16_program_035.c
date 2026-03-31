#include <stdio.h>
#include <stdlib.h>

/* Function with high register pressure - designed to trigger early remat */
static __attribute__((noinline)) 
unsigned long long high_pressure_computation(int *input, int n) {
    /* Declare many variables to create register pressure */
    register int a0, a1, a2, a3, a4, a5, a6, a7;
    register int b0, b1, b2, b3, b4, b5, b6, b7;
    register int c0, c1, c2, c3, c4, c5, c6, c7;
    register int d0, d1, d2, d3, d4, d5, d6, d7;
    register unsigned long long result = 0;
    
    /* Initialize from input array to prevent constant propagation */
    a0 = input[0] ^ 0x12345678;
    a1 = input[1] ^ 0x23456789;
    a2 = input[2] ^ 0x3456789A;
    a3 = input[3] ^ 0x456789AB;
    a4 = input[4] ^ 0x56789ABC;
    a5 = input[5] ^ 0x6789ABCD;
    a6 = input[6] ^ 0x789ABCDE;
    a7 = input[7] ^ 0x89ABCDEF;
    
    b0 = input[8]  | 0x11111111;
    b1 = input[9]  | 0x22222222;
    b2 = input[10] | 0x33333333;
    b3 = input[11] | 0x44444444;
    b4 = input[12] | 0x55555555;
    b5 = input[13] | 0x66666666;
    b6 = input[14] | 0x77777777;
    b7 = input[15] | 0x88888888;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges and be cheap to recompute */
    c0 = (a0 + 0x1000) & 0xFFFFF;  /* Candidate 1: a0 + constant */
    c1 = (a1 * 3) >> 1;            /* Candidate 2: a1 * 3 / 2 */
    c2 = a2 & 0xFF00FF;            /* Candidate 3: mask operation */
    c3 = a3 << 4;                  /* Candidate 4: shift operation */
    c4 = a4 ^ 0xAAAAAAAA;          /* Candidate 5: xor with constant */
    c5 = a5 + a0;                  /* Candidate 6: sum of two vars */
    c6 = a6 * 7;                   /* Candidate 7: multiplication */
    c7 = a7 - 0x100;               /* Candidate 8: subtraction */
    
    /* Force c0-c7 to stay live across many operations */
    /* by using them much later in the computation */
    
    /* Complex control flow to create challenging liveness patterns */
    for (int i = 0; i < n; i++) {
        /* Nested loop with different computations */
        for (int j = 0; j < 3; j++) {
            /* Use some variables inside loops, creating pressure */
            d0 = input[i % 16] + j;
            d1 = input[(i + 1) % 16] * j;
            d2 = input[(i + 2) % 16] & j;
            d3 = input[(i + 3) % 16] | j;
            d4 = input[(i + 4) % 16] ^ j;
            d5 = input[(i + 5) % 16] - j;
            d6 = input[(i + 6) % 16] << (j & 3);
            d7 = input[(i + 7) % 16] >> (j & 3);
            
            /* Conditional branch using different variable sets */
            if ((i + j) & 1) {
                /* Use b variables in one branch */
                result += b0 + b1 + b2 + b3 + d0 + d1;
            } else {
                /* Use a variables in other branch */
                result += a0 + a1 + a2 + a3 + d2 + d3;
            }
            
            /* More computations to increase register pressure */
            int t0 = d4 * d5;
            int t1 = d6 / (d7 ? d7 : 1);
            int t2 = t0 ^ t1;
            int t3 = t2 << 2;
            int t4 = t3 & 0xFF;
            int t5 = t4 | 0x80;
            int t6 = t5 - 64;
            int t7 = t6 * 3;
            
            result += t7;
        }
        
        /* Use the rematerialization candidates inside the loop */
        /* This forces them to stay live across loop iterations */
        if (i & 1) {
            result += c0 + c2 + c4 + c6;
        } else {
            result += c1 + c3 + c5 + c7;
        }
    }
    
    /* Final use of all rematerialization candidates */
    /* Ensures they have very long live ranges */
    result += (c0 * c1) + (c2 * c3) + (c4 * c5) + (c6 * c7);
    
    /* Use all a and b variables in final computation */
    /* to ensure they can't be optimized away */
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    
    return result;
}

/* Another function to create more complex call graph */
static __attribute__((noinline))
unsigned long long process_data(int *data, int size) {
    unsigned long long total = 0;
    
    /* Process in chunks to create more register pressure */
    for (int chunk = 0; chunk < size; chunk += 16) {
        int chunk_size = (size - chunk) > 16 ? 16 : (size - chunk);
        int chunk_data[16];
        
        /* Copy and modify data */
        for (int i = 0; i < 16; i++) {
            if (i < chunk_size) {
                chunk_data[i] = data[chunk + i] + i;
            } else {
                chunk_data[i] = i * 0x12345;
            }
        }
        
        /* Call high-pressure function */
        total += high_pressure_computation(chunk_data, 5);
    }
    
    return total;
}

int main() {
    /* Create input data with some variation */
    int input_data[64];
    for (int i = 0; i < 64; i++) {
        input_data[i] = rand() % 1000 + i;
    }
    
    /* Perform computation that should trigger early remat */
    unsigned long long result = process_data(input_data, 64);
    
    printf("Result: %llu\n", result);
    
    /* Verify with a simple check */
    if (result != 0) {
        printf("Computation completed successfully.\n");
    }
    
    return 0;
}
