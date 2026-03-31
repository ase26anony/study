/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass by creating
 * high register pressure with rematerializable values.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to keep RTL complex */
static __attribute__((noinline,noipa))
uint64_t high_pressure_computation(const uint32_t* data, int n) {
    /* Many distinct local variables to create register pressure */
    uint64_t a, b, c, d, e, f, g, h, i, j, k, l, m, o, p, q, r, s, t;
    uint64_t u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah;
    
    /* Initialize from input data with different patterns */
    a = data[0] + 1001;
    b = data[1] * 3;
    c = data[2] ^ 0x55AA55AA;
    d = data[3] - 273;
    e = data[4] | 0xFF00FF00;
    f = data[5] << 3;
    g = data[6] >> 2;
    h = data[7] + data[0];
    i = data[8] * 7;
    j = data[9] & 0x0F0F0F0F;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loops */
    uint64_t remat1 = (a * 37) + 12345;      /* Candidate 1: a*37+12345 */
    uint64_t remat2 = (b ^ c) | 0x88888888;  /* Candidate 2: b^c|0x88888888 */
    uint64_t remat3 = (d << 4) ^ (e >> 2);   /* Candidate 3: d<<4^e>>2 */
    uint64_t remat4 = (f & g) + 0xDEADBEEF;  /* Candidate 4: f&g+0xDEADBEEF */
    uint64_t remat5 = (h * i) - j;           /* Candidate 5: h*i-j */
    
    /* First loop - uses remat values, creates many live variables */
    uint64_t sum1 = 0;
    for (int idx = 0; idx < n; idx++) {
        /* Complex computation using many variables kept live */
        k = data[idx % 10] + idx;
        l = remat1 * k + 17;      /* Uses remat1 - may need rematerialization */
        m = remat2 ^ (k << 2);
        o = remat3 + (k * 3);
        p = remat4 | (k & 0xFF);
        q = remat5 - (k / 2);
        
        /* More intermediate values */
        r = (a + b) * (c - d);
        s = (e & f) | (g ^ h);
        t = (i * j) + (k * l);
        u = (m & 0xF0F0F0F0) + o;
        v = (p << 1) ^ (q >> 1);
        w = (r * s) - t;
        x = (u | v) & w;
        y = (x * 19) + 23;
        z = (y ^ 0x12345678) + remat1;  /* Another use of remat1 */
        
        /* Conditional inside loop creates merging points */
        if (idx % 3 == 0) {
            aa = z * remat2;      /* Uses remat2 */
            ab = aa + remat3;     /* Uses remat3 */
            sum1 += ab;
        } else if (idx % 3 == 1) {
            ac = z / remat4;      /* Uses remat4 */
            ad = ac ^ remat5;     /* Uses remat5 */
            sum1 += ad;
        } else {
            ae = z & remat1;      /* Uses remat1 again */
            af = ae | remat2;     /* Uses remat2 again */
            sum1 += af;
        }
        
        /* Keep more variables live across iteration */
        ag = (a * b) + (c * d) + (e * f) + (g * h) + (i * j);
        ah = ag ^ (k * l * m * o * p);
    }
    
    /* Second nested loop with different computations */
    uint64_t sum2 = 0;
    for (int outer = 0; outer < 5; outer++) {
        for (int inner = 0; inner < n/2; inner++) {
            /* Use remat values again - may need rematerialization here */
            uint64_t tmp1 = remat1 * outer + remat2 * inner;
            uint64_t tmp2 = remat3 ^ outer | remat4 & inner;
            uint64_t tmp3 = remat5 + (outer << 4) - (inner >> 2);
            
            /* More computations with many live values */
            uint64_t val1 = (a * outer) + (b * inner);
            uint64_t val2 = (c ^ outer) & (d | inner);
            uint64_t val3 = (e + outer) * (f - inner);
            uint64_t val4 = (g & outer) | (h ^ inner);
            uint64_t val5 = (i * outer) + (j * inner);
            
            /* Use all the remat values in final computation */
            uint64_t combined = (tmp1 * val1) + (tmp2 * val2) + 
                               (tmp3 * val3) + (remat1 * val4) + 
                               (remat2 * val5) + (remat3 * tmp1) +
                               (remat4 * tmp2) + (remat5 * tmp3);
            
            /* Conditional with different variable usage */
            if ((outer + inner) % 2 == 0) {
                sum2 += combined * remat1;  /* Uses remat1 */
            } else {
                sum2 += combined / remat2;  /* Uses remat2 */
            }
            
            /* More variables to increase pressure */
            uint64_t extra1 = (val1 * val2) + (val3 * val4) + val5;
            uint64_t extra2 = (tmp1 & tmp2) | (tmp3 ^ combined);
            uint64_t extra3 = (extra1 << 3) + (extra2 >> 1);
            
            /* Use remat values one more time */
            if (extra3 > 1000) {
                sum2 += remat3 * extra1 + remat4 * extra2 + remat5 * extra3;
            }
        }
    }
    
    /* Final computation using all remat values and many variables */
    /* This ensures they stay live until the very end */
    uint64_t result = (sum1 * remat1) + (sum2 * remat2) + 
                     (a * remat3) + (b * remat4) + (c * remat5) +
                     (d * e) + (f * g) + (h * i) + (j * k) +
                     (l * m) + (o * p) + (q * r) + (s * t) +
                     (u * v) + (w * x) + (y * z) + (aa * ab) +
                     (ac * ad) + (ae * af) + (ag * ah);
    
    return result;
}

/* Wrapper to prevent optimization across calls */
static __attribute__((noinline))
uint64_t compute_hash(const uint32_t* data, int size) {
    return high_pressure_computation(data, size);
}

int main() {
    /* Input data - enough to feed all the variables */
    uint32_t input_data[20];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < 20; i++) {
        input_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform computation */
    uint64_t result = compute_hash(input_data, 50);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", (unsigned long long)result);
    
    /* Also use in a way compiler can't predict */
    volatile uint64_t* volatile_ptr = (volatile uint64_t*)&result;
    if (*volatile_ptr > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
