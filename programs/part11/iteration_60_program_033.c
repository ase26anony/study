/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline))
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4 | v3;
    int d = c - x5 * v4;
    int e = d + x6 / (v5 ? v5 : 1);
    int f = e & x7 | v6;
    int g = f * x8 + v7;
    int h = g - x9 ^ v8;
    int i = h | x10 & v9;
    int j = i * a + v10;
    
    /* More intermediate values with mixed operations */
    int k = (j << 2) | (b >> 3);
    int l = k * c - d;
    int m = l ^ e + f;
    int n = m & g | h;
    int o = n - i * j;
    int p = o + k / (l ? l : 1);
    int q = p ^ m & n;
    int r = q * o - p;
    int s = r | q ^ r;
    int t = s + t * v1;  /* Self-reference to create complex DF */
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)a;
    short s2 = (short)b;
    short s3 = s1 + s2;
    int i1 = (int)s3 * c;
    
    char c1 = (char)d;
    char c2 = (char)e;
    char c3 = c1 - c2;
    int i2 = (int)c3 * f;
    
    /* Complex switch to create control flow with different live sets */
    int selector = (t & 0x7) + v1;
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + (short)c + (char)d;
            break;
        case 1:
            result = e - f + (short)g - (char)h;
            break;
        case 2:
            result = i * j + (short)k * (char)l;
            break;
        case 3:
            result = m & n | (short)o & (char)p;
            break;
        case 4:
            result = q ^ r + (short)s ^ (char)t;
            break;
        case 5:
            result = i1 + i2 + s1 - c1;
            break;
        case 6:
            result = (a << s1) | (b >> c1);
            break;
        case 7:
            result = (e * s2) - (f / (c2 ? c2 : 1));
            break;
        default:
            result = x1 + x2 + x3;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use addresses in computations */
    result += *ptr1 * v1;
    result += ptr2 - ptr1;
    result += (int)(ptr3) & 0xFF;
    
    /* Inline assembly to create complex dataflow */
    asm volatile ("# Assembly barrier %0 %1 %2" 
                  : "+r" (result), "+r" (a), "+r" (b)
                  : "r" (c), "r" (d)
                  : "memory", "cc");
    
    /* More computations after assembly */
    int u = result * v2 + a;
    int v = u ^ v3 - b;
    int w = v | v4 * c;
    int x = w & v5 + d;
    int y = x ^ v6 - e;
    int z = y | v7 * f;
    
    /* Final aggregation */
    return result + u + v + w + x + y + z + i1 + i2;
}

/* Main function with loop to increase pressure */
int main(void) {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * i;
    }
    
    /* Loop to create sustained register pressure */
    for (int iter = 0; iter < 1000; iter++) {
        /* Varying inputs to prevent constant propagation */
        int base = iter + v1;
        
        /* Call compute_heavy with many arguments */
        int res = compute_heavy(
            base + 0,  base + 1,  base + 2,  base + 3,  base + 4,
            base + 5,  base + 6,  base + 7,  base + 8,  base + 9
        );
        
        /* Use __builtin_expect to create conditional blocks */
        if (__builtin_expect((res & 1) != 0, 0)) {
            total += res * 3;
        } else {
            total += res / 2;
        }
        
        /* Additional computations in loop */
        int temp1 = total ^ (iter * 7);
        int temp2 = temp1 & 0xFFFF;
        short temp3 = (short)temp2;
        int temp4 = (int)temp3 * v8;
        
        total = (total + temp4) & 0x7FFFFFFF;
        
        /* Memory operations with address calculations */
        int idx = (iter * 13) & 0xFF;
        int *addr = &global_array[idx];
        total += *addr + (int)addr;
        
        /* Another switch for control flow complexity */
        switch (iter & 3) {
            case 0:
                total += global_array[temp2 & 0xFF] * 2;
                break;
            case 1:
                total -= global_array[(temp2 >> 8) & 0xFF] / 2;
                break;
            case 2:
                total ^= global_array[idx] | temp4;
                break;
            case 3:
                total |= global_array[(iter * 17) & 0xFF] & temp4;
                break;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
