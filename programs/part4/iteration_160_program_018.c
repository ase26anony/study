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
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at basic block end before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x * 3;
    int b = y + 7;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = (x << 3) | (y >> 2);
    int f = z * z - x;
    int g = y + z * 2;
    int h = x * y + z;
    
    /* Force all values to be live across this call */
    func1();
    
    /* Use all values after call - ensures they must be saved/restored */
    int result = a + b - c + d - e + f - g + h;
    
    /* Call is at end of basic block before return */
    if (result > 100) {
        /* More computations to increase register pressure */
        int i = a * b;
        int j = c * d;
        int k = e * f;
        func2(i + j + k);
        /* This call is at BB end before return */
        return result + i - j + k;
    }
    
    return result;
}

/* Test 2: Call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y, int mode) {
    int result = 0;
    
    switch (mode & 3) {
        case 0: {
            /* Many live values in this case */
            int a = x + 1;
            int b = y * 2;
            int c = x ^ y;
            int d = (x << 4) | (y & 0xF);
            int e = y - x;
            int f = x * y;
            
            /* Call with all values live */
            func3();
            
            /* Use values after call */
            result = a + b + c + d + e + f;
            break;  /* Call is at BB end before break */
        }
        case 1: {
            int a = x * x;
            int b = y * y;
            int c = a + b;
            func1();
            result = c;
            break;
        }
        case 2: {
            /* Even more live values */
            int v1 = x + y;
            int v2 = x - y;
            int v3 = x * 3;
            int v4 = y * 5;
            int v5 = v1 ^ v2;
            int v6 = v3 & v4;
            int v7 = v5 | v6;
            
            func2(v7);
            
            result = v1 + v2 + v3 + v4 + v5 + v6 + v7;
            break;
        }
        default:
            result = x + y;
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int values[8];
    int sum = 0;
    
    /* Compute many values in a way that prevents optimization */
    for (int i = 0; i < 8; i++) {
        values[i] = seed * (i + 1);
        values[i] ^= (values[i] << 3);
        values[i] += i * 7;
    }
    
    /* Additional computations to increase register pressure */
    int a = values[0] * values[1];
    int b = values[2] | values[3];
    int c = values[4] & values[5];
    int d = values[6] ^ values[7];
    int e = a + b;
    int f = c - d;
    int g = e * f;
    int h = (a << 2) | (b >> 1);
    
    /* Critical call with many live values */
    int temp = func4(g, h);
    
    /* Use all values after call */
    for (int i = 0; i < 8; i++) {
        sum += values[i];
    }
    
    sum = sum + a - b + c - d + e - f + g - h + temp;
    
    /* Another call at potential BB end */
    if (sum > 1000) {
        int x = a * 2;
        int y = b / 3;
        func2(x + y);
        return sum + x - y;
    }
    
    return sum;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        int a = x + 10;
        int b = y * 2;
        int c = z & 0xFF;
        
        if (y > 0) {
            int d = a * b;
            int e = c ^ 0xAA;
            int f = d + e;
            
            /* Call at BB end in nested if */
            func3();
            
            result = f;
            return result;  /* Return immediately after call */
        } else {
            int d = a + b;
            int e = c | 0x55;
            
            func1();
            
            result = d - e;
        }
    } else {
        int a = y + z;
        int b = x * 3;
        
        func2(a + b);
        
        result = a - b;
    }
    
    /* More computations with live values */
    int extra1 = result * 2;
    int extra2 = result + 5;
    int extra3 = extra1 ^ extra2;
    
    func4(extra1, extra2);
    
    return result + extra3;
}

/* Test 5: Call through volatile function pointer */
NOINLINE int test_volatile_call(int x, int y) {
    /* Volatile function pointer ensures call isn't optimized away */
    void (*volatile fp)(int) = (void (*)(int))func2;
    
    /* Many live values */
    int a = x * 3 + 1;
    int b = y * 7 - 2;
    int c = (x << 4) | (y & 0xF);
    int d = x ^ y ^ 0xCC;
    int e = a + b;
    int f = c * d;
    int g = e ^ f;
    
    /* Call through volatile pointer */
    fp(g);
    
    /* Use all values */
    int result = a + b - c + d - e + f - g;
    
    /* Another call at potential BB end */
    if (result > 50) {
        int h = result * 2;
        fp(h);
        return h;
    }
    
    return result;
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use arguments to make values dynamic but reproducible */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += test_call_at_bb_end(base + i, base - i, base * i);
        total += test_call_in_switch_case(base * 2 + i, base / 2 + i, i);
        total += test_call_between_complex_ops(8, base + i * 10);
        total += test_nested_conditionals(base - i, base + i, base * i);
        total += test_volatile_call(base * 3 + i, base * 4 - i);
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
