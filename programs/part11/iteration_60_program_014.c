/* Test case for GCC early rematerialization pass
 * Targets lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-schedule-insns -fno-schedule-insns2 -march=x86-64 -mtune=generic -fPIC -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20;

/* Global array to force address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
high_pressure_function(int p1, int p2, int p3, int p4, int p5,
                       short sp1, short sp2, short sp3) 
{
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    short sa, sb, sc, sd, se, sf, sg, sh, si, sj;
    
    /* Initial computations creating many live values */
    a = p1 + p2;                    /* 1 */
    b = a * p3;                     /* 2 */
    c = b - p4;                     /* 3 */
    d = c ^ p5;                     /* 4 */
    e = d + v1;                     /* 5 - volatile barrier */
    f = e * 42;                     /* 6 - constant for remat */
    g = f / (p1 | 1);               /* 7 */
    h = g << 3;                     /* 8 */
    i = h & 0xFF;                   /* 9 */
    j = i + sp1;                    /* 10 - mode mixing */
    k = j - sp2;                    /* 11 */
    l = k * 137;                    /* 12 - another constant */
    m = l ^ (p2 * 2);               /* 13 */
    n = m + v2;                     /* 14 - volatile barrier */
    o = n | 0xAAAA;                 /* 15 */
    p = o & 0x5555;                 /* 16 */
    q = p + sp3;                    /* 17 */
    r = q * 19;                     /* 18 */
    s = r - (p3 << 1);              /* 19 */
    t = s ^ (p4 >> 2);              /* 20 */
    u = t + v3;                     /* 21 - volatile barrier */
    v = u * 11;                     /* 22 */
    w = v / (p5 + 1);               /* 23 */
    x = w << 4;                     /* 24 */
    y = x & 0xFFF;                  /* 25 */
    z = y + (int)vs1;               /* 26 - short to int conversion */
    aa = z - (int)vs2;              /* 27 - another conversion */
    ab = aa * 29;                   /* 28 */
    ac = ab ^ (p1 * 3);             /* 29 */
    ad = ac + v4;                   /* 30 - volatile barrier */
    ae = ad | 0xCCCC;               /* 31 */
    af = ae & 0x3333;               /* 32 */
    ag = af + (p2 * 4);             /* 33 */
    ah = ag * 31;                   /* 34 */
    ai = ah - (p3 * 5);             /* 35 */
    aj = ai ^ (p4 * 6);             /* 36 */
    
    /* More computations using all values to keep them live */
    int sum1 = a + b + c + d + e + f + g + h + i + j;
    int sum2 = k + l + m + n + o + p + q + r + s + t;
    int sum3 = u + v + w + x + y + z + aa + ab + ac + ad;
    int sum4 = ae + af + ag + ah + ai + aj;
    
    /* Complex switch to create control flow divergence */
    int selector = (sum1 ^ sum2) & 0x7;
    
    switch (selector) {
        case 0:
            sa = (short)(a + b + c);  /* Mode conversion */
            sb = (short)(d + e + f);
            sc = sa * sb;
            sum1 += (int)sc * 2;      /* Back to int */
            break;
        case 1:
            sd = (short)(g + h + i);
            se = (short)(j + k + l);
            sf = sd ^ se;
            sum2 += (int)sf * 3;
            break;
        case 2:
            sg = (short)(m + n + o);
            sh = (short)(p + q + r);
            si = sg & sh;
            sum3 += (int)si * 5;
            break;
        case 3:
            sj = (short)(s + t + u);
            sa = (short)(v + w + x);
            sb = sj | sa;
            sum4 += (int)sb * 7;
            break;
        case 4:
            /* Mixed mode computations */
            sum1 = sum1 + (int)((short)sum2 * (short)sum3);
            break;
        case 5:
            sum2 = sum2 + (int)((short)sum4 * (short)sum1);
            break;
        case 6:
            sum3 = sum3 + (int)((short)sum2 * (short)sum4);
            break;
        case 7:
            sum4 = sum4 + (int)((short)sum3 * (short)sum1);
            break;
    }
    
    /* Loop with address calculations that may be rematerialized */
    int array_sum = 0;
    for (int idx = 0; idx < 64; idx++) {
        /* Base address calculation - potential remat candidate */
        int *base_ptr = &global_array[idx * 2];
        
        /* Multiple uses of base address */
        int val1 = base_ptr[0] * sum1;
        int val2 = base_ptr[1] * sum2;
        
        /* More computations to increase pressure */
        int tmp1 = val1 + (idx * 13);      /* Constant for remat */
        int tmp2 = val2 + (idx * 17);      /* Another constant */
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = tmp3 & 0xFF;
        
        /* Use volatile to prevent optimization */
        if (__builtin_expect(v5 > 0, 1)) {
            tmp4 += v5;
        }
        
        array_sum += tmp4;
        
        /* Nested loop with more computations */
        for (int jdx = 0; jdx < 4; jdx++) {
            int inner_tmp = (tmp4 * jdx) + (sum3 >> jdx);
            array_sum += inner_tmp & 0xF;
        }
    }
    
    /* Final aggregation */
    int result = sum1 + sum2 + sum3 + sum4 + array_sum;
    
    /* Inline assembly to create complex dataflow patterns */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+r" (result)
        : "r" (v5)
        : "%eax", "cc"
    );
    
    /* More mixed-mode operations */
    short final_short = (short)result;
    int final_int = (int)final_short * 2;
    result = result + final_int;
    
    return result;
}

/* Another function to create more compilation context */
void __attribute__((noinline))
helper_function(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex addressing modes */
        arr[i] = arr[i] * 3 + (i & 0xF);
        
        /* Bitfield-like operations */
        int upper = (arr[i] >> 16) & 0xFFFF;
        int lower = arr[i] & 0xFFFF;
        arr[i] = (upper * lower) | (upper << 16);
    }
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call high-pressure function multiple times with different args */
    int total = 0;
    for (int iter = 0; iter < 100; iter++) {
        int result = high_pressure_function(
            iter, iter*2, iter*3, iter*4, iter*5,
            (short)(iter*6), (short)(iter*7), (short)(iter*8)
        );
        total += result;
        
        /* Modify globals to affect future iterations */
        if (iter % 10 == 0) {
            helper_function(global_array, 64);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
