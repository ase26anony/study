/* Test case for early-remat.cc lines 930-937 */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20;

/* Global array to force address calculations */
int global_arr[100];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10)
{
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4;
    int d = c + (x5 << 2);
    int e = d | x6;
    int f = e - x7;
    int g = f * x8;
    int h = g / (x9 + 1);
    int i = h & x10;
    int j = i + (x1 ^ x2);
    int k = j * x3;
    int l = k - x4;
    int m = l | x5;
    int n = m ^ x6;
    int o = n + x7;
    int p = o * x8;
    int q = p / (x9 + 2);
    int r = q & x10;
    int s = r + (x2 ^ x3);
    int t = s * x4;
    int u = t - x5;
    int v = u | x6;
    int w = v ^ x7;
    int z = w + x8;
    
    /* Mixed-type operations to trigger mode changes */
    short sa = (short)a;
    short sb = (short)b;
    int mixed1 = (int)sa * (int)sb + vs1;
    short sc = (short)c + vs2;
    int mixed2 = (int)sc * d;
    
    /* Complex control flow with switch */
    int selector = (z ^ mixed1) & 7;
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + mixed1;
            break;
        case 1:
            result = c + d + mixed2;
            break;
        case 2:
            result = e + f + (int)sa;
            break;
        case 3:
            result = g + h + (int)sb;
            break;
        case 4:
            result = i + j + mixed1;
            break;
        case 5:
            result = k + l + mixed2;
            break;
        case 6:
            result = m + n + (int)sc;
            break;
        case 7:
            result = o + p + z;
            break;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_arr[a & 63];
    int *ptr2 = &global_arr[b & 63];
    int *ptr3 = &global_arr[c & 63];
    
    /* Use addresses in computations */
    result += *ptr1 + *ptr2 + *ptr3;
    
    /* More arithmetic to keep values live */
    result = result * 2 - 1;
    result = result ^ (mixed1 << 3);
    result = result + (mixed2 >> 2);
    
    /* Use __builtin_expect to create branch prediction hints
       that affect basic block layout */
    if (__builtin_expect(result > 1000, 0)) {
        result = result / 2;
    } else {
        result = result * 3;
    }
    
    return result;
}

/* Another function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations)
{
    int sum = 0;
    int i, j, k;
    
    /* Multiple induction variables */
    for (i = 0, j = 1, k = 2; i < iterations; i++, j++, k++) {
        /* Many live values across loop iterations */
        int t1 = i * v1;
        int t2 = j * v2;
        int t3 = k * v3;
        int t4 = t1 + t2;
        int t5 = t3 - t1;
        int t6 = t4 * t5;
        int t7 = t6 ^ t2;
        int t8 = t7 + t3;
        int t9 = t8 * i;
        int t10 = t9 - j;
        
        /* Mixed types within loop */
        short st1 = (short)t1;
        short st2 = (short)t2;
        int mixed = (int)st1 * (int)st2;
        
        /* Complex condition with many live values */
        if (__builtin_expect((t10 & 255) < 128, 1)) {
            sum += t4 + t5 + mixed;
        } else {
            sum += t6 + t7 + mixed;
        }
        
        /* Use all values to keep them live */
        sum = (sum + t8 + t9 + t10) & 0xFFFF;
        
        /* Address calculation that might be rematerialized */
        int idx = (i * j + k) & 63;
        sum += global_arr[idx];
    }
    
    return sum;
}

/* Function with inline assembly to create complex dataflow */
int __attribute__((noinline))
asm_dataflow(int x, int y)
{
    int result1, result2;
    
    /* Inline assembly with multiple outputs tied to C variables */
    asm volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        "movl %0, %1\n\t"
        "imull %2, %1"
        : "=&r" (result1), "=&r" (result2)
        : "r" (x), "r" (y)
        : "cc"
    );
    
    /* Use results in further computations */
    int combined = result1 + result2;
    
    /* More operations to create DF chains */
    combined = combined * 2;
    combined = combined ^ (x << 3);
    combined = combined + (y >> 2);
    
    return combined;
}

/* Main function that combines all patterns */
int main(void)
{
    int i, total = 0;
    
    /* Initialize global array */
    for (i = 0; i < 100; i++) {
        global_arr[i] = i * 3 + 1;
    }
    
    /* Create many calls with different arguments to prevent constant propagation */
    for (i = 0; i < 50; i++) {
        /* Vary arguments to prevent optimization */
        int arg1 = i + v1;
        int arg2 = i * 2 + v2;
        int arg3 = i * 3 + v3;
        int arg4 = i * 4 + v4;
        int arg5 = i * 5 + v5;
        int arg6 = (i << 1) ^ 0x55;
        int arg7 = (i << 2) ^ 0xAA;
        int arg8 = (i << 3) ^ 0xFF;
        int arg9 = i * 9 + 1;
        int arg10 = i * 10 + 2;
        
        /* Call compute_heavy with many live arguments */
        int res1 = compute_heavy(arg1, arg2, arg3, arg4, arg5,
                                 arg6, arg7, arg8, arg9, arg10);
        
        /* Call loop_pressure */
        int res2 = loop_pressure(10 + (i & 7));
        
        /* Call asm_dataflow */
        int res3 = asm_dataflow(arg1, arg2);
        
        /* Combine results */
        total += res1 + res2 + res3;
        
        /* Use volatile to prevent dead code elimination */
        if (v1) {
            total = total & 0x7FFFFFFF;
        }
    }
    
    /* Final computation using bit-fields (may trigger sub-register modes) */
    struct {
        unsigned int a : 5;
        unsigned int b : 7;
        unsigned int c : 10;
        unsigned int d : 10;
    } bits;
    
    bits.a = total & 0x1F;
    bits.b = (total >> 5) & 0x7F;
    bits.c = (total >> 12) & 0x3FF;
    bits.d = (total >> 22) & 0x3FF;
    
    int final_result = bits.a + bits.b * 2 + bits.c * 3 + bits.d * 4;
    
    printf("Result: %d\n", final_result);
    return final_result != 0;
}
