#include <stdio.h>
#include <stdlib.h>

/* Function with high register pressure and rematerialization candidates */
static __attribute__((noinline)) 
unsigned long long high_pressure_computation(int *data, int n) {
    /* Many distinct variables to create register pressure */
    register int a = data[0] ^ 0x12345678;
    register int b = data[1] + 0x89ABCDEF;
    register int c = data[2] * 0x11111111;
    register int d = data[3] | 0xF0F0F0F0;
    register int e = data[4] & 0x0F0F0F0F;
    register int f = data[5] - 0x55555555;
    register int g = data[6] ^ 0xAAAAAAAA;
    register int h = data[7] + 0x33333333;
    register int i = data[8] * 0x77777777;
    register int j = data[9] | 0xCCCCCCCC;
    
    /* Rematerialization candidates - cheap to recompute */
    int r1 = a + 0x1000;      /* Candidate 1: a + constant */
    int r2 = b & 0x00FF00FF;  /* Candidate 2: b & mask */
    int r3 = c << 3;          /* Candidate 3: c << shift */
    int r4 = d ^ 0x11111111;  /* Candidate 4: d ^ constant */
    int r5 = e | 0x80808080;  /* Candidate 5: e | mask */
    
    /* More variables to increase pressure */
    register int k = data[10] ^ 0x44444444;
    register int l = data[11] + 0x88888888;
    register int m = data[12] * 0x99999999;
    register int o = data[13] | 0x22222222;
    register int p = data[14] & 0xDDDDDDDD;
    register int q = data[15] - 0x66666666;
    register int r = data[16] ^ 0xBBBBBBBB;
    register int s = data[17] + 0x77777777;
    register int t = data[18] * 0x33333333;
    register int u = data[19] | 0xEEEEEEEE;
    
    /* More rematerialization candidates */
    int r6 = f + 0x2000;      /* Candidate 6 */
    int r7 = g & 0xF0F0F0F0;  /* Candidate 7 */
    int r8 = h << 2;          /* Candidate 8 */
    int r9 = i ^ 0x55555555;  /* Candidate 9 */
    int r10 = j | 0x0A0A0A0A; /* Candidate 10 */
    
    /* Complex control flow to extend liveness */
    unsigned long long result = 0;
    
    /* Outer loop - keeps many variables live */
    for (int outer = 0; outer < 3; outer++) {
        /* Use rematerialization candidates inside loop */
        int temp1 = r1 + outer;  /* Use candidate 1 */
        int temp2 = r2 - outer;  /* Use candidate 2 */
        
        /* Nested inner loop with conditional */
        for (int inner = 0; inner < 2; inner++) {
            /* Use different sets of variables conditionally */
            if (inner % 2 == 0) {
                /* Use first set of remat candidates */
                temp1 += r3 + r4 + r5;
                temp2 += r6 + r7;
            } else {
                /* Use second set of remat candidates */
                temp1 += r8 + r9 + r10;
                temp2 += (k & 0xFF) + (l & 0xFF);
            }
            
            /* More computations to keep variables live */
            int mix1 = (m >> inner) ^ (o << inner);
            int mix2 = (p + inner) | (q - inner);
            int mix3 = (r ^ inner) & (s | inner);
            int mix4 = (t * inner) + (u % (inner + 1));
            
            /* Use all variables in complex expression */
            result += (unsigned long long)temp1 * temp2;
            result += (unsigned long long)mix1 * mix2;
            result += (unsigned long long)mix3 * mix4;
            
            /* Force liveness of original variables */
            result += a + b + c + d + e;
            result += f + g + h + i + j;
            result += k + l + m + o + p;
            result += q + r + s + t + u;
        }
        
        /* Modify some variables to prevent CSE */
        a ^= outer;
        b += outer;
        c *= (outer + 1);
        d |= outer;
        e &= ~outer;
        
        /* Force liveness of remat candidates across loop iterations */
        result += r1 + r2 + r3 + r4 + r5;
        result += r6 + r7 + r8 + r9 + r10;
    }
    
    /* Final computation using all rematerialization candidates */
    int final1 = r1 * r2 * r3 * r4 * r5;
    int final2 = r6 * r7 * r8 * r9 * r10;
    
    /* Use all original variables one more time */
    int sum_all = a + b + c + d + e + f + g + h + i + j +
                  k + l + m + o + p + q + r + s + t + u;
    
    /* Combine everything into final result */
    result += (unsigned long long)final1 * final2;
    result += sum_all;
    
    return result;
}

/* Another function to create more pressure */
static __attribute__((noinline))
unsigned long long create_more_pressure(int *data) {
    /* Different computations to avoid pattern recognition */
    int v1 = data[0] * 0x13579BDF;
    int v2 = data[1] ^ 0x2468ACE0;
    int v3 = data[2] + 0x98765432;
    int v4 = data[3] | 0xABCDEF01;
    int v5 = data[4] & 0xFEDCBA98;
    
    /* Chain computations to create dependencies */
    int chain1 = v1 << (v2 & 0x7);
    int chain2 = v3 >> (v4 & 0x7);
    int chain3 = v5 ^ (v1 & v2);
    int chain4 = (v3 | v4) + (v5 ^ 0x55555555);
    int chain5 = (v1 + v2) * (v3 - v4);
    
    /* Keep all chain results live */
    unsigned long long acc = 0;
    for (int i = 0; i < 4; i++) {
        acc += chain1 + chain2 + chain3 + chain4 + chain5;
        /* Modify to prevent elimination */
        chain1 ^= i;
        chain2 += i;
        chain3 *= (i + 1);
        chain4 |= i;
        chain5 &= ~i;
    }
    
    return acc;
}

int main(void) {
    /* Initialize data array with varied values */
    int data[32];
    for (int i = 0; i < 32; i++) {
        data[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    /* Call high-pressure functions */
    unsigned long long result1 = high_pressure_computation(data, 20);
    unsigned long long result2 = create_more_pressure(data + 10);
    
    /* Combine results deterministically */
    unsigned long long final_result = result1 ^ result2;
    
    printf("Result: %llu\n", final_result);
    
    /* Verify with a simple check */
    if (final_result != 0) {
        printf("Computation successful\n");
    }
    
    return 0;
}
