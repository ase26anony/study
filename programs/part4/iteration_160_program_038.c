/* test-caller-save.c - Program to trigger caller-save optimization edge cases */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of calls */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * 3 + 1;
    int live2 = b ^ 0xABCD;
    int live3 = c & 0xFF00FF;
    int live4 = d | 0x12345678;
    int live5 = e << 3;
    int live6 = f >> 2;
    int live7 = a + b + c;
    int live8 = d - e - f;
    
    /* Additional computations to increase register pressure */
    int tmp1 = live1 * live2;
    int tmp2 = live3 ^ live4;
    int tmp3 = live5 | live6;
    int tmp4 = live7 & live8;
    
    /* Call at the end of basic block before return */
    if (a > 0) {
        /* All these values must be saved across the call */
        func1();
        /* This return makes the call the BB_END before insertion */
        return tmp1 + tmp2 + tmp3 + tmp4 + live1 + live2 + live3 + live4;
    } else {
        /* Alternative path to ensure both branches are compiled */
        int alt1 = live1 * 2;
        int alt2 = live2 / 3;
        func2();
        return alt1 + alt2;
    }
}

/* Test 2: Call in switch case with complex live ranges */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many independent computations */
    int val1 = x * x;
    int val2 = y * y;
    int val3 = z * z;
    int val4 = x + y + z;
    int val5 = x ^ y ^ z;
    int val6 = (x << 4) | (y << 2) | z;
    int val7 = ~(x & y & z);
    int val8 = (x > y) ? x - y : y - x;
    
    switch (x % 4) {
        case 0:
            /* Case ends with call then break - creates BB ending with call */
            result = val1 + val2;
            func1();
            break;
            
        case 1:
            /* Different call, different live set */
            result = val3 + val4;
            func2();
            break;
            
        case 2:
            /* More complex computation before call */
            result = val5 * val6;
            {
                /* Nested scope to potentially create additional BB structure */
                int nested = val7 ^ val8;
                result += nested;
                func3();
            }
            break;
            
        default:
            /* Default case with yet another pattern */
            result = val1 + val3 + val5 + val7;
            func4();
            break;
    }
    
    /* Use all values after switch to extend liveness */
    return result + val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
}

/* Test 3: Call between complex operations in loop */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Unrolled loop to create many live values */
    for (int i = 0; i < n && i < 8; i++) {
        /* Independent computations creating register pressure */
        int t1 = seed + i * 3;
        int t2 = seed ^ (i << 8);
        int t3 = seed * (i + 1);
        int t4 = ~seed + i;
        int t5 = seed & (0xFF << i);
        int t6 = seed | (i * 0x1111);
        int t7 = seed - i * 7;
        int t8 = (seed << i) | (seed >> (32 - i));
        
        /* Make a call with all these values live */
        if (i % 2 == 0) {
            VOLATILE_CALL(func1);
        } else {
            VOLATILE_CALL(func2);
        }
        
        /* Use values after call to keep them live */
        acc1 += t1;
        acc2 += t2;
        acc3 += t3;
        acc4 += t4;
        acc5 += t5;
        acc6 += t6;
        acc7 += t7;
        acc8 += t8;
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final computation using all accumulators */
    return acc1 ^ acc2 ^ acc3 ^ acc4 ^ acc5 ^ acc6 ^ acc7 ^ acc8;
}

/* Test 4: Nested calls with intervening computations */
NOINLINE int test_nested_live_ranges(int a, int b, int c) {
    /* Layer 1 computations */
    int l1a = a * 2 + 1;
    int l1b = b / 3 - 2;
    int l1c = c << 1;
    int l1d = a ^ b ^ c;
    
    /* First call */
    func1();
    
    /* Layer 2 computations - must save/restore around previous call */
    int l2a = l1a + l1b;
    int l2b = l1c * l1d;
    int l2c = l1a & l1b & l1c;
    int l2d = l1d | 0xFFFF;
    
    /* Second call */
    func2();
    
    /* Layer 3 computations */
    int l3a = l2a << 2;
    int l3b = l2b >> 1;
    int l3c = l2c ^ l2d;
    int l3d = ~l2a + ~l2b;
    
    /* Third call at potential BB end */
    if (l3a > l3b) {
        func3();
        return l3a + l3b + l3c + l3d;
    } else {
        func4();
        return l3a * l3b * l3c * l3d;
    }
}

/* Test 5: Multiple calls in same basic block with different live sets */
NOINLINE int test_multiple_calls_same_bb(int x) {
    /* First set of live values */
    int set1_a = x * 3;
    int set1_b = x + 0x1234;
    int set1_c = x ^ 0xABCD;
    
    /* First call */
    VOLATILE_CALL(func1);
    
    /* Second set - must preserve first set AND create new values */
    int set2_a = set1_a + set1_b;
    int set2_b = set1_c * 2;
    int set2_c = set1_a & set1_b;
    int set2_d = set1_c | set1_a;
    
    /* Second call */
    VOLATILE_CALL(func2);
    
    /* Third set - even more values */
    int set3_a = set2_a + set2_b;
    int set3_b = set2_c * set2_d;
    int set3_c = set2_a ^ set2_b ^ set2_c;
    int set3_d = ~set2_d;
    int set3_e = set2_a << 3;
    int set3_f = set2_b >> 2;
    
    /* Third call at end of BB */
    if (set3_a > 0) {
        VOLATILE_CALL(func3);
        /* Call is BB_END before save/restore insertion */
        return set3_a + set3_b + set3_c + set3_d + set3_e + set3_f;
    }
    
    return 0;
}

int main(void) {
    int total = 0;
    
    /* Initialize with "random" but deterministic values */
    int base = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int val = base + i * 17;
        
        total += test_call_at_bb_end(val, val+1, val+2, val+3, val+4, val+5);
        total += test_call_in_switch_case(val, val*2, val/3);
        total += test_call_between_complex_ops(8, val);
        total += test_nested_live_ranges(val, val^0x55, val*3);
        total += test_multiple_calls_same_bb(val);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile("" : "+r" (total));
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
