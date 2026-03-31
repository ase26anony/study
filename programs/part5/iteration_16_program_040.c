#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to preserve complex RTL patterns */
static __attribute__((noinline)) 
unsigned long high_pressure_computation(int *data, int n) {
    /* Many distinct variables to create register pressure */
    register int a0, a1, a2, a3, a4, a5, a6, a7;
    register int b0, b1, b2, b3, b4, b5, b6, b7;
    register int c0, c1, c2, c3, c4, c5, c6, c7;
    register int d0, d1, d2, d3, d4, d5, d6, d7;
    unsigned long result = 0;
    
    /* Initialize from input data - creates many live ranges */
    a0 = data[0] + 1;   /* Remat candidate: cheap to recompute */
    a1 = data[1] * 2;
    a2 = data[2] & 0xFF;
    a3 = data[3] << 3;
    a4 = data[4] | 0x1000;
    a5 = data[5] ^ 0xAAAA;
    a6 = data[6] - 42;
    a7 = data[7] + 999;
    
    b0 = data[8] * 3;
    b1 = data[9] / 2;
    b2 = data[10] & 0xF0F0;
    b3 = data[11] >> 2;
    b4 = data[12] | 0x0F0F;
    b5 = data[13] ^ 0x5555;
    b6 = data[14] + 1234;
    b7 = data[15] - 567;
    
    /* Create rematerialization candidates with long live ranges */
    /* These will be used much later, keeping them live across many ops */
    c0 = a0 + b0;      /* Pure computation - good remat candidate */
    c1 = a1 & b1;
    c2 = a2 ^ b2;
    c3 = a3 | b3;
    c4 = a4 - b4;
    c5 = a5 + b5;
    c6 = a6 * b6;
    c7 = a7 / (b7 != 0 ? b7 : 1);
    
    /* Complex control flow to complicate liveness analysis */
    for (int i = 0; i < n; i++) {
        /* Many independent computations inside loop */
        d0 = data[i] + i;
        d1 = data[(i + 1) % n] * (i + 2);
        d2 = data[(i + 2) % n] & (i * 3);
        d3 = data[(i + 3) % n] << (i % 4);
        d4 = data[(i + 4) % n] | (i * 5);
        d5 = data[(i + 5) % n] ^ (i * 7);
        d6 = data[(i + 6) % n] - (i * 11);
        d7 = data[(i + 7) % n] + (i * 13);
        
        /* Use the long-lived variables from outside loop */
        /* This forces them to stay live across loop iterations */
        if (i % 2 == 0) {
            d0 += c0;  /* Use remat candidate */
            d1 += c1;
            d2 += c2;
        } else {
            d3 += c3;
            d4 += c4;
            d5 += c5;
        }
        
        /* More computations to increase pressure */
        result += d0 * d1 + d2 * d3 - d4 * d5 + d6 * d7;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            /* Use different combinations of variables */
            int temp = (d0 << j) + (d1 >> j) * (d2 & j);
            if (temp > 0) {
                result += temp + c6;  /* Use another remat candidate */
            } else {
                result -= temp + c7;
            }
        }
    }
    
    /* Final use of all remat candidates - ensures they stay live */
    result += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
    
    return result;
}

/* Another function to create more opportunities */
static __attribute__((noinline))
unsigned long create_pressure(int *data, int size) {
    unsigned long total = 0;
    
    /* Multiple independent computation chains */
    for (int outer = 0; outer < 4; outer++) {
        /* Variables defined outside inner loop, used inside */
        int base1 = data[outer] * 17 + 31;
        int base2 = data[outer + 1] & 0x7F;
        int base3 = data[outer + 2] | 0x8000;
        int base4 = data[outer + 3] ^ 0x3333;
        
        /* These are good remat candidates - cheap pure computations */
        int derived1 = base1 + (outer << 2);
        int derived2 = base2 - (outer * 3);
        int derived3 = base3 & (0xFF << outer);
        int derived4 = base4 | (1 << (outer % 16));
        
        for (int inner = 0; inner < size; inner++) {
            /* Many live variables across loop iterations */
            int val1 = data[inner] + derived1;
            int val2 = data[(inner + 1) % size] * derived2;
            int val3 = data[(inner + 2) % size] & derived3;
            int val4 = data[(inner + 3) % size] | derived4;
            
            /* Complex conditional using many variables */
            if ((val1 + val2) > (val3 - val4)) {
                total += val1 * val3 + derived1 * derived3;
            } else {
                total += val2 * val4 + derived2 * derived4;
            }
            
            /* More arithmetic to increase register usage */
            total += (val1 << 2) + (val2 >> 1) - (val3 & 0xF) + (val4 | 0xA);
        }
    }
    
    return total;
}

int main() {
    /* Create input data */
    int data[64];
    for (int i = 0; i < 64; i++) {
        data[i] = (i * 37 + 123) & 0xFFF;
    }
    
    /* Call high-pressure functions */
    unsigned long result1 = high_pressure_computation(data, 32);
    unsigned long result2 = create_pressure(data, 16);
    
    /* Ensure results are used */
    unsigned long final_result = result1 ^ result2;
    
    printf("Result: %lu\n", final_result);
    
    /* Verify with a simple computation */
    unsigned long check = 0;
    for (int i = 0; i < 64; i++) {
        check += data[i];
    }
    printf("Check sum: %lu\n", check);
    
    return 0;
}
