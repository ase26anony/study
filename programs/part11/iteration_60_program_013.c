/* Test case for GCC early rematerialization pass
 * Targeting lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -funroll-loops -fno-schedule-insns -fno-schedule-insns2 test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5, 
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Initial computations creating many def-use chains */
    a = x1 + x2 + v1;          /* volatile creates barrier */
    b = a * x3 - x4;
    c = b ^ x5 + x6;
    d = c | x7 & x8;
    e = d - x9 * x10;
    f = e + (x1 ^ x2);
    g = f * x3 / (x4 + 1);
    h = g << (x5 & 3);
    i = h >> (x6 % 4);
    j = i + x7 - x8;
    k = j * x9 + x10;
    l = k ^ a ^ b;
    m = l | c & d;
    n = m - e + f;
    o = n * g / (h + 1);
    p = o << (i & 3);
    q = p >> (j % 4);
    r = q + k - l;
    s = r * m + n;
    t = s ^ o ^ p;
    u = t | q & r;
    v = u - s + t;
    w = v * u / (v + 1);
    x = w << (w & 3);
    y = x >> (x % 4);
    z = y + v - w;
    aa = z * x + y;
    ab = aa ^ z ^ aa;
    ac = ab | aa & ab;
    ad = ac - ab + ac;
    ae = ad * ac / (ad + 1);
    af = ae << (ae & 3);
    ag = af >> (af % 4);
    ah = ag + ae - af;
    ai = ah * ag + ah;
    aj = ai ^ ah ^ ai;
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)aj;
    char c1 = (char)ai;
    long long ll1 = (long long)ah * ag;
    float f1 = (float)af;
    
    /* Complex switch to create control flow complexity */
    int selector = (aj ^ v2) & 0xF;  /* volatile creates barrier */
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + (int)s1;
            break;
        case 1:
            result = c - d * (int)c1;
            break;
        case 2:
            result = e | f ^ (int)ll1;
            break;
        case 3:
            result = g & h + (int)f1;
            break;
        case 4:
            result = i << (j & 3);
            break;
        case 5:
            result = k >> (l % 4);
            break;
        case 6:
            result = m + n - o;
            break;
        case 7:
            result = p * q / (r + 1);
            break;
        case 8:
            result = s ^ t ^ u;
            break;
        case 9:
            result = v | w & x;
            break;
        case 10:
            result = y - z + aa;
            break;
        case 11:
            result = ab * ac / (ad + 1);
            break;
        case 12:
            result = ae << (af & 3);
            break;
        case 13:
            result = ag >> (ah % 4);
            break;
        case 14:
            result = ai + aj - result;
            break;
        case 15:
            result = (result ^ v3) + 1;  /* volatile creates barrier */
            break;
        default:
            result = 0;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Memory operations using different address calculations */
    *ptr1 = result + a;
    *ptr2 = result - b;
    *ptr3 = result ^ c;
    
    /* Inline assembly to create complex dataflow */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_out1)
        : "r" (result), "r" (d)
        : "%eax"
    );
    
    asm volatile (
        "imull %1, %2\n\t"
        "addl %%ecx, %0\n\t"
        : "=r" (asm_out2)
        : "r" (e), "r" (f), "0" (asm_out1)
        : "%ecx"
    );
    
    /* More mixed-type operations */
    unsigned short us1 = (unsigned short)asm_out2;
    signed char sc1 = (signed char)asm_out1;
    unsigned long ul1 = (unsigned long)result * asm_out2;
    
    /* Final computation using all values */
    int final = (result + asm_out1 + asm_out2 + 
                (int)us1 + (int)sc1 + (int)ul1 + 
                *ptr1 + *ptr2 + *ptr3);
    
    return final;
}

/* Main function with loop to increase pressure */
int main() {
    int i, j;
    int sum = 0;
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Outer loop to create more optimization opportunities */
    for (j = 0; j < 100; j++) {
        /* Call with many different arguments to prevent constant propagation */
        int result = compute_heavy(
            j + 1, j + 2, j + 3, j + 4, j + 5,
            j + 6, j + 7, j + 8, j + 9, j + 10
        );
        
        sum += result;
        
        /* Conditional with volatile to prevent optimization */
        if (v1 > 10000) {
            sum ^= result;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
