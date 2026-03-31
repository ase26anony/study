/* test-caller-save.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_sink = x; }
NOINLINE void func3(void) { /* Empty but not pure */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many independent values that must stay in registers */
    int a = x * 3 + 1;
    int b = y / 2 - 5;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = x + y + z;
    int f = x * y - z;
    int g = (x << 3) | (y >> 2);
    int h = ~(x & y & z);
    
    /* All these values are live across the call */
    /* Call is at the end of basic block before return */
    if (x > 0) {
        /* Force register pressure - use all values in computation */
        int temp1 = a + b;
        int temp2 = c - d;
        int temp3 = e ^ f;
        int temp4 = g | h;
        
        /* Non-inline call with many live registers */
        func1();
        
        /* Return uses all live values - forces them to be preserved */
        return temp1 + temp2 + temp3 + temp4 + a + b + c + d + e + f + g + h;
    } else {
        /* Different path to create control flow */
        func2(x);
        return x + y + z;
    }
}

/* Test 2: Call in switch case with register pressure */
NOINLINE int test_call_in_switch_case(int selector, int v1, int v2, int v3) {
    int result = 0;
    
    /* Create many live values */
    int live1 = v1 * 2 + 7;
    int live2 = v2 / 3 - 11;
    int live3 = v3 & 0x7F;
    int live4 = v1 ^ v2 ^ v3;
    int live5 = (v1 << 2) | (v2 >> 1);
    int live6 = ~(v1 | v2 | v3);
    int live7 = v1 * v2 - v3;
    int live8 = v1 + v2 * 3 + v3 * 5;
    
    switch (selector & 3) {
        case 0:
            /* Basic block ends with call, then break */
            func3();
            /* All live values used after call */
            result = live1 + live2 + live3;
            break;
            
        case 1:
            /* More computations then call at end */
            result = live4 - live5;
            func1();
            /* Note: result computation before call, 
               but live1-live8 still live for break statement */
            break;
            
        case 2:
            /* Call with many live values, used after */
            func2(live6);
            result = live7 * live8 + live1;
            break;
            
        default:
            /* Complex computation forcing spills */
            result = live1 * live2 + live3 * live4 - live5 * live6 + live7 - live8;
            func3();
            /* Call at end of basic block */
            break;
    }
    
    /* Use all live values to force preservation */
    return result + live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
}

/* Test 3: Call between complex operations with loop-generated values */
NOINLINE int test_call_between_complex_ops(int base, int iterations) {
    /* Unrolled computations to create many live values */
    int val1 = base + 1;
    int val2 = base * 2;
    int val3 = base & 0xFFFF;
    int val4 = base ^ 0xAAAA;
    int val5 = base << 1;
    int val6 = base >> 2;
    int val7 = ~base;
    int val8 = base * 3 + 5;
    
    /* Additional values from simple loop */
    for (int i = 0; i < iterations && i < 4; i++) {
        /* Each iteration creates more live values */
        val1 += i * 2;
        val2 ^= i * 3;
        val3 |= i << 4;
        val4 &= ~i;
    }
    
    /* All values val1-val8 are live across this call */
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = func3;
    fp();
    
    /* Complex use of all live values */
    int sum = 0;
    sum += val1 * val2;
    sum -= val3 | val4;
    sum += val5 & val6;
    sum ^= val7 + val8;
    
    /* More computations to increase register pressure after call */
    int extra1 = sum * 3;
    int extra2 = sum / 2;
    int extra3 = sum & 0xFF;
    int extra4 = sum ^ 0x55;
    
    /* Another call with new live values */
    func1();
    
    /* Final computation using everything */
    return sum + extra1 - extra2 + extra3 ^ extra4 + val1 + val2;
}

/* Test 4: Nested calls with overlapping live ranges */
NOINLINE int test_nested_live_ranges(int a, int b, int c) {
    /* First set of live values */
    int x1 = a + b;
    int x2 = b * c;
    int x3 = a ^ c;
    int x4 = (a << 3) + (b >> 1);
    
    /* First call - x1-x4 must be preserved */
    int mid = func4(x1, x2);
    
    /* New values that become live */
    int y1 = mid + x3;
    int y2 = mid * x4;
    int y3 = x1 ^ x2 ^ x3 ^ x4;
    int y4 = (mid << 2) | (x1 >> 2);
    
    /* Second call with both old and new values live */
    /* This creates complex save/restore decisions */
    func2(y1);
    
    /* Use all values */
    return x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4 + mid;
}

/* Test 5: Call as last instruction before return in multiple paths */
NOINLINE int test_multiple_bb_ends(int cond1, int cond2, int x) {
    int result = 0;
    
    /* Create many live values */
    int l1 = x * 2;
    int l2 = x + 100;
    int l3 = x & 0xFF;
    int l4 = x ^ 0x1234;
    int l5 = ~x;
    int l6 = x << 1;
    int l7 = x >> 2;
    int l8 = x * x;
    
    if (cond1) {
        if (cond2) {
            /* Path 1: Call at end of basic block */
            func3();
            /* Return uses all live values */
            return l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
        } else {
            /* Path 2: Different computation, then call at end */
            result = l1 * l2 - l3;
            func1();
            return result + l4 + l5;
        }
    } else {
        /* Path 3: More computations, call, then return */
        int temp = l6 | l7;
        func2(temp);
        /* Call is last instruction before return in this block */
        return l8 + temp + l1 + l2;
    }
}

int main(void) {
    int total = 0;
    
    /* Seed for reproducible but varied inputs */
    int seed = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(seed + i, seed - i, seed * i);
        total += test_call_in_switch_case(i, seed, seed * 2, seed / 2);
        total += test_call_between_complex_ops(seed + i * 10, i + 1);
        total += test_nested_live_ranges(seed - i, seed + i, i);
        total += test_multiple_bb_ends(i & 1, i & 2, seed + i * 5);
        
        /* Modify seed to vary inputs */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
