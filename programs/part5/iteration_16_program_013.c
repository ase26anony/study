#include <stdio.h>
#include <stdlib.h>

/* Function with high register pressure - designed to trigger early remat */
static __attribute__((noinline)) 
unsigned long long high_pressure_computation(int *input, int n) {
    /* Declare many local variables to create register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    register int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    register int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    register unsigned long long result = 0;
    
    /* Initialize variables with input values */
    v0 = input[0] + 1;
    v1 = input[1] * 2;
    v2 = input[2] & 0xFF;
    v3 = input[3] | 0x80;
    v4 = input[4] << 1;
    v5 = input[5] >> 2;
    v6 = input[6] ^ 0x55;
    v7 = input[7] + 0x100;
    v8 = input[8] * 3;
    v9 = input[9] & 0xF0;
    
    /* Create rematerialization candidates - cheap to recompute */
    int cand1 = v0 + 0x1234;      /* Simple addition */
    int cand2 = v1 & 0x00FF;      /* Bitmask operation */
    int cand3 = v2 << 2;          /* Simple shift */
    int cand4 = v3 | 0x0100;      /* Bitwise OR */
    int cand5 = v4 + v0;          /* Addition of two values */
    int cand6 = v5 ^ 0xAA;        /* XOR with constant */
    int cand7 = v6 * 2;           /* Multiplication by 2 */
    int cand8 = v7 & ~0x0F;       /* Bitmask with complement */
    
    /* Keep candidates live across many operations */
    result += cand1;
    result += cand2;
    
    /* Complex control flow to create liveness challenges */
    for (int i = 0; i < n; i++) {
        /* More variables inside loop */
        v10 = input[i % 10] + i;
        v11 = input[(i + 1) % 10] * i;
        v12 = input[(i + 2) % 10] & i;
        v13 = input[(i + 3) % 10] | i;
        v14 = input[(i + 4) % 10] ^ i;
        v15 = input[(i + 5) % 10] << (i % 4);
        v16 = input[(i + 6) % 10] >> (i % 4);
        v17 = input[(i + 7) % 10] + (i * 2);
        v18 = input[(i + 8) % 10] * (i + 1);
        v19 = input[(i + 9) % 10] & (i | 0xF);
        
        /* Use candidates inside loop - forcing them to stay live */
        if (i % 2 == 0) {
            result += cand3 + v10;
            result += cand4 * v11;
        } else {
            result += cand5 ^ v12;
            result += cand6 | v13;
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v20 = v10 + j;
            v21 = v11 * j;
            v22 = v12 & j;
            v23 = v13 | j;
            v24 = v14 ^ j;
            
            /* More uses of candidates */
            result += cand7 + v20;
            result += cand8 & v21;
            
            /* Conditional with different variable sets */
            if (j % 2 == 0) {
                v25 = v15 + v20;
                v26 = v16 * v21;
                result += v25 + v26;
            } else {
                v27 = v17 & v22;
                v28 = v18 | v23;
                result += v27 ^ v28;
            }
            
            /* Final use of all candidates before loop ends */
            v29 = cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7 + cand8;
            result += v29 & 0xFF;
        }
        
        /* Force all variables to be used across iteration boundaries */
        v0 = v0 ^ v10;
        v1 = v1 + v11;
        v2 = v2 & v12;
        v3 = v3 | v13;
        v4 = v4 ^ v14;
        v5 = v5 + v15;
        v6 = v6 & v16;
        v7 = v7 | v17;
        v8 = v8 ^ v18;
        v9 = v9 + v19;
    }
    
    /* Final computation using all candidates and variables */
    result += cand1 * 2;
    result += cand2 / 4;
    result += cand3 << 1;
    result += cand4 >> 2;
    result += cand5 ^ 0xCC;
    result += cand6 | 0x33;
    result += cand7 & 0xAA;
    result += cand8 + 0x1000;
    
    /* Use all v variables one more time */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    
    return result;
}

/* Another function to create different control flow patterns */
static __attribute__((noinline))
unsigned long long create_remat_opportunities(int *data, int size) {
    unsigned long long sum = 0;
    
    /* Define values outside loop */
    int base1 = data[0] + 0x100;
    int base2 = data[1] * 2;
    int base3 = data[2] & 0xFF00;
    int base4 = data[3] | 0x00FF;
    
    /* These become remat candidates */
    int remat_cand1 = base1 + 0x200;
    int remat_cand2 = base2 & 0x0F0F;
    int remat_cand3 = base3 << 1;
    int remat_cand4 = base4 >> 2;
    
    for (int i = 0; i < size; i++) {
        /* Many computations inside loop */
        int t1 = data[i] + i;
        int t2 = data[(i + 1) % size] * (i + 2);
        int t3 = data[(i + 2) % size] & (i + 3);
        int t4 = data[(i + 3) % size] | (i + 4);
        int t5 = data[(i + 4) % size] ^ (i + 5);
        int t6 = data[(i + 5) % size] << ((i + 6) % 8);
        int t7 = data[(i + 6) % size] >> ((i + 7) % 8);
        int t8 = data[(i + 7) % size] + (i * 3);
        int t9 = data[(i + 8) % size] * (i * 4);
        int t10 = data[(i + 9) % size] & (i * 5);
        
        /* Use remat candidates repeatedly */
        sum += remat_cand1 * t1;
        sum += remat_cand2 | t2;
        sum += remat_cand3 & t3;
        sum += remat_cand4 ^ t4;
        
        /* Conditional block with different computations */
        if (i % 3 == 0) {
            sum += remat_cand1 + t5;
            sum += remat_cand2 * t6;
        } else if (i % 3 == 1) {
            sum += remat_cand3 | t7;
            sum += remat_cand4 & t8;
        } else {
            sum += remat_cand1 ^ t9;
            sum += remat_cand2 + t10;
        }
        
        /* More variables to increase pressure */
        int u1 = t1 + t2;
        int u2 = t3 * t4;
        int u3 = t5 & t6;
        int u4 = t7 | t8;
        int u5 = t9 ^ t10;
        
        sum += u1 + u2 + u3 + u4 + u5;
    }
    
    /* Final use ensures candidates stay live */
    return sum + remat_cand1 + remat_cand2 + remat_cand3 + remat_cand4;
}

int main(void) {
    /* Create input data */
    int data[20];
    for (int i = 0; i < 20; i++) {
        data[i] = (i * 37 + 123) & 0xFFF;  /* Semi-random values */
    }
    
    /* Call high-pressure functions */
    unsigned long long result1 = high_pressure_computation(data, 50);
    unsigned long long result2 = create_remat_opportunities(data, 20);
    
    /* Combine results to ensure all computations are used */
    unsigned long long final_result = result1 ^ result2;
    
    printf("Result: %llu\n", final_result);
    
    /* Verify with a simple check */
    if (final_result != 0) {
        printf("Computation successful (non-zero result)\n");
    }
    
    return 0;
}
