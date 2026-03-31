/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions to force actual calls */
__attribute__((noinline, noipa)) void func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void func2(int x) {
    global_sink += x;
}

__attribute__((noinline, noipa)) void func3(void) {
    /* Clobber many registers */
    asm volatile("" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Test 1: Call at end of basic block before return */
__attribute__((noinline, noipa)) 
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0xABCD;
    int v4 = d + e * 2;
    int v5 = f - a;
    int v6 = (a + b) * (c - d);
    int v7 = e | f;
    int v8 = (a << 3) | (b >> 2);
    int v9 = c * d - e;
    int v10 = f + 12345;
    
    /* Use volatile function pointer to prevent inlining */
    void (*volatile fp)(void) = func1;
    
    /* Complex condition to create basic block structure */
    if (a > b) {
        /* All these values are live across the call */
        int t1 = v1 + v2;
        int t2 = v3 - v4;
        int t3 = v5 * v6;
        int t4 = v7 & v8;
        int t5 = v9 ^ v10;
        
        /* Call at the end of basic block, just before return */
        fp();
        
        /* This return makes the call the BB_END before insertion */
        return t1 + t2 + t3 + t4 + t5;
    } else {
        /* Alternative path with different live values */
        int t1 = v2 + v3;
        int t2 = v4 - v5;
        int t3 = v6 * v7;
        int t4 = v8 & v9;
        int t5 = v10 ^ v1;
        
        /* Another call at BB end */
        void (*volatile fp2)(void) = func1;
        fp2();
        
        return t1 * t2 + t3 - t4 + t5;
    }
}

/* Test 2: Call in switch case with break */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int live1 = x * 17 + 1;
    int live2 = y ^ 0xDEADBEEF;
    int live3 = z << 3;
    int live4 = x + y * 2;
    int live5 = z - x;
    int live6 = (x ^ y) & z;
    int live7 = y | (z << 1);
    int live8 = x * y - z;
    
    switch (x & 0x3) {
        case 0: {
            /* Multiple values live across call */
            int t1 = live1 + live2;
            int t2 = live3 - live4;
            int t3 = live5 * live6;
            
            /* Call that clobbers registers */
            func3();
            
            /* Use all live values after call */
            result = t1 + t2 + t3 + live7 + live8;
            break;  /* Creates BB ending with call */
        }
        case 1: {
            int t1 = live2 + live3;
            int t2 = live4 - live5;
            
            /* Volatile pointer call */
            void (*volatile fp)(void) = func1;
            fp();
            
            result = t1 * t2 + live6 - live7;
            break;
        }
        case 2: {
            /* Even more live values */
            int t1 = live1 * live2;
            int t2 = live3 ^ live4;
            int t3 = live5 & live6;
            int t4 = live7 | live8;
            
            func2(t1);
            
            result = t1 + t2 + t3 + t4;
            break;
        }
        default:
            result = live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
            break;
    }
    
    return result;
}

/* Test 3: Complex loop with call in middle */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int iterations, int seed) {
    int acc = seed;
    
    /* Unrolled loop creates many temporary values */
    for (int i = 0; i < iterations; i++) {
        /* Create many independent values that must be kept live */
        int v1 = acc * 3 + i;
        int v2 = acc ^ (i << 2);
        int v3 = (acc + i) & 0xFF;
        int v4 = acc - i * 7;
        int v5 = (acc << 1) | (i & 0xF);
        int v6 = i * i - acc;
        int v7 = acc + (i ^ 0xAA);
        int v8 = (acc * i) >> 2;
        int v9 = i + 777;
        int v10 = acc ^ i ^ 0x1234;
        
        /* Function call with many live values across it */
        if (i & 1) {
            void (*volatile fp)(int) = func2;
            fp(v1);
        } else {
            func3();
        }
        
        /* Use all values after call - forces them to be live across call */
        acc = v1 + v2 - v3 + v4 * v5 + v6 - v7 + v8 ^ v9 + v10;
        
        /* More computations to increase register pressure */
        int t1 = v1 * v2;
        int t2 = v3 + v4;
        int t3 = v5 ^ v6;
        int t4 = v7 - v8;
        int t5 = v9 & v10;
        
        /* Another potential call site */
        if ((i & 3) == 0) {
            func1();
        }
        
        acc += t1 + t2 + t3 + t4 + t5;
    }
    
    return acc;
}

/* Test 4: Nested conditionals with calls at block ends */
__attribute__((noinline, noipa))
int test_nested_conditions(int a, int b, int c) {
    /* Many live values */
    int x1 = a + b;
    int x2 = b * c;
    int x3 = a ^ c;
    int x4 = (a << 2) | (b >> 1);
    int x5 = c - a;
    int x6 = (a + c) * b;
    int x7 = b ^ 0x55;
    int x8 = c * 3 - a;
    
    int result = 0;
    
    if (a > 0) {
        if (b > 0) {
            int t1 = x1 + x2;
            int t2 = x3 - x4;
            
            /* Call at end of inner block */
            func1();
            
            result = t1 + t2;  /* Return would make call BB_END */
            
            /* But we continue, so BB ends with call */
            if (c > 0) {
                int t3 = x5 * x6;
                int t4 = x7 ^ x8;
                
                /* Another call at BB end */
                void (*volatile fp)(void) = func1;
                fp();
                
                result += t3 + t4;
            } else {
                result += x5 + x6;
            }
        } else {
            result = x3 + x4 + x5;
            func2(result);
        }
    } else {
        result = x6 + x7 + x8;
    }
    
    /* Force use of all values at the end */
    global_sink += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Varying inputs to create different execution paths */
    total += test_call_at_bb_end(1, 2, 3, 4, 5, 6);
    total += test_call_at_bb_end(10, 20, 30, 40, 50, 60);
    
    total += test_call_in_switch_case(100, 200, 300);
    total += test_call_in_switch_case(5, 15, 25);
    
    total += test_call_between_complex_ops(3, 42);
    total += test_call_between_complex_ops(5, 123);
    
    total += test_nested_conditions(7, 8, 9);
    total += test_nested_conditions(-1, 5, 10);
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
