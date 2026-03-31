/* Test case for GCC early rematerialization pass - targeting lines 930-937 */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5, 
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to increase register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4;
    int d = c + x5 * 2;
    int e = d - x6 / 3;
    int f = e | x7;
    int g = f & x8;
    int h = g * x9;
    int i = h + x10;
    int j = i - a;
    int k = j * b;
    int l = k ^ c;
    int m = l + d;
    int n = m - e;
    int o = n | f;
    int p = o & g;
    int q = p * h;
    int r = q + i;
    int s = r - j;
    int t = s * k;
    int u = t ^ l;
    int v = u + m;
    int w = v - n;
    int z = w | o;
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)z;
    short s2 = (short)p;
    int mixed1 = (int)s1 * (int)s2;  /* Mode conversion: SI -> HI -> SI */
    
    char c1 = (char)(mixed1 & 0xFF);
    int mixed2 = (int)c1 * q;        /* QI -> SI */
    
    /* Complex address calculations (potential rematerialization candidates) */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use addresses in computations */
    int addr_sum = (*ptr1) + (*ptr2) + (*ptr3);
    
    /* Switch statement with different live value subsets */
    int selector = (mixed2 + addr_sum) & 0x7;
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + c + (int)s1;
            break;
        case 1:
            result = d + e + f + mixed1;
            break;
        case 2:
            result = g + h + i + (int)c1;
            break;
        case 3:
            result = j + k + l + addr_sum;
            break;
        case 4:
            result = m + n + o + (*ptr1);
            break;
        case 5:
            result = p + q + r + (*ptr2);
            break;
        case 6:
            result = s + t + u + (*ptr3);
            break;
        case 7:
            result = v + w + z + mixed2;
            break;
    }
    
    /* Inline assembly to create complex dataflow */
    int asm_out;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "imull %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (asm_out)
        : "r" (result), "r" (mixed1), "r" (mixed2)
        : "%eax"
    );
    
    /* More arithmetic to extend live ranges */
    int final1 = asm_out * 2;
    int final2 = final1 / 3;
    int final3 = final2 ^ 0xABCD;
    int final4 = final3 + v3;
    
    return final4;
}

/* Second function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values within loop */
        int t1 = i * 3;
        int t2 = t1 + 5;
        int t3 = t2 ^ 0x1234;
        int t4 = t3 * 7;
        int t5 = t4 - i;
        int t6 = t5 & 0xFF;
        int t7 = t6 | 0x80;
        int t8 = t7 * 11;
        int t9 = t8 + 13;
        int t10 = t9 ^ 0x5678;
        
        /* Conditional that uses volatile to prevent optimization */
        if (__builtin_expect(v1 > 100, 1)) {
            t10 += v2;
        }
        
        /* Mixed-type operations */
        short st1 = (short)t10;
        char ct1 = (char)t9;
        int mixed = (int)st1 * (int)ct1;
        
        /* Address calculation reused - potential rematerialization */
        int *addr = &global_array[i & 0xFF];
        *addr = mixed;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + mixed;
        
        /* Nested loop for additional pressure */
        for (int j = 0; j < 3; j++) {
            int nt1 = sum * j;
            int nt2 = nt1 + i;
            int nt3 = nt2 ^ j;
            sum += nt3;
        }
    }
    
    return sum;
}

/* Main function with maximum register pressure */
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Create many input values */
    int inputs[20];
    for (int i = 0; i < 20; i++) {
        inputs[i] = rand() % 1000;
    }
    
    /* Chain computations to increase pressure */
    int result1 = compute_heavy(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4],
                               inputs[5], inputs[6], inputs[7], inputs[8], inputs[9]);
    
    int result2 = compute_heavy(inputs[10], inputs[11], inputs[12], inputs[13], inputs[14],
                               inputs[15], inputs[16], inputs[17], inputs[18], inputs[19]);
    
    int result3 = loop_pressure(100);
    
    /* Final computation using all results */
    int final_result = result1 + result2 + result3;
    
    /* Use volatile to ensure computation isn't optimized away */
    v3 = final_result;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
