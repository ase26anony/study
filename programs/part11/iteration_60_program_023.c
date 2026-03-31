/* Test case for GCC early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    short sa, sb, sc, sd, se, sf, sg, sh;
    
    /* Initial computations creating many def-use chains */
    a = x1 + x2 + v1;
    b = a * x3 - v2;
    c = b ^ x4;
    d = c | x5;
    e = d - x6;
    f = e * x7;
    g = f / (x8 ? x8 : 1);
    h = g << x9;
    i = h >> x10;
    j = i & x1;
    
    /* Mode mixing: integer to short conversions */
    sa = (short)(a + b);
    sb = (short)(c - d);
    sc = (short)(e ^ f);
    sd = (short)(g | h);
    
    /* More computations with mixed modes */
    k = (int)sa * j;
    l = (int)sb + k;
    m = (int)sc - l;
    n = (int)sd ^ m;
    
    /* Complex address calculations (potential rematerialization candidates) */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use addresses in computations */
    o = *ptr1 + n;
    p = *ptr2 - o;
    q = *ptr3 * p;
    
    /* More intermediate values */
    r = q << 2;
    s = r >> 1;
    t = s & 0x7FFF;
    u = t | 0x8000;
    v = u ^ 0xFFFF;
    w = v + 1;
    x = w - 2;
    y = x * 3;
    z = y / 4;
    aa = z % 5;
    ab = aa << 6;
    ac = ab >> 7;
    ad = ac & 0xF;
    ae = ad | 0x10;
    af = ae ^ 0x1F;
    
    /* More mode conversions */
    se = (short)(af + ae);
    sf = (short)(ad - ac);
    sg = (short)(ab ^ aa);
    sh = (short)(z | y);
    
    /* Use volatile as dataflow barrier */
    if (__builtin_expect(v1 > 0, 1)) {
        /* Additional computations after barrier */
        int ba = (int)se * af;
        int bb = (int)sf + ba;
        int bc = (int)sg - bb;
        int bd = (int)sh ^ bc;
        
        /* Switch statement creating complex control flow */
        switch (bd & 0x7) {
            case 0:
                return a + b + c + sa + sb;
            case 1:
                return d + e + f + sc + sd;
            case 2:
                return g + h + i + se + sf;
            case 3:
                return j + k + l + sg + sh;
            case 4:
                return m + n + o + (int)sa * 2;
            case 5:
                return p + q + r + (int)sb / 2;
            case 6:
                return s + t + u + (int)sc << 1;
            case 7:
                return v + w + x + (int)sd >> 1;
            default:
                return y + z + aa + ab;
        }
    }
    
    return af;
}

/* Another function with inline assembly to create complex DF references */
int __attribute__((noinline))
asm_mixed(int x, int y) {
    int result;
    
    /* Inline assembly with multiple operands */
    asm volatile (
        "addl %1, %0\n\t"
        "imull %2, %0\n\t"
        "andl %3, %0"
        : "=r" (result)
        : "r" (x), "r" (y), "r" (0xFF),
          "0" (0)  /* Initial value in result */
        : "cc"
    );
    
    return result;
}

/* Function using bit-fields for sub-register accesses */
struct packed_data {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
};

int __attribute__((noinline))
bitfield_ops(struct packed_data *pd) {
    /* Operations causing mode changes */
    unsigned int x = pd->a;      /* 4-bit to 32-bit */
    unsigned int y = pd->b << 4; /* 8-bit to 32-bit with shift */
    unsigned int z = pd->c;      /* 12-bit to 32-bit */
    
    /* Mixed-width computations */
    int r1 = x * y;
    int r2 = (short)z * (short)y;  /* Mode conversion */
    int r3 = r1 + r2;
    
    /* More computations to increase register pressure */
    int t1 = r3 << pd->d;
    int t2 = t1 >> 4;
    int t3 = t2 & 0xFFF;
    int t4 = t3 | 0x1000;
    int t5 = t4 ^ 0x1FFF;
    
    return t5;
}

/* Main function with maximum register pressure */
int main(void) {
    int i, total = 0;
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Create many live values across loop iterations */
    for (i = 0; i < 1000; i++) {
        /* Varying inputs to prevent constant propagation */
        int base = i + v1;
        
        /* Call compute_heavy with many arguments */
        int r1 = compute_heavy(
            base, base + 1, base + 2, base + 3, base + 4,
            base + 5, base + 6, base + 7, base + 8, base + 9
        );
        
        /* Call asm_mixed creating complex DF */
        int r2 = asm_mixed(r1, base);
        
        /* Bitfield operations */
        struct packed_data pd = {
            .a = (base >> 0) & 0xF,
            .b = (base >> 4) & 0xFF,
            .c = (base >> 12) & 0xFFF,
            .d = (base >> 24) & 0xFF
        };
        int r3 = bitfield_ops(&pd);
        
        /* More arithmetic to chain values */
        int t1 = r1 * r2;
        int t2 = t1 + r3;
        int t3 = t2 ^ base;
        int t4 = t3 << 3;
        int t5 = t4 >> 1;
        int t6 = t5 & 0x7FFFFFFF;
        
        /* Use volatile condition as barrier */
        if (__builtin_expect(v2 > 0, 1)) {
            total += t6;
            
            /* Additional mode mixing */
            short st = (short)(t6 & 0xFFFF);
            total += (int)st * 2;
            
            /* Address calculation that might be rematerialized */
            int *addr = &global_array[(t6 + i) & 0xFF];
            total += *addr;
        }
        
        /* Prevent loop unrolling from reducing pressure too much */
        if (i % 7 == 0) {
            v1 = (v1 * 3) & 0xFF;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
