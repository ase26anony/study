/* test-caller-save.c - Test program to trigger caller-save insertion logic */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_counter += x; }
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a + b; }

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values across the call */
    int a = x * 2;
    int b = y + 3;
    int c = z & 0xFF;
    int d = x ^ y;
    int e = y | z;
    int f = z << 2;
    int g = x >> 1;
    int h = y * y;
    int i = z + x;
    int j = x - y;
    
    /* Force all values to be computed and kept in registers */
    a = a + 1;
    b = b * 2;
    c = c | 0x80;
    d = d ^ 0x55;
    e = e & 0xAA;
    f = f + 10;
    g = g - 5;
    h = h % 100;
    i = i * 3;
    j = j / 2;
    
    /* Non-inline call with many live values */
    func1();
    
    /* Use all live values after call - forces them to be preserved */
    int result = a + b + c + d + e + f + g + h + i + j;
    
    /* This return creates a basic block ending with the call */
    return result;
}

/* Test 2: Call in switch case that ends with break */
NOINLINE int test_call_in_switch_case(int x, int y, int mode) {
    int result = 0;
    
    switch (mode & 3) {
        case 0: {
            /* Create live values in this case */
            int a = x + 1;
            int b = y * 2;
            int c = x ^ y;
            int d = y - x;
            int e = x | y;
            
            /* Force computation */
            a = a * 3;
            b = b + 5;
            c = c & 0xFF;
            d = d << 1;
            e = e >> 2;
            
            /* Call at what could be BB end before break */
            func2(x);
            
            /* Use values after call */
            result = a + b + c + d + e;
            break;  /* Creates BB boundary after call */
        }
        case 1: {
            int a = x * x;
            int b = y * y;
            func3();
            result = a - b;
            break;
        }
        case 2: {
            int a = x + y;
            int b = x - y;
            func1();
            result = a * b;
            break;
        }
        default: {
            result = x + y;
            break;
        }
    }
    
    return result;
}

/* Test 3: Complex loop creating register pressure, then call */
NOINLINE int test_call_between_complex_ops(int iterations) {
    int vals[10];
    int sum = 0;
    
    /* Compute many values in a loop - creates register pressure */
    for (int i = 0; i < 10 && i < iterations; i++) {
        vals[i] = i * i + iterations;
        /* Force each computation to be different */
        vals[i] = vals[i] ^ (i * 0x1234);
        vals[i] = vals[i] & 0xFFFF;
        vals[i] = vals[i] + i;
    }
    
    /* Non-inline call with all vals[] potentially live */
    func3();
    
    /* Use all computed values after call */
    for (int i = 0; i < 10 && i < iterations; i++) {
        sum += vals[i];
        sum = sum ^ vals[i];  /* Complex use to prevent optimization */
    }
    
    return sum;
}

/* Test 4: Multiple calls in different branches */
NOINLINE int test_multiple_calls_in_branches(int x, int y, int cond) {
    int result = 0;
    
    if (cond > 0) {
        /* Branch 1: many live values, call at end before return-like structure */
        int a = x * 3;
        int b = y * 4;
        int c = x + y;
        int d = x - y;
        int e = x ^ y;
        int f = x | y;
        int g = x & y;
        
        /* Complex computations */
        a = (a << 1) | 1;
        b = (b >> 1) & 0x7F;
        c = c * c;
        d = d + 100;
        e = e ^ 0xAA;
        f = f | 0x55;
        g = g * 2;
        
        /* Call that could be at BB end */
        int ret = func4(a, b);
        
        /* Force all values to be used */
        result = ret + c + d + e + f + g;
        
        /* Early return creates BB ending with call */
        return result;
    } else if (cond < 0) {
        /* Branch 2: different pattern */
        int a = y * x;
        int b = x + 100;
        int c = y - 50;
        
        func2(a);
        
        result = a + b + c;
    } else {
        /* Branch 3: yet another pattern */
        int a = x * 5;
        int b = y * 6;
        
        func1();
        
        result = a - b;
    }
    
    return result;
}

/* Test 5: Call through volatile function pointer */
NOINLINE int test_volatile_call(int x, int y) {
    /* Create many live values */
    int vals[8];
    for (int i = 0; i < 8; i++) {
        vals[i] = x + i * y;
        vals[i] = vals[i] * vals[i];
        vals[i] = vals[i] & 0xFFF;
    }
    
    /* Volatile function pointer ensures call isn't optimized away */
    void (*volatile fp)(void) = func3;
    fp();  /* This call must be preserved */
    
    /* Use all values */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += vals[i];
        sum = sum ^ (vals[i] << (i & 3));
    }
    
    return sum;
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line args or defaults for variability */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += test_call_at_bb_end(base + i, base - i, base * i);
        total += test_call_in_switch_case(base + i * 2, base - i * 3, i);
        total += test_call_between_complex_ops(base + i * 5);
        total += test_multiple_calls_in_branches(base + i, base - i, i - 1);
        total += test_volatile_call(base * i, base + i);
    }
    
    /* Store to volatile to prevent optimization */
    global_result = total;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
