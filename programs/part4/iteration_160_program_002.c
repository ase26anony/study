/* test-caller-save.c - Test program to trigger caller-save insertion at BB_END */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and optimization of helper functions */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink++; }
NOINLINE void func2(int x) { global_sink ^= x; }
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across the call */
    int a = x * 2 + 1;
    int b = y / 3 - 2;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = a + b;
    int f = c | d;
    int g = (x << 3) | (y >> 2);
    int h = (z * 7) % 13;
    
    /* All these values are live across this call */
    func1();
    
    /* Use all live values after the call */
    int result = a + b - c + d - e + f ^ g | h;
    
    /* This call is at the end of BB before return */
    if (result > 100) {
        int i = a * b;
        int j = c + d;
        int k = e ^ f;
        int l = g & h;
        func2(i + j);
        /* The call above is BB_END, and any inserted save/restore
           should become the new BB_END */
        return k + l + result;
    }
    
    return result;
}

/* Test 2: Call in a switch case that ends with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    switch (x % 4) {
        case 0: {
            /* Create register pressure */
            int a = x + y;
            int b = y - z;
            int c = x * z;
            int d = (x << 2) | (y >> 1);
            int e = a ^ b ^ c;
            
            func3();
            
            /* All values used after call */
            result = a + b + c + d + e;
            break;  /* Call is before break, at BB end */
        }
        case 1: {
            int f = x * 3;
            int g = y * 5;
            int h = z * 7;
            int i = f ^ g;
            int j = h & 0x7F;
            
            /* Call through volatile pointer to prevent optimization */
            void (*volatile fp)(void) = func3;
            fp();
            
            result = f - g + h - i + j;
            break;
        }
        case 2: {
            /* Even more live values */
            int k = x + 1;
            int l = y + 2;
            int m = z + 3;
            int n = k * l;
            int o = m ^ n;
            int p = (k << l) & 0xFF;
            int q = (m >> 2) | p;
            
            func1();
            
            result = k + l + m + n + o + p + q;
            break;
        }
        default:
            result = x + y + z;
    }
    
    return result;
}

/* Test 3: Complex loop creating register pressure before call */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int vals[8];
    int sum = 0;
    
    /* Compute many values in a loop - creates register pressure */
    for (int i = 0; i < 8; i++) {
        vals[i] = (seed * (i + 1)) ^ (seed >> (i % 4));
    }
    
    /* Make all vals[] live across the call */
    func1();
    
    /* Use all values after call */
    for (int i = 0; i < 8; i++) {
        sum += vals[i];
        
        /* More computations to increase register pressure */
        vals[i] = vals[i] * 3 - 1;
        sum ^= vals[i];
    }
    
    /* Another call at potential BB end */
    if (sum > 1000) {
        int a = vals[0] + vals[1];
        int b = vals[2] - vals[3];
        int c = vals[4] * vals[5];
        int d = vals[6] ^ vals[7];
        
        func2(a + b);
        /* This call is BB_END before return */
        return a + b + c + d;
    }
    
    return sum;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        int a = x * 2;
        int b = y + 3;
        int c = z - 4;
        int d = a ^ b;
        int e = c & 0x7F;
        
        if (y > 0) {
            int f = a + b;
            int g = c * d;
            int h = e | f;
            
            func3();
            /* Call at BB end before else branch */
            result = f + g + h;
        } else {
            int i = b - c;
            int j = d ^ e;
            int k = i * j;
            
            func1();
            /* Another call at BB end */
            result = i + j + k;
        }
        
        /* More live values */
        int l = result * 2;
        int m = l ^ a;
        
        func2(m);
        /* Call at BB end before return */
        return m + l;
    } else {
        return x + y + z;
    }
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    /* Run tests with different inputs to cover various paths */
    for (int i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i*2, i*3);
        total += test_call_in_switch_case(i, i+1, i+2);
        total += test_call_between_complex_ops(8, i);
        total += test_nested_conditionals(i, i-1, i+1);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
