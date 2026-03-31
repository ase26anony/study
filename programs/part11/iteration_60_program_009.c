/* Test case for early rematerialization register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;

/* Global array for address calculations */
int global_array[100];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2;
    int b = a * x3;
    int c = b - x4;
    int d = c ^ x5;
    int e = d | x6;
    int f = e & x7;
    int g = f + x8;
    int h = g - x9;
    int i = h * x10;
    int j = i ^ a;
    int k = j | b;
    int l = k & c;
    int m = l + d;
    int n = m - e;
    int o = n * f;
    int p = o ^ g;
    int q = p | h;
    int r = q & i;
    int s = r + j;
    int t = s - k;
    int u = t * l;
    int v = u ^ m;
    int w = v | n;
    int x = w & o;
    int y = x + p;
    int z = y - q;
    
    /* Mixed-type operations to trigger mode changes */
    short sa = (short)a;
    short sb = (short)b;
    short sc = (short)c;
    short sd = (short)d;
    
    /* Use volatile to create dataflow barriers */
    if (__builtin_expect(v1 > 0, 1)) {
        sa = (short)(sa + vs1);
        sb = (short)(sb + vs2);
    }
    
    /* Complex switch with different live value subsets */
    int selector = (z & 0x7); /* 8 cases */
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + (int)sa + r;
            break;
        case 1:
            result = b + (int)sb + s;
            /* Mode conversion */
            result = (short)result + t;
            break;
        case 2:
            result = c + (int)sc + u;
            /* Another mode conversion */
            result = (int)((short)result) + v;
            break;
        case 3:
            result = d + (int)sd + w;
            break;
        case 4:
            result = e + (int)sa + x;
            /* Force address calculation rematerialization */
            int *ptr1 = &global_array[result % 100];
            int *ptr2 = &global_array[(result + 1) % 100];
            result = *ptr1 + *ptr2 + y;
            break;
        case 5:
            result = f + (int)sb + z;
            /* More address calculations */
            for (int idx = 0; idx < 5; idx++) {
                result += global_array[(result + idx) % 100];
            }
            break;
        case 6:
            result = g + (int)sc + a + b;
            /* Mixed width operations */
            result = (result & 0xFFFF) + (short)result;
            break;
        case 7:
            result = h + (int)sd + c + d;
            /* Use inline asm for complex dataflow */
            asm volatile ("# Dummy asm" : "+r" (result) : "r" (v2));
            break;
    }
    
    /* Additional computations to keep values live */
    if (__builtin_expect(v3 > 0, 0)) {
        result += i + j + k;
    } else {
        result += l + m + n;
    }
    
    /* More mixed-type operations */
    long la = (long)result;
    long lb = (long)(o + p);
    long lc = la * lb;
    
    /* Bitfield-like operations */
    struct packed {
        unsigned int a : 5;
        unsigned int b : 7;
        unsigned int c : 10;
        unsigned int d : 10;
    } pf;
    
    pf.a = result & 0x1F;
    pf.b = (result >> 5) & 0x7F;
    pf.c = (result >> 12) & 0x3FF;
    pf.d = (result >> 22) & 0x3FF;
    
    int from_packed = pf.a + pf.b + pf.c + pf.d;
    
    /* Final complex expression with many live values */
    return result + (int)(lc >> 32) + from_packed + 
           q + r + s + t + u + v + w + x + y + z;
}

/* Another function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations) {
    int sum = 0;
    volatile int barrier = v4;
    
    for (int i = 0; i < iterations; i++) {
        /* Many intermediate values in loop */
        int t1 = i * 3;
        int t2 = t1 + barrier;
        int t3 = t2 ^ 0xABCD;
        int t4 = t3 | 0x1234;
        int t5 = t4 & 0xF0F0;
        int t6 = t5 - i;
        int t7 = t6 * 7;
        int t8 = t7 + barrier;
        int t9 = t8 ^ t1;
        int t10 = t9 | t2;
        int t11 = t10 & t3;
        int t12 = t11 + t4;
        int t13 = t12 - t5;
        int t14 = t13 * t6;
        int t15 = t14 ^ t7;
        int t16 = t15 | t8;
        int t17 = t16 & t9;
        int t18 = t17 + t10;
        int t19 = t18 - t11;
        int t20 = t19 * t12;
        
        /* Mode conversions within loop */
        short st1 = (short)t1;
        short st2 = (short)t2;
        short st3 = (short)t3;
        int it1 = (int)st1;
        int it2 = (int)st2;
        int it3 = (int)st3;
        
        /* Address calculation that could be rematerialized */
        int idx = (t20 + i) % 100;
        int *addr = &global_array[idx];
        
        /* Use in expression */
        sum += t20 + it1 + it2 + it3 + *addr;
        
        /* Conditional that merges many live values */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            sum += t13 + t14 + t15 + t16 + t17;
        }
    }
    
    return sum;
}

/* Main function with all the pressure */
int main(int argc, char **argv) {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Get inputs from command line or use defaults */
    int input1 = argc > 1 ? atoi(argv[1]) : 100;
    int input2 = argc > 2 ? atoi(argv[2]) : 200;
    int input3 = argc > 3 ? atoi(argv[3]) : 300;
    
    /* Chain computations to create complex dataflow */
    int result1 = compute_heavy(
        input1, input2, input3, v1, v2,
        input1 ^ input2, input2 ^ input3, input3 ^ input1,
        v3, v4
    );
    
    int result2 = loop_pressure(50);
    
    /* Use results to prevent elimination */
    int final_result = result1 + result2 + v5;
    
    /* Print to ensure side effect */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
