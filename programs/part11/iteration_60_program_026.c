/* Test case for GCC early rematerialization pass - targeting lines 930-937 */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Global array for address calculations */
int global_array[1000];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Use volatiles to prevent constant propagation */
    int base1 = x1 + v1;
    int base2 = x2 + v2;
    
    /* Long chain of arithmetic operations - creates many intermediate values */
    a = base1 + base2;
    b = a * x3 + v3;
    c = b - x4;
    d = c ^ x5;
    e = d | x6;
    f = e & x7;
    g = f + x8;
    h = g - x9;
    i = h * x10;
    j = i / (v4 + 1);
    k = j << 2;
    l = k >> 1;
    m = l & 0xFF;
    n = m | 0x80;
    o = n ^ 0x55;
    p = o + v5;
    q = p - v6;
    r = q * v7;
    s = r / v8;
    t = s % (v9 + 1);
    
    /* More operations creating def-use chains */
    u = t + a;  /* Use 'a' again */
    v = u - b;  /* Use 'b' again */
    w = v * c;  /* Use 'c' again */
    x = w / d;  /* Use 'd' again */
    y = x | e;  /* Use 'e' again */
    z = y & f;  /* Use 'f' again */
    aa = z ^ g; /* Use 'g' again */
    ab = aa + h; /* Use 'h' again */
    ac = ab - i; /* Use 'i' again */
    ad = ac * j; /* Use 'j' again */
    ae = ad / k; /* Use 'k' again */
    af = ae | l; /* Use 'l' again */
    ag = af & m; /* Use 'm' again */
    ah = ag ^ n; /* Use 'n' again */
    ai = ah + o; /* Use 'o' again */
    aj = ai - p; /* Use 'p' again */
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)aj;
    char c1 = (char)s1;
    int i1 = (int)c1;
    long l1 = (long)i1;
    
    /* Bit-field like operations */
    struct {
        unsigned int low : 8;
        unsigned int high : 8;
        unsigned int mid : 16;
    } bits;
    
    bits.low = i1 & 0xFF;
    bits.high = (i1 >> 8) & 0xFF;
    bits.mid = (i1 >> 16) & 0xFFFF;
    
    /* Complex switch to create control flow with different live sets */
    int selector = (aj + v10) % 8;
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + (short)c;  /* Mode mix: int + int + short */
            break;
        case 1:
            result = d - e - (char)f;   /* Mode mix: int - int - char */
            break;
        case 2:
            result = g * h * (int)s1;   /* Using short converted to int */
            break;
        case 3:
            result = i / j / (long)l1;  /* Using long */
            break;
        case 4:
            result = k | l | bits.low;  /* Using bit-field */
            break;
        case 5:
            result = m & n & bits.high; /* Using bit-field */
            break;
        case 6:
            result = o ^ p ^ bits.mid;  /* Using bit-field */
            break;
        case 7:
            result = q + r + c1;        /* Using char */
            break;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[a % 1000];
    int *ptr2 = &global_array[b % 1000];
    int *ptr3 = &global_array[c % 1000];
    
    /* Multiple memory accesses using different base addresses */
    *ptr1 = result;
    *ptr2 = result + 1;
    *ptr3 = result - 1;
    
    /* Inline assembly to create complex dataflow */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (asm_out1)
        : "r" (result), "r" (v1)
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (asm_out2)
        : "r" (asm_out1)
        : "cc"
    );
    
    /* Loop with induction variables */
    int sum = 0;
    for (int idx1 = 0; idx1 < 10; idx1++) {
        for (int idx2 = 0; idx2 < 10; idx2++) {
            /* Use many live values in address calculation */
            int index = (idx1 * a + idx2 * b + c) % 1000;
            sum += global_array[index] + idx1 + idx2;
            
            /* More arithmetic to increase pressure */
            int temp1 = idx1 * d + idx2 * e;
            int temp2 = idx1 * f + idx2 * g;
            int temp3 = temp1 ^ temp2;
            sum += temp3;
        }
    }
    
    /* Conditional with __builtin_expect */
    if (__builtin_expect((sum & 1) != 0, 0)) {
        /* Merge many live values */
        result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t;
    } else {
        result = sum + asm_out2;
    }
    
    /* Final mixed-type expression */
    return result + (short)asm_out1 + (char)v10;
}

/* Another function to create more compilation context */
void __attribute__((noinline))
use_vector_types() {
    /* Vector types for different machine modes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    v8hi vec4 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi vec5 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8hi vec6 = vec4 * vec5;
    
    /* Use results to prevent elimination */
    global_array[0] = vec3[0] + vec6[0];
}

int main() {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 1000; i++) {
        global_array[i] = i;
    }
    
    /* Call vector function */
    use_vector_types();
    
    /* Multiple calls with different arguments to create varied patterns */
    for (int iter = 0; iter < 100; iter++) {
        result ^= compute_heavy(
            iter, iter+1, iter+2, iter+3, iter+4,
            iter+5, iter+6, iter+7, iter+8, iter+9
        );
        
        /* Modify volatiles to change dataflow */
        v1 = (v1 * 3) % 100;
        v2 = (v2 * 5) % 100;
        v3 = (v3 * 7) % 100;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
