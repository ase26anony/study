/* Test case for GCC early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int p1, int p2, int p3, int p4, int p5, 
                       int p6, int p7, int p8, int p9, int p10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    int result = 0;
    
    /* Initial computations creating many live values */
    a = p1 + p2;           /* 1 */
    b = p3 - p4;           /* 2 */
    c = a * b;             /* 3 */
    d = p5 ^ p6;           /* 4 */
    e = c + d;             /* 5 */
    f = p7 & p8;           /* 6 */
    g = e | f;             /* 7 */
    h = p9 << 2;           /* 8 */
    i = g >> 1;            /* 9 */
    j = h + i;             /* 10 */
    k = p10 * 3;           /* 11 */
    l = j - k;             /* 12 */
    m = a + b + c;         /* 13 */
    n = d ^ e ^ f;         /* 14 */
    o = g & h & i;         /* 15 */
    p = j | k | l;         /* 16 */
    q = m * n;             /* 17 */
    r = o + p;             /* 18 */
    s = q - r;             /* 19 */
    t = s * 2;             /* 20 */
    
    /* More intermediate values with different modes */
    short sa = (short)a;   /* Mode conversion */
    short sb = (short)b;
    int u1 = (int)sa * sb; /* 21 - mixed mode computation */
    
    char ca = (char)c;     /* More mode conversions */
    char cb = (char)d;
    int u2 = (int)ca + cb; /* 22 */
    
    /* Use volatile to create dataflow complexity */
    if (__builtin_expect(v1 > 0, 1)) {
        u = t + u1;        /* 23 */
        v = u - u2;        /* 24 */
    } else {
        u = t - u1;        /* 25 */
        v = u + u2;        /* 26 */
    }
    
    /* Loop with high register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Complex expression chain - each creates a new temporary */
        w = u + v + iter;          /* 27 */
        x = w * 3 - iter;          /* 28 */
        y = x >> (iter & 3);       /* 29 */
        z = y ^ (iter * 7);        /* 30 */
        aa = z & 0xFF;             /* 31 */
        ab = aa + global_array[iter & 0xFF]; /* Address calculation - remat candidate */
        
        /* Switch to create complex control flow */
        switch (ab & 7) {
            case 0:
                ac = w + x;        /* 32 */
                ad = y - z;        /* 33 */
                ae = ac * ad;      /* 34 */
                result += ae;
                break;
            case 1:
                ac = w - x;        /* 35 */
                ad = y + z;        /* 36 */
                ae = ac / (ad ? ad : 1); /* 37 */
                result += ae;
                break;
            case 2:
                /* More mode mixing */
                short sac = (short)ac;
                short sad = (short)ad;
                af = (int)sac * sad; /* 38 */
                result += af;
                break;
            case 3:
                ag = aa << 2;      /* 39 */
                ah = ab >> 1;      /* 40 */
                ai = ag ^ ah;      /* 41 */
                result += ai;
                break;
            case 4:
                /* Use inline asm for complex dataflow */
                asm volatile (
                    "addl %1, %0\n\t"
                    "subl %2, %0"
                    : "+r" (result)
                    : "r" (w), "r" (x)
                    : "cc"
                );
                break;
            case 5:
                /* More arithmetic */
                aj = w * x - y * z; /* 42 */
                result += aj;
                break;
            case 6:
                /* Address calculation reused - good remat candidate */
                int *ptr1 = &global_array[(w + iter) & 0xFF];
                int *ptr2 = &global_array[(x - iter) & 0xFF];
                result += *ptr1 + *ptr2;
                break;
            case 7:
                /* Mixed width operations */
                char cb1 = (char)w;
                char cb2 = (char)x;
                short ss1 = (short)y;
                short ss2 = (short)z;
                result += (int)cb1 * cb2 + (int)ss1 * ss2;
                break;
        }
        
        /* Update some values for next iteration */
        u = (u + v) ^ iter;
        v = (v - w) & 0xFFFF;
        
        /* Use volatile condition */
        if (__builtin_expect(v2 < iter, 0)) {
            w = w * 2;
            x = x / 2;
        }
    }
    
    /* Final complex expression using many live values */
    int final1 = a + c + e + g + i + k + m + o + q + s;
    int final2 = b + d + f + h + j + l + n + p + r + t;
    int final3 = u + v + w + x + y + z + aa + ab;
    
    result += final1 * final2 + final3;
    
    /* Use another volatile to prevent tail optimization */
    if (__builtin_expect(v3 != 0, 1)) {
        result = result ^ 0x12345678;
    }
    
    return result;
}

/* Helper to create more register pressure */
void nested_calls(int depth, int *sum) {
    if (depth <= 0) return;
    
    int local1 = depth * 2;
    int local2 = depth * 3;
    int local3 = local1 + local2;
    int local4 = local1 * local2;
    int local5 = local3 ^ local4;
    
    *sum += local5;
    
    /* Recursive call with many parameters */
    nested_calls(depth - 1, sum);
}

int main() {
    int total = 0;
    
    /* Call with many parameters to increase register pressure */
    for (int i = 0; i < 10; i++) {
        int res = complex_remat_test(
            i, i*2, i*3, i*4, i*5,
            i*6, i*7, i*8, i*9, i*10
        );
        total += res;
    }
    
    /* Additional nested calls for more complexity */
    int nested_sum = 0;
    nested_calls(5, &nested_sum);
    total += nested_sum;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
