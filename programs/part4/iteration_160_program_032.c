/* caller-save-test.c - Test program to trigger caller-save insertion logic */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointer to prevent optimization */
typedef void (*volatile func_ptr_t)(void);

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Function call at the end of a basic block before return
   This should create a basic block where the call is BB_END */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0x55AA55AA;
    int v4 = d + e * f;
    int v5 = (a & b) | (c & d);
    int v6 = e * 7 - f;
    int v7 = (a + b) * (c - d);
    int v8 = (e << 3) | (f & 0xFF);
    
    /* Use volatile function pointer to ensure call isn't optimized */
    func_ptr_t fp = func1;
    fp();
    
    /* All values used after call - must be saved/restored */
    return v1 + v2 - v3 + v4 * v5 + v6 / (v7 + 1) + v8;
}

/* Test 2: Function call in a switch case that ends with break
   Creates basic blocks ending with calls */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    switch (x % 4) {
        case 0: {
            /* Many live values in this case */
            int a = y * 2;
            int b = z + 1;
            int c = y ^ z;
            int d = (y << 4) | (z & 0xF);
            int e = a * b - c;
            int f = d + 5;
            
            func2();  /* Call at end of basic block before break */
            
            result = a + b + c + d + e + f;
            break;
        }
        case 1: {
            int a = y * 3;
            int b = z * 5;
            int c = a ^ b;
            
            /* Another call at basic block end */
            func3();
            
            result = a - b + c;
            break;
        }
        default: {
            int a = y + z;
            int b = y - z;
            
            func4();
            
            result = a * b;
            break;
        }
    }
    
    return result;
}

/* Test 3: Complex loop creating register pressure, then a call,
   then use of all values */
NOINLINE int test_call_between_complex_ops(int n) {
    /* Create many accumulator variables in a loop */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Loop creates many live values */
    for (int i = 0; i < n; i++) {
        acc1 += i * 1;
        acc2 += i * 3;
        acc3 += i * 5;
        acc4 += i * 7;
        acc5 += i * 11;
        acc6 += i * 13;
        acc7 += i * 17;
        acc8 += i * 19;
    }
    
    /* Call with all accumulators live */
    func_ptr_t fp = func1;
    fp();
    
    /* Use all accumulators after call */
    return acc1 + acc2 * 2 + acc3 / 3 + acc4 * acc5 + 
           acc6 - acc7 + acc8 * 2;
}

/* Test 4: Nested conditionals creating multiple basic blocks
   ending with calls */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int result = 0;
    
    if (a > 0) {
        int x1 = b * 2;
        int x2 = c * 3;
        int x3 = x1 ^ x2;
        int x4 = (b << 2) + c;
        
        if (b > 0) {
            int y1 = x1 + x2;
            int y2 = x3 * x4;
            int y3 = y1 ^ y2;
            
            func2();  /* Call in nested block */
            
            result = y1 + y2 + y3;
        } else {
            int y1 = x1 - x2;
            int y2 = x3 | x4;
            
            func3();  /* Call at end of else block */
            
            result = y1 * y2;
        }
    } else {
        int x1 = b + c;
        int x2 = b - c;
        int x3 = x1 * x2;
        int x4 = x1 ^ x2;
        int x5 = x3 << 1;
        
        func4();  /* Call at end of else block */
        
        result = x3 + x4 + x5;
    }
    
    return result;
}

/* Test 5: Multiple consecutive calls with live values between them */
NOINLINE int test_multiple_calls(int a, int b, int c, int d) {
    /* Values live across first call */
    int v1 = a * 2 + 1;
    int v2 = b << 1;
    int v3 = c ^ d;
    
    func1();
    
    /* More computation creating new live values */
    int v4 = v1 + v2;
    int v5 = v3 * 3;
    int v6 = (v4 & v5) | (a & b);
    
    func2();
    
    /* Even more live values */
    int v7 = v4 * v5;
    int v8 = v6 + v1;
    int v9 = v2 - v3;
    
    func3();
    
    /* Use all values */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

int main(void) {
    int total = 0;
    
    /* Seed for pseudo-random but deterministic behavior */
    int seed = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(seed + i, i * 2, i * 3, i * 4, i * 5, i * 6);
        total += test_call_in_switch_case(seed + i, i * 7, i * 11);
        total += test_call_between_complex_ops(5 + (i % 3));
        total += test_nested_conditionals(seed - i, i * 13, i * 17);
        total += test_multiple_calls(i, i * 2, i * 3, i * 5);
        
        /* Modify seed slightly each iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
