#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to keep RTL complex */
static __attribute__((noinline)) 
unsigned long high_pressure_computation(int *data, int n) {
    /* Declare many variables to create register pressure */
    register int a, b, c, d, e, f, g, h, i, j, k, l, m, o, p, q, r, s, t;
    register unsigned long result = 0;
    
    /* Initialize from input data with different patterns */
    a = data[0] ^ 0x1234;
    b = data[1] + 0x5678;
    c = data[2] * 3;
    d = data[3] | 0xABCD;
    e = data[4] - 0x1111;
    f = data[5] & 0xFFFF;
    g = data[6] << 2;
    h = data[7] >> 1;
    i = data[8] ^ data[0];
    j = data[9] + data[1];
    k = data[10] * data[2];
    l = data[11] | data[3];
    m = data[12] - data[4];
    o = data[13] & data[5];
    p = data[14] << data[6];
    q = data[15] >> data[7];
    r = data[16] ^ 0xDEAD;
    s = data[17] + 0xBEEF;
    t = data[18] * 7;
    
    /* Create rematerialization candidates - pure functions kept live */
    int cand1 = a + 0x1000;      /* Simple addition - cheap to recompute */
    int cand2 = b & 0xFF00;      /* Bitwise AND - cheap */
    int cand3 = c << 3;          /* Shift - cheap */
    int cand4 = d | 0x00FF;      /* Bitwise OR - cheap */
    int cand5 = e - 0x2000;      /* Subtraction - cheap */
    
    /* Complex loop with many live variables */
    for (int iter = 0; iter < n; iter++) {
        /* Use all candidates inside loop - they must stay live */
        int tmp1 = cand1 + (iter & 0xF);
        int tmp2 = cand2 | (iter << 8);
        int tmp3 = cand3 ^ (iter * 2);
        int tmp4 = cand4 & (0xFF00 + iter);
        int tmp5 = cand5 - (iter >> 1);
        
        /* More computations using original variables */
        int mix1 = a * tmp1;
        int mix2 = b + tmp2;
        int mix3 = c ^ tmp3;
        int mix4 = d | tmp4;
        int mix5 = e & tmp5;
        
        /* Conditional to create complex control flow */
        if (iter % 3 == 0) {
            /* Use one set of variables */
            result += mix1 + mix2 + f + g;
        } else if (iter % 3 == 1) {
            /* Use different set */
            result += mix3 + mix4 + h + i;
        } else {
            /* Use another set */
            result += mix5 + j + k + l;
        }
        
        /* More operations to extend live ranges */
        int aux1 = m * iter;
        int aux2 = o ^ iter;
        int aux3 = p << (iter & 3);
        int aux4 = q >> (iter & 1);
        int aux5 = r + iter;
        int aux6 = s - iter;
        int aux7 = t * (iter + 1);
        
        /* Use these in nested conditional */
        if (iter % 5 == 0) {
            result += aux1 + aux2;
        } else if (iter % 5 == 1) {
            result += aux3 + aux4;
        } else if (iter % 5 == 2) {
            result += aux5 + aux6;
        } else {
            result += aux7;
        }
        
        /* Force all original variables to stay live across loop iterations */
        a ^= 1;
        b += 2;
        c *= 3;
        d |= 4;
        e -= 5;
        f &= 0xFFF0;
        g <<= 1;
        h >>= 1;
        i ^= 0x1111;
        j += 0x2222;
        k *= 5;
        l |= 0x3333;
        m -= 0x4444;
        o &= 0x5555;
        p <<= 2;
        q >>= 2;
        r ^= 0x6666;
        s += 0x7777;
        t *= 9;
    }
    
    /* Final use of all candidates to ensure they're live until the end */
    result += cand1 + cand2 + cand3 + cand4 + cand5;
    
    /* Use all original variables in final computation */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + o + p + q + r + s + t;
    
    return result;
}

/* Another function to create more pressure */
static __attribute__((noinline))
unsigned long secondary_pressure(int *data, int n) {
    int v1 = data[0] * 11;
    int v2 = data[1] + 22;
    int v3 = data[2] ^ 33;
    int v4 = data[3] | 44;
    int v5 = data[4] & 55;
    int v6 = data[5] << 2;
    int v7 = data[6] >> 2;
    int v8 = data[7] * 3;
    int v9 = data[8] + 9;
    int v10 = data[9] ^ 10;
    
    /* Candidates for remat */
    int cand_a = v1 + 100;
    int cand_b = v2 & 0xF0;
    int cand_c = v3 << 1;
    
    unsigned long sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use candidates */
        sum += cand_a + (i & 0xF);
        sum += cand_b | (i << 4);
        sum += cand_c ^ i;
        
        /* Complex expression using all variables */
        int tmp = v1 + v2 - v3 * v4 / (v5 + 1) + (v6 | v7) ^ (v8 & v9) + v10;
        sum += tmp;
        
        /* Modify variables to prevent elimination */
        v1 ^= i;
        v2 += i;
        v3 *= (i & 0x7) + 1;
        v4 |= i;
        v5 &= 0xFF00 + i;
        v6 <<= 1;
        v7 >>= 1;
        v8 += v9;
        v9 ^= v10;
        v10 -= i;
    }
    
    /* Final use */
    return sum + cand_a + cand_b + cand_c;
}

int main() {
    /* Create input data */
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Call high-pressure functions */
    unsigned long result1 = high_pressure_computation(data, 50);
    unsigned long result2 = secondary_pressure(data, 30);
    
    /* Combine results to ensure computation isn't optimized away */
    unsigned long final_result = result1 ^ result2;
    
    printf("Result: %lu\n", final_result);
    
    /* Also use in a way that prevents dead code elimination */
    if (final_result > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
