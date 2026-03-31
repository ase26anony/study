#include <stdio.h>
#include <stdlib.h>

/* Function with high register pressure - designed to trigger early remat */
static __attribute__((noinline)) 
unsigned long long high_pressure_computation(int *data, int n) {
    /* Many distinct local variables to create high register pressure */
    register int v0  = data[0]  ^ 0x5A5A5A5A;
    register int v1  = data[1]  | 0x33333333;
    register int v2  = data[2]  & 0x0F0F0F0F;
    register int v3  = data[3]  + 0x11111111;
    register int v4  = data[4]  - 0x22222222;
    register int v5  = data[5]  * 3;
    register int v6  = data[6]  / 7;
    register int v7  = data[7]  << 2;
    register int v8  = data[8]  >> 1;
    register int v9  = data[9]  % 13;
    register int v10 = data[10] ^ 0xAAAAAAAA;
    register int v11 = data[11] | 0xCCCCCCCC;
    register int v12 = data[12] & 0xF0F0F0F0;
    register int v13 = data[13] + 0x12345678;
    register int v14 = data[14] - 0x87654321;
    register int v15 = data[15] * 5;
    
    /* Rematerialization candidates - cheap to recompute but kept alive */
    int r0 = v0 + 0x1000;      /* Candidate 1: v0 + constant */
    int r1 = v1 & 0x00FF00FF;  /* Candidate 2: v1 & mask */
    int r2 = v2 << 3;          /* Candidate 3: v2 << shift */
    int r3 = v3 ^ 0x99999999;  /* Candidate 4: v3 ^ constant */
    int r4 = v4 | 0x0000FFFF;  /* Candidate 5: v4 | mask */
    
    /* Complex loop with conditional branches to create merging points */
    unsigned long long result = 0;
    for (int i = 0; i < n; i++) {
        /* Use remat candidates inside loop - their defs are outside */
        int temp = 0;
        if (i & 1) {
            /* Use some candidates in this branch */
            temp += r0 * i;
            temp += r1 / (i + 1);
            temp ^= r2;
            /* Keep many variables live across this computation */
            temp += v5 * v6;
            temp -= v7 | v8;
            temp ^= v9 & v10;
        } else {
            /* Use different candidates in this branch */
            temp += r3 - i;
            temp += r4 ^ (i * 2);
            temp |= r0;  /* Reuse r0 here */
            /* Keep different variables live */
            temp += v11 * v12;
            temp -= v13 | v14;
            temp ^= v15 & v0;
        }
        
        /* Nested loop to further extend liveness */
        for (int j = 0; j < 3; j++) {
            /* Use all remat candidates and many variables here */
            int inner = temp;
            inner += (r0 + r1) * j;
            inner -= (r2 ^ r3) / (j + 1);
            inner |= r4 & (v0 + v1);
            inner ^= (v2 * v3) + (v4 | v5);
            inner += (v6 - v7) ^ (v8 & v9);
            inner -= (v10 * v11) | (v12 ^ v13);
            inner ^= (v14 + v15) & (r0 - r1);
            
            /* Force all variables to be used to prevent dead code elimination */
            result += inner + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                     v9 + v10 + v11 + v12 + v13 + v14 + v15;
        }
        
        /* More independent computations to increase pressure */
        int a = v0 * v1 + v2;
        int b = v3 / v4 - v5;
        int c = v6 << v7 % 8;
        int d = v8 ^ v9 | v10;
        int e = v11 & v12 + v13;
        int f = v14 - v15 * v0;
        
        /* Use all these new values */
        result += a * b + c - d ^ e | f;
    }
    
    /* Final combination using all remat candidates */
    result += (unsigned long long)r0 * r1;
    result ^= (unsigned long long)r2 << 16;
    result -= (unsigned long long)r3 / 256;
    result |= (unsigned long long)r4 & 0xFFFFFFFF;
    
    return result;
}

/* Another function to create different control flow patterns */
static __attribute__((noinline))
unsigned long long complex_control_flow(int *data, int n) {
    /* Initialize many variables */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = data[i % 16] + i * 7;
    }
    
    /* Create rematerialization candidates */
    int cand1 = vars[0] + 0xABCD;
    int cand2 = vars[1] & 0x1234;
    int cand3 = vars[2] << 4;
    int cand4 = vars[3] ^ 0x5678;
    int cand5 = vars[4] | 0x9ABC;
    
    unsigned long long total = 0;
    
    /* Outer loop with switch statement */
    for (int i = 0; i < n; i++) {
        switch (i % 4) {
            case 0:
                total += cand1 * vars[5] + cand2 / (vars[6] + 1);
                total ^= cand3 | vars[7];
                break;
            case 1:
                total -= cand4 - vars[8] * cand5;
                total |= cand1 & vars[9];
                break;
            case 2:
                total += cand2 ^ vars[10] + cand3 * vars[11];
                total &= cand4 | vars[12];
                break;
            case 3:
                total -= cand5 / (vars[13] + 1) - cand1;
                total ^= cand2 & vars[14];
                break;
        }
        
        /* Inner loop with early exit */
        for (int j = 0; j < 5; j++) {
            if (j > i % 3) break;
            
            /* Use all candidates and many variables */
            int mix = 0;
            mix += cand1 * j;
            mix -= cand2 / (j + 1);
            mix ^= cand3 | j;
            mix += cand4 - j * cand5;
            mix |= cand1 & (vars[15] + j);
            
            /* Use many original variables */
            for (int k = 0; k < 8; k++) {
                mix += vars[k] * (j + k);
            }
            
            total += mix;
        }
    }
    
    return total;
}

int main(void) {
    /* Initialize test data */
    int data[16];
    for (int i = 0; i < 16; i++) {
        data[i] = rand() % 1000 + 1;
    }
    
    /* Call high-pressure functions */
    unsigned long long result1 = high_pressure_computation(data, 50);
    unsigned long long result2 = complex_control_flow(data, 40);
    
    /* Combine results deterministically */
    unsigned long long final_result = result1 ^ result2;
    
    printf("Result: %llu\n", final_result);
    
    /* Verify with a simple computation */
    unsigned long long check = 0;
    for (int i = 0; i < 16; i++) {
        check += data[i] * (i + 1);
    }
    printf("Check: %llu\n", check);
    
    return 0;
}
