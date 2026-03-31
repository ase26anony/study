#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
long compute_chain(int start, int iterations) {
    long a = start, b = start * 2, c = start * 3;
    long d = start * 4, e = start * 5, f = start * 6;
    long g = start * 7, h = start * 8, i = start * 9;
    long j = start * 10, k = start * 11, l = start * 12;
    long m = start * 13, n = start * 14, o = start * 15;
    long p = start * 16, q = start * 17, r = start * 18;
    long s = start * 19, t = start * 20;
    
    /* Long chain of dependent operations */
    for (int idx = 0; idx < iterations; idx++) {
        a += b ^ c;
        b += c ^ d;
        c += d ^ e;
        d += e ^ f;
        e += f ^ g;
        f += g ^ h;
        g += h ^ i;
        h += i ^ j;
        i += j ^ k;
        j += k ^ l;
        k += l ^ m;
        l += m ^ n;
        m += n ^ o;
        n += o ^ p;
        o += p ^ q;
        p += q ^ r;
        q += r ^ s;
        r += s ^ t;
        s += t ^ a;
        t += a ^ b;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent exit condition */
        if (__builtin_expect_with_probability((a & 0xFFF) == 0, 0, 0.3)) {
            break;
        }
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    int local1 = val * 2;
    int local2 = val * 3;
    int local3 = val * 4;
    int local4 = val * 5;
    
    /* Mix of operations */
    local1 ^= local2;
    local2 += local3;
    local3 *= local4;
    local4 -= local1;
    
    if (depth > 0) {
        /* Recursive call creates scheduling boundaries */
        int res = recursive_compute(depth - 1, val + depth);
        local1 ^= res;
        local2 += res;
        local3 *= res;
        local4 -= res;
    }
    
    /* Volatile read to create uncertainty */
    volatile int* ptr1 = (volatile int*)&g_var1;
    volatile int* ptr2 = (volatile int*)&g_var2;
    
    /* Pointer aliasing access */
    int temp = *ptr1 + *ptr2;
    
    return local1 + local2 + local3 + local4 + temp;
}

__attribute__((noinline, optimize("O3")))
unsigned long switch_complex(int selector) {
    unsigned long result = 0;
    unsigned long v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    unsigned long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    unsigned long v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    unsigned long v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Complex switch with many cases - each creates different scheduling pattern */
    switch (selector & 0xF) {
        case 0:
            v1 += v2 * v3;
            v4 ^= v5 | v6;
            v7 -= v8 & v9;
            v10 *= v11 ^ v12;
            result = v1 + v4 + v7 + v10;
            break;
        case 1:
            v2 += v3 * v4;
            v5 ^= v6 | v7;
            v8 -= v9 & v10;
            v11 *= v12 ^ v13;
            result = v2 + v5 + v8 + v11;
            break;
        case 2:
            v3 += v4 * v5;
            v6 ^= v7 | v8;
            v9 -= v10 & v11;
            v12 *= v13 ^ v14;
            result = v3 + v6 + v9 + v12;
            break;
        case 3:
            v4 += v5 * v6;
            v7 ^= v8 | v9;
            v10 -= v11 & v12;
            v13 *= v14 ^ v15;
            result = v4 + v7 + v10 + v13;
            break;
        case 4:
            v5 += v6 * v7;
            v8 ^= v9 | v10;
            v11 -= v12 & v13;
            v14 *= v15 ^ v16;
            result = v5 + v8 + v11 + v14;
            break;
        case 5:
            v6 += v7 * v8;
            v9 ^= v10 | v11;
            v12 -= v13 & v14;
            v15 *= v16 ^ v17;
            result = v6 + v9 + v12 + v15;
            break;
        case 6:
            v7 += v8 * v9;
            v10 ^= v11 | v12;
            v13 -= v14 & v15;
            v16 *= v17 ^ v18;
            result = v7 + v10 + v13 + v16;
            break;
        case 7:
            v8 += v9 * v10;
            v11 ^= v12 | v13;
            v14 -= v15 & v16;
            v17 *= v18 ^ v19;
            result = v8 + v11 + v14 + v17;
            break;
        case 8:
            v9 += v10 * v11;
            v12 ^= v13 | v14;
            v15 -= v16 & v17;
            v18 *= v19 ^ v20;
            result = v9 + v12 + v15 + v18;
            break;
        case 9:
            v10 += v11 * v12;
            v13 ^= v14 | v15;
            v16 -= v17 & v18;
            v19 *= v20 ^ v1;
            result = v10 + v13 + v16 + v19;
            break;
        case 10:
            v11 += v12 * v13;
            v14 ^= v15 | v16;
            v17 -= v18 & v19;
            v20 *= v1 ^ v2;
            result = v11 + v14 + v17 + v20;
            break;
        case 11:
            v12 += v13 * v14;
            v15 ^= v16 | v17;
            v18 -= v19 & v20;
            v1 *= v2 ^ v3;
            result = v12 + v15 + v18 + v1;
            break;
        case 12:
            v13 += v14 * v15;
            v16 ^= v17 | v18;
            v19 -= v20 & v1;
            v2 *= v3 ^ v4;
            result = v13 + v16 + v19 + v2;
            break;
        case 13:
            v14 += v15 * v16;
            v17 ^= v18 | v19;
            v20 -= v1 & v2;
            v3 *= v4 ^ v5;
            result = v14 + v17 + v20 + v3;
            break;
        case 14:
            v15 += v16 * v17;
            v18 ^= v19 | v20;
            v1 -= v2 & v3;
            v4 *= v5 ^ v6;
            result = v15 + v18 + v1 + v4;
            break;
        default: /* case 15 */
            v16 += v17 * v18;
            v19 ^= v20 | v1;
            v2 -= v3 & v4;
            v5 *= v6 ^ v7;
            result = v16 + v19 + v2 + v5;
            break;
    }
    
    return result;
}

__attribute__((noinline, optimize("O3")))
long software_pipelined_loop(int* data, int size) {
    long sum = 0;
    int i = 0;
    
    /* Manual software pipelining attempt */
    if (size > 3) {
        int a = data[0];
        int b = data[1];
        int c = data[2];
        
        i = 3;
        
        /* Loop with irregular control flow */
        do {
            int d = data[i];
            
            /* Complex dependency chain */
            a = (a ^ b) + c;
            b = (b ^ c) + d;
            c = (c ^ d) + a;
            d = (d ^ a) + b;
            
            sum += a + b + c + d;
            
            /* Conditional break with unpredictable probability */
            if (__builtin_expect_with_probability((sum & 0x3FF) == 0, 0, 0.1)) {
                /* goto creates irregular control flow */
                goto early_exit;
            }
            
            i++;
            
            /* Memory operation in the middle */
            asm volatile("" : : : "memory");
            
        } while (i < size);
        
        early_exit:
        sum += a * 100 + b * 10 + c;
    }
    
    return sum;
}

__attribute__((optimize("O3")))
int main() {
    unsigned long final_result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37) & 0xFF;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    final_result ^= compute_chain(1, 1000);
    
    /* Kernel 2: Recursive computation */
    for (int i = 0; i < 50; i++) {
        final_result += recursive_compute(3, i);
    }
    
    /* Kernel 3: Complex switch statements */
    for (int i = 0; i < 100; i++) {
        final_result ^= switch_complex(i);
    }
    
    /* Kernel 4: Software pipelined loop */
    final_result += software_pipelined_loop(g_array, 256);
    
    /* Kernel 5: Mixed operations with volatile accesses */
    {
        int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
        int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
        int x11 = 11, x12 = 12, x13 = 13, x14 = 14, x15 = 15;
        int x16 = 16, x17 = 17, x18 = 18, x19 = 19, x20 = 20;
        
        /* Unrolled loop with memory barriers */
        for (int i = 0; i < 100; i++) {
            x1 = (x1 + x2) ^ x3;
            x2 = (x2 + x3) ^ x4;
            x3 = (x3 + x4) ^ x5;
            asm volatile("" : : : "memory");
            x4 = (x4 + x5) ^ x6;
            x5 = (x5 + x6) ^ x7;
            x6 = (x6 + x7) ^ x8;
            asm volatile("" : : : "memory");
            x7 = (x7 + x8) ^ x9;
            x8 = (x8 + x9) ^ x10;
            x9 = (x9 + x10) ^ x11;
            asm volatile("" : : : "memory");
            x10 = (x10 + x11) ^ x12;
            x11 = (x11 + x12) ^ x13;
            x12 = (x12 + x13) ^ x14;
            
            /* Volatile write */
            g_var1 = x1;
            
            x13 = (x13 + x14) ^ x15;
            x14 = (x14 + x15) ^ x16;
            x15 = (x15 + x16) ^ x17;
            asm volatile("" : : : "memory");
            x16 = (x16 + x17) ^ x18;
            x17 = (x17 + x18) ^ x19;
            x18 = (x18 + x19) ^ x20;
            x19 = (x19 + x20) ^ x1;
            x20 = (x20 + x1) ^ x2;
            
            /* Volatile read */
            x1 ^= g_var2;
        }
        
        final_result += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 +
                       x11 + x12 + x13 + x14 + x15 + x16 + x17 + x18 + x19 + x20;
    }
    
    printf("Result: %lu\n", final_result);
    return 0;
}
