/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int compute_heavy(int x1, int x2, int x3, int x4, int x5,
                  int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to increase register pressure */
    int a = x1 + x2;      /* 1 */
    int b = a * x3;       /* 2 */
    int c = b - x4;       /* 3 */
    int d = c ^ x5;       /* 4 */
    int e = d | x6;       /* 5 */
    int f = e & x7;       /* 6 */
    int g = f + x8;       /* 7 */
    int h = g - x9;       /* 8 */
    int i = h * x10;      /* 9 */
    int j = i / (x1 + 1); /* 10 */
    int k = j << 2;       /* 11 */
    int l = k >> 1;       /* 12 */
    int m = l & 0xFF;     /* 13 */
    int n = m | 0x80;     /* 14 */
    int o = n ^ 0x55;     /* 15 */
    int p = o + v1;       /* 16 - volatile creates barrier */
    int q = p * v2;       /* 17 */
    int r = q - v3;       /* 18 */
    int s = r ^ v4;       /* 19 */
    int t = s | v5;       /* 20 */
    
    /* More operations to ensure at least 30 intermediate values */
    int u = t + a;        /* 21 */
    int v = u * b;        /* 22 */
    int w = v - c;        /* 23 */
    int x = w ^ d;        /* 24 */
    int y = x | e;        /* 25 */
    int z = y & f;        /* 26 */
    int aa = z + g;       /* 27 */
    int ab = aa - h;      /* 28 */
    int ac = ab * i;      /* 29 */
    int ad = ac / j;      /* 30 */
    
    /* Mixed-type arithmetic to create mode conversions */
    short s1 = (short)ad;          /* Convert to 16-bit */
    int i1 = (int)s1 * 2;          /* Convert back to 32-bit */
    unsigned short us1 = (unsigned short)i1;
    int i2 = (int)us1 + 100;
    
    /* Complex loop with address calculations (rematerialization candidates) */
    int sum = 0;
    for (int idx = 0; idx < 100; idx++) {
        /* Base address calculation - potential rematerialization candidate */
        int *addr = &global_array[idx];
        
        /* Use the address multiple times in different expressions */
        int val1 = *addr + idx;
        int val2 = *(addr + 1) - idx;
        int val3 = *(addr + 2) * idx;
        int val4 = *(addr + 3) ^ idx;
        
        /* Use many live values in the loop */
        val1 += a + b;
        val2 += c - d;
        val3 *= e ^ f;
        val4 |= g & h;
        
        /* Switch statement to create complex control flow */
        switch (idx % 7) {
            case 0:
                sum += val1 + i1;
                break;
            case 1:
                sum += val2 + i2;
                break;
            case 2:
                sum += val3 + (int)s1;  /* Mode conversion */
                break;
            case 3:
                sum += val4 + (int)us1; /* Mode conversion */
                break;
            case 4:
                sum += val1 * val2;
                break;
            case 5:
                sum += val3 | val4;
                break;
            case 6:
                sum += (val1 ^ val2) & (val3 | val4);
                break;
        }
        
        /* Inline assembly to create complex dataflow patterns */
        asm volatile (
            "addl %[v1], %[sum]\n\t"
            "subl %[v2], %[sum]\n\t"
            : [sum] "+r" (sum)
            : [v1] "r" (v1), [v2] "r" (v2)
            : "cc"
        );
    }
    
    /* More arithmetic with the accumulated sum */
    int result = sum;
    result += a * 2;
    result -= b / 3;
    result ^= c << 1;
    result |= d >> 2;
    result &= e + 5;
    result += f - 6;
    result *= g ^ 7;
    result /= h | 8;
    result += i & 9;
    result -= j + 10;
    
    /* Final volatile access to prevent optimization */
    result += v1 - v2 + v3 - v4 + v5;
    
    return result;
}

/* Another function to create more compilation context */
int secondary_computation(int base) {
    /* Bit-field structure to trigger sub-register accesses */
    struct packed {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits;
    
    bits.a = base & 0x7;
    bits.b = (base >> 3) & 0x1F;
    bits.c = (base >> 8) & 0xFF;
    bits.d = (base >> 16) & 0xFFFF;
    
    /* Operations on bit-fields cause mode changes */
    int x = bits.a;
    int y = bits.b;
    int z = bits.c;
    int w = bits.d;
    
    /* Mixed operations */
    return (x << 24) | (y << 16) | (z << 8) | w;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call with many arguments to create initial register pressure */
    int result = compute_heavy(
        100, 200, 300, 400, 500,
        600, 700, 800, 900, 1000
    );
    
    /* Additional computation to keep things live */
    result += secondary_computation(result);
    
    /* Use __builtin_expect to create conditional basic blocks */
    if (__builtin_expect(result > 1000000, 0)) {
        result /= 2;
    } else {
        result *= 3;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
