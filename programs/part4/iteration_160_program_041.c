/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_DO(x) do { volatile int _v = 1; if (_v) { x; } } while(0)

/* Non-inline functions to force calls */
NOINLINE void func1(void) { asm volatile("" ::: "memory"); }
NOINLINE void func2(int x) { asm volatile("" ::: "memory"); }
NOINLINE void func3(void) { asm volatile("" ::: "memory"); }
NOINLINE void func4(void) { asm volatile("" ::: "memory"); }

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0x55AA55AA;
    int v4 = d | 0x12345678;
    int v5 = e + f * 7;
    int v6 = (a + b) * (c - d);
    int v7 = v1 ^ v2;
    int v8 = v3 | v4;
    
    /* Function call with many live registers */
    func1();
    
    /* Use all live values after call - forces them to be preserved */
    return v1 + v2 - v3 + v4 * v5 + v6 / 2 + v7 ^ v8;
}

/* Test 2: Call in switch case at end of basic block */
NOINLINE int test_call_in_switch_case(int x, int a, int b, int c, int d) {
    int result = 0;
    
    /* Create live values before switch */
    int v1 = a + x;
    int v2 = b * x;
    int v3 = c ^ x;
    int v4 = d << 2;
    int v5 = v1 + v2;
    int v6 = v3 | v4;
    
    switch (x & 3) {
        case 0:
            /* More computations to increase register pressure */
            v1 = v1 * 2 + 1;
            v2 = v2 ^ 0xFF;
            v3 = v3 | 0xAA;
            v4 = v4 + 555;
            func2(x);  /* Call at end of basic block before break */
            result = v1 + v2;
            break;
            
        case 1:
            v5 = v5 * 3;
            v6 = v6 ^ 0xCC;
            func3();
            result = v3 + v4 + v5;
            break;
            
        case 2:
            /* Even more live values */
            int v7 = v1 * v2;
            int v8 = v3 + v4;
            int v9 = v5 ^ v6;
            func4();
            result = v7 + v8 - v9;
            break;
            
        default:
            result = v1 + v2 + v3 + v4 + v5 + v6;
            break;
    }
    
    return result;
}

/* Test 3: Complex control flow with call at block end */
NOINLINE int test_complex_control_flow(int cond, int a, int b, int c, int d, int e) {
    /* Create many live values */
    int v1 = a + 1;
    int v2 = b * 2;
    int v3 = c ^ 0xDEADBEEF;
    int v4 = d | 0xCAFEBABE;
    int v5 = e << 3;
    int v6 = v1 + v2;
    int v7 = v3 ^ v4;
    int v8 = v5 * 7;
    
    if (cond > 0) {
        /* More computations in this branch */
        v1 = v1 * cond;
        v2 = v2 + cond;
        v3 = v3 ^ cond;
        func1();  /* Call at end of if-block before return */
        return v1 + v2 + v3 + v4;
    } else if (cond < 0) {
        v5 = v5 - cond;
        v6 = v6 * (-cond);
        v7 = v7 | 0x1234;
        func2(cond);
        return v5 + v6 + v7 + v8;
    } else {
        /* Even more computations in else block */
        int v9 = v1 * v2;
        int v10 = v3 + v4;
        int v11 = v5 ^ v6;
        int v12 = v7 | v8;
        func3();
        /* Call at end of basic block before return */
        return v9 + v10 + v11 + v12;
    }
}

/* Test 4: Loop with call at end of basic block */
NOINLINE int test_loop_with_call(int iterations, int a, int b, int c) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create values that must live across call */
        int v1 = a + i;
        int v2 = b * i;
        int v3 = c ^ i;
        int v4 = v1 * v2;
        int v5 = v2 + v3;
        int v6 = v3 ^ v4;
        
        /* Function call inside loop */
        func1();
        
        /* Use values after call */
        sum += v1 + v2 + v3 + v4 + v5 + v6;
        
        /* Modify inputs for next iteration */
        a = (a + 1) ^ 0x55;
        b = (b * 3) & 0xFF;
        c = (c - 1) | 0xAA;
    }
    
    return sum;
}

/* Test 5: Nested calls with many live values */
NOINLINE int test_nested_calls(int x) {
    /* Create a chain of computations */
    int v1 = x * 2;
    int v2 = x + 0x1234;
    int v3 = x ^ 0xABCD;
    int v4 = v1 + v2;
    int v5 = v2 * v3;
    int v6 = v3 ^ v4;
    int v7 = v4 | v5;
    int v8 = v5 + v6;
    
    /* First call */
    func1();
    
    /* More computations between calls */
    v1 = v1 + v8;
    v2 = v2 ^ v7;
    v3 = v3 * v6;
    v4 = v4 | v5;
    
    /* Second call - might be at end of basic block */
    if (x > 100) {
        func2(x);
        return v1 + v2 + v3 + v4;
    } else {
        func3();
        return v1 - v2 + v3 - v4;
    }
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line args or defaults to vary inputs */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(base + i, base - i, base * i, 
                                    base / (i + 1), base ^ i, base | i);
    }
    
    /* Run test 2 */
    for (int i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, base + 1, base + 2, 
                                         base + 3, base + 4);
    }
    
    /* Run test 3 with various conditions */
    for (int i = -5; i <= 5; i++) {
        total += test_complex_control_flow(i, base, base * 2, 
                                          base / 2, base ^ 0xFF, base | 0xAA);
    }
    
    /* Run test 4 */
    total += test_loop_with_call(5, base, base + 10, base + 20);
    
    /* Run test 5 */
    for (int i = 0; i < 20; i++) {
        total += test_nested_calls(base + i * 10);
    }
    
    /* Store result to volatile to prevent optimization */
    global_sink = total;
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
