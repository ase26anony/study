/* Test case for early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;

/* Global array to create address calculations */
int global_array[100];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah;
    short sa, sb, sc, sd, se, sf, sg, sh, si, sj;
    
    /* Complex expression chain with many intermediate values */
    a = x1 + x2 + v1;           /* Use volatile */
    b = a * x3 - v2;
    c = b ^ x4;
    d = c + x5 * 3;
    e = d - x6 / 2;
    f = e | x7;
    g = f & x8;
    h = g * x9 + 42;           /* Constant that could be rematerialized */
    i = h - x10;
    j = i << 2;
    k = j >> 1;
    l = k + (x1 & x2);
    m = l * (x3 | x4);
    n = m - (x5 ^ x6);
    o = n + (x7 & x8);
    p = o * (x9 | x10);
    q = p - a + b;
    r = q * c / (d + 1);
    s = r ^ e ^ f;
    t = s + g - h;
    u = t * i / j;
    v = u + k - l;
    w = v * m / n;
    x = w + o - p;
    y = x * q / r;
    z = y + s - t;
    aa = z * u / v;
    ab = aa + w - x;
    ac = ab * y / z;
    ad = ac + aa - ab;
    ae = ad * ac / (ab + 1);
    af = ae ^ ad ^ ac;
    ag = af + ae - ad;
    ah = ag * af / ae;
    
    /* Mixed-type operations to create mode conversions */
    sa = (short)ah;
    sb = (short)ag + vs1;      /* Use volatile short */
    sc = sa * sb;
    sd = sc - vs2;
    se = sd / vs3;
    sf = (short)af + se;
    sg = sf * sa;
    sh = sg - sb;
    si = sh / sc;
    sj = si + sd;
    
    /* Complex control flow with switch to create dataflow complexity */
    int switch_val = (ah + sj) % 8;
    
    switch (switch_val) {
        case 0:
            /* Use subset of values in different modes */
            a = (int)sa + (int)sb;
            b = a * c;
            break;
        case 1:
            c = (int)sc - (int)sd;
            d = c ^ e;
            break;
        case 2:
            e = (int)se * (int)sf;
            f = e & g;
            break;
        case 3:
            g = (int)sg / (int)sh;
            h = g | i;
            break;
        case 4:
            i = (int)si + (int)sj;
            j = i - k;
            break;
        case 5:
            k = (int)sa * (int)sc;
            l = k ^ m;
            break;
        case 6:
            m = (int)se - (int)sf;
            n = m & o;
            break;
        case 7:
            o = (int)sg / (int)sh;
            p = o | q;
            break;
    }
    
    /* Address calculations that could be rematerialized */
    int *ptr1 = &global_array[a % 100];
    int *ptr2 = &global_array[b % 100];
    int *ptr3 = &global_array[c % 100];
    int *ptr4 = &global_array[d % 100];
    
    /* Use addresses in memory operations */
    *ptr1 = e;
    *ptr2 = f;
    *ptr3 = g;
    *ptr4 = h;
    
    /* More arithmetic using the pointers (address calculations) */
    int idx1 = (*ptr1 + *ptr2) % 100;
    int idx2 = (*ptr3 + *ptr4) % 100;
    int *ptr5 = &global_array[idx1];
    int *ptr6 = &global_array[idx2];
    
    *ptr5 = i + j;
    *ptr6 = k - l;
    
    /* Inline assembly to create complex dataflow patterns */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "subl %[in4], %[out2]"
        : [out1] "=r" (asm_out1), [out2] "=r" (asm_out2)
        : [in1] "r" (m), [in2] "r" (n), [in3] "r" (o), [in4] "r" (p)
        : "cc"
    );
    
    /* Use assembly outputs */
    int result1 = asm_out1 * q;
    int result2 = asm_out2 / r;
    
    /* Another volatile barrier */
    if (v1 > 0) {
        result1 += v2;
        result2 -= v3;
    }
    
    /* Final complex computation using most variables */
    int final_result = 
        (result1 + result2) * (s - t) / (u + 1) +
        (v * w) - (x / y) +
        (z ^ aa) | (ab & ac) +
        (ad - ae) * (af + ag) / (ah + 1) +
        ((int)sa * sb) - ((int)sc / sd) +
        ((int)se + sf) * ((int)sg - sh) / ((int)si + sj);
    
    /* Prevent tail recursion optimization */
    volatile int dummy = final_result;
    asm volatile ("" : : "r" (dummy));
    
    return final_result;
}

/* Another function to create more compilation context */
void __attribute__((noinline))
use_results(int *results, int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += results[i];
        /* Complex addressing mode */
        results[(i * 37) % count] += sum;
    }
    asm volatile ("" : : "r" (sum));
}

int main(void) {
    int results[10];
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Call compute_heavy multiple times with different arguments
       to increase overall register pressure in the compilation unit */
    results[0] = compute_heavy(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    results[1] = compute_heavy(11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    results[2] = compute_heavy(21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
    results[3] = compute_heavy(31, 32, 33, 34, 35, 36, 37, 38, 39, 40);
    results[4] = compute_heavy(41, 42, 43, 44, 45, 46, 47, 48, 49, 50);
    
    /* More calls with different patterns */
    for (int i = 5; i < 10; i++) {
        int base = i * 10;
        results[i] = compute_heavy(
            base + 1, base + 2, base + 3, base + 4, base + 5,
            base + 6, base + 7, base + 8, base + 9, base + 10);
    }
    
    /* Process results */
    use_results(results, 10);
    
    /* Final checksum */
    int checksum = 0;
    for (int i = 0; i < 10; i++) {
        checksum ^= results[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
