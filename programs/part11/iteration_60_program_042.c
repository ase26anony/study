/* Test case for early-remat.cc lines 930-937 */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to force dataflow complexity */
volatile int g_volatile = 12345;

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
high_pressure_function(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10)
{
    /* Many local variables to create register pressure */
    int a = x1 + x2;
    int b = a * x3;
    int c = b - x4;
    int d = c ^ x5;
    int e = d | x6;
    int f = e & x7;
    int g = f + x8;
    int h = g - x9;
    int i = h * x10;
    int j = i ^ x1;
    int k = j | x2;
    int l = k & x3;
    int m = l + x4;
    int n = m - x5;
    int o = n * x6;
    int p = o ^ x7;
    int q = p | x8;
    int r = q & x9;
    int s = r + x10;
    int t = s - x1;
    
    /* Mixed-type arithmetic to create mode conversions */
    short s1 = (short)(a & 0xFFFF);
    short s2 = (short)(b & 0xFFFF);
    int i1 = (int)s1 * (int)s2;  /* Mode change: SI -> HI -> SI */
    
    long long ll1 = (long long)i * (long long)j;
    long long ll2 = (long long)k * (long long)l;
    
    /* Complex expression with many intermediate values */
    int u = t + i1;
    int v = u * (int)(ll1 & 0xFFFFFFFF);
    int w = v ^ (int)(ll2 & 0xFFFFFFFF);
    
    /* Use volatile as dataflow barrier */
    if (__builtin_expect(g_volatile > 10000, 0)) {
        u += g_volatile;
        v -= g_volatile;
    }
    
    /* Switch with many cases using different subsets of live values */
    int selector = (w & 0x7);  /* 0-7 */
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + c + (short)(d & 0xFF);  /* Mixed modes */
            break;
        case 1:
            result = e * f * g * (int)((short)h);  /* Mode conversion */
            break;
        case 2:
            result = i ^ j ^ k ^ l;
            break;
        case 3:
            result = m + n + o + p;
            break;
        case 4:
            result = q * r * s * t;
            break;
        case 5:
            result = u & v & w & i1;
            break;
        case 6:
            /* Force address calculation rematerialization */
            int *ptr = &result;
            *ptr += a + b + c + d + e;
            result += *ptr;
            break;
        case 7:
            /* More mixed-type operations */
            result = (short)((a >> 8) & 0xFF) * 
                     (short)((b >> 8) & 0xFF) *
                     (short)((c >> 8) & 0xFF);
            break;
        default:
            result = 42;  /* Constant for rematerialization */
    }
    
    /* Final complex computation using all values */
    result += a - b + c - d + e - f + g - h + i - j + 
              k - l + m - n + o - p + q - r + s - t + 
              u - v + w - i1 + (int)(ll1 >> 32) + (int)(ll2 >> 32);
    
    return result;
}

/* Another function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure_function(int iterations)
{
    int i, j, k;
    int sum = 0;
    
    /* Multiple induction variables */
    for (i = 0, j = 100, k = 1000; i < iterations; i++, j--, k += 2) {
        /* Many intermediate values in loop */
        int t1 = i * j;
        int t2 = j * k;
        int t3 = k * i;
        int t4 = t1 + t2;
        int t5 = t2 + t3;
        int t6 = t3 + t1;
        int t7 = t4 * t5;
        int t8 = t5 * t6;
        int t9 = t6 * t4;
        int t10 = t7 ^ t8;
        int t11 = t8 ^ t9;
        int t12 = t9 ^ t7;
        
        /* Address calculations that could be rematerialized */
        int *ptr1 = &t1;
        int *ptr2 = &t2;
        int *ptr3 = &t3;
        
        /* Use in mixed modes */
        short s_t1 = (short)(t1 & 0xFFFF);
        short s_t2 = (short)(t2 & 0xFFFF);
        int mixed = (int)s_t1 * (int)s_t2 * (int)((short)(t3 & 0xFFFF));
        
        /* Complex condition with volatile */
        if (__builtin_expect((g_volatile & 1) == (i & 1), 0)) {
            t10 += mixed;
            t11 -= mixed;
            t12 ^= mixed;
        }
        
        /* Switch inside loop for complex CFG */
        switch (i & 3) {
            case 0: sum += t10; break;
            case 1: sum += t11 + (short)(t12 & 0xFF); break;  /* Mode mix */
            case 2: sum += t12 * mixed; break;
            case 3: sum += t10 ^ t11 ^ t12 ^ mixed; break;
        }
        
        /* More intermediate values */
        int t13 = t10 * 31;
        int t14 = t11 * 37;
        int t15 = t12 * 41;
        int t16 = t13 + t14 + t15;
        int t17 = t13 ^ t14 ^ t15;
        int t18 = (t16 & 0xFFFF) * (t17 & 0xFFFF);
        
        sum += t18;
        
        /* Force spill/remat candidates with constants */
        sum += 42 * i;      /* Constant 42 */
        sum += 100 * j;     /* Constant 100 */
        sum += 1000 * k;    /* Constant 1000 */
    }
    
    return sum;
}

/* Function using inline assembly for complex dataflow */
void __attribute__((noinline))
asm_dataflow(int *a, int *b, int *c)
{
    /* Inline assembly with multiple constraints */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        : "=r" (*a)
        : "r" (b), "r" (c)
        : "%eax", "memory"
    );
    
    /* More assembly with different constraints */
    int tmp;
    asm volatile (
        "imull %1, %0\n\t"
        "addl $42, %0\n\t"
        : "+r" (tmp)
        : "r" (*b)
        : "cc"
    );
    
    *c = tmp;
}

/* Main function with everything combined */
int main(void)
{
    int i, total = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    int inputs[10];
    for (i = 0; i < 10; i++) {
        inputs[i] = g_volatile + i * 7;
    }
    
    /* Call high pressure function many times */
    for (i = 0; i < 100; i++) {
        total += high_pressure_function(
            inputs[0] + i,
            inputs[1] - i,
            inputs[2] * (i + 1),
            inputs[3] ^ i,
            inputs[4] | i,
            inputs[5] & (i * 3),
            inputs[6] + (i << 2),
            inputs[7] - (i >> 1),
            inputs[8] * (i % 7),
            inputs[9] ^ (i * 11)
        );
        
        /* Modify inputs to prevent optimization */
        inputs[i % 10] += total & 0xFF;
    }
    
    /* Call loop pressure function */
    total += loop_pressure_function(50);
    
    /* Use inline assembly function */
    int x = total, y = total * 2, z = total / 2;
    asm_dataflow(&x, &y, &z);
    total += x + y + z;
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0;
}
