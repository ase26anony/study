/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * by creating high register pressure with rematerializable values
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 64

/* Force no inlining to keep RTL complex */
static __attribute__((noinline, noipa))
uint64_t high_pressure_computation(const int* data, int n) {
    /* Many distinct variables to create register pressure */
    register int v0  = data[0]  ^ 0x55AA55AA;
    register int v1  = data[1]  | 0x12345678;
    register int v2  = data[2]  & 0xF0F0F0F0;
    register int v3  = data[3]  + 0x11111111;
    register int v4  = data[4]  - 0x22222222;
    register int v5  = data[5]  * 3;
    register int v6  = data[6]  << 2;
    register int v7  = data[7]  >> 1;
    register int v8  = data[8]  ^ 0x33333333;
    register int v9  = data[9]  | 0x44444444;
    register int v10 = data[10] & 0x55555555;
    register int v11 = data[11] + 0x66666666;
    register int v12 = data[12] - 0x77777777;
    register int v13 = data[13] * 5;
    register int v14 = data[14] << 3;
    register int v15 = data[15] >> 2;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loop */
    int r0 = v0 + 0x1000;      /* Candidate 1: v0 + constant */
    int r1 = v1 & 0x00FF00FF;  /* Candidate 2: v1 & mask */
    int r2 = v2 << 4;          /* Candidate 3: v2 << shift */
    int r3 = v3 ^ 0xAAAAAAAA;  /* Candidate 4: v3 ^ constant */
    int r4 = v4 | 0xCCCCCCCC;  /* Candidate 5: v4 | mask */
    int r5 = v5 * 7;           /* Candidate 6: v5 * constant */
    int r6 = v6 + v7;          /* Candidate 7: v6 + v7 */
    int r7 = v8 & v9;          /* Candidate 8: v8 & v9 */
    
    uint64_t sum = 0;
    
    /* Complex loop with conditional branches to create merging points */
    for (int i = 0; i < n; i++) {
        int idx = data[i] & 0xF;  /* Use different index each iteration */
        
        /* Keep rematerialization candidates live across loop iterations */
        /* by using them in conditional computations */
        if (idx & 1) {
            /* Use some candidates in this branch */
            sum += r0 + r1 + (v10 * idx);
            sum += r2 ^ r3;
        } else {
            /* Use different candidates in this branch */
            sum += r4 | r5;
            sum += r6 - r7 + (v11 >> idx);
        }
        
        /* More computations to increase pressure */
        int t0 = v12 * i;
        int t1 = v13 + i;
        int t2 = v14 ^ i;
        int t3 = v15 & i;
        
        /* Nested conditional with more variable usage */
        if (i % 3 == 0) {
            sum += t0 + t1 + r0;  /* r0 used again */
            sum += v0 * v1;       /* Original vars still live */
        } else if (i % 3 == 1) {
            sum += t2 | t3;
            sum += r1 + r2;       /* r1, r2 used again */
        } else {
            sum += t0 ^ t1;
            sum += r3 * r4;       /* r3, r4 used again */
        }
        
        /* Additional computations that use many variables */
        sum += (v0 << (i & 3)) + (v1 >> (i & 3));
        sum += (v2 & (0xFF << (i & 7))) | (v3 & (0xFF00 >> (i & 7)));
        sum += (v4 + i) * (v5 - i);
        sum += (v6 ^ v7) + (v8 | v9);
        
        /* Force all candidates to stay live by using them occasionally */
        if (i % 5 == 0) {
            sum += r5 * r6 * r7;
        }
    }
    
    /* Final computation using all rematerialization candidates */
    /* This ensures they must stay live until the end */
    uint64_t final = sum;
    final += r0 * 2;
    final += r1 / 3;
    final += r2 << 1;
    final ^= r3;
    final |= r4;
    final -= r5;
    final += r6 * r7;
    
    /* Use all original variables in final result */
    final += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    final += v8 * v9 * v10 * v11;
    final ^= v12 | v13 | v14 | v15;
    
    return final;
}

/* Another function to create more complex call graph */
static __attribute__((noinline, noipa))
uint64_t nested_pressure(const int* data, int depth) {
    if (depth <= 0) {
        return data[0];
    }
    
    /* Create more local variables */
    int a = data[depth] + depth;
    int b = data[depth + 1] * depth;
    int c = data[depth + 2] ^ depth;
    int d = data[depth + 3] | depth;
    
    /* Rematerialization candidates */
    int ra = a << 2;
    int rb = b & 0x0F0F0F0F;
    int rc = c + 0x10101010;
    int rd = d ^ 0x33333333;
    
    /* Recursive call keeps variables live across call */
    uint64_t sub = nested_pressure(data, depth - 1);
    
    /* Use candidates after recursive call */
    uint64_t result = sub;
    result += ra * rb;
    result -= rc | rd;
    result += (a * b) ^ (c * d);
    
    return result;
}

/* Main driver that creates the pressure scenario */
int main(void) {
    int test_data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        test_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call high-pressure function multiple times */
    uint64_t total = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Modify data slightly each iteration */
        test_data[iter % ARRAY_SIZE] ^= iter;
        
        /* Main high-pressure computation */
        total += high_pressure_computation(test_data, ARRAY_SIZE / 2);
        
        /* Nested computation adds more pressure */
        total ^= nested_pressure(test_data, 8);
    }
    
    printf("Result: %llu\n", (unsigned long long)total);
    return 0;
}
