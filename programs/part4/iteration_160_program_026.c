/* caller-save-test.c - Test program to trigger specific uncovered lines in GCC's caller-save.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will be called */
__attribute__((noinline, noipa)) void func1(void) {
    /* Empty function to create call site */
}

__attribute__((noinline, noipa)) void func2(int x) {
    /* Use argument to prevent optimization */
    global_sink += x;
}

__attribute__((noinline, noipa)) void func3(void) {
    /* Another empty function */
}

/* Test 1: Function call at the end of a basic block before return */
__attribute__((noinline, noipa)) 
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must survive across the call */
    int live1 = a * b + c;
    int live2 = d ^ e | f;
    int live3 = g << 2;
    int live4 = h & 0xFF;
    int live5 = a + b + c + d;
    int live6 = e * f * g;
    int live7 = h * 3 + 1;
    int live8 = (a ^ b) & (c ^ d);
    int live9 = (e | f) ^ (g | h);
    int live10 = a * 2 + b * 3;
    
    /* Complex condition to create basic block structure */
    if (live1 > 100) {
        /* More computations to increase register pressure */
        int temp1 = live2 * live3;
        int temp2 = live4 ^ live5;
        int temp3 = live6 + live7;
        int temp4 = live8 | live9;
        
        /* Function call at what could be BB_END */
        func1();
        
        /* Use all live values after the call - forces them to be saved/restored */
        return live1 + live2 + live3 + live4 + live5 + 
               live6 + live7 + live8 + live9 + live10 +
               temp1 + temp2 + temp3 + temp4;
    } else {
        /* Alternative path with different computations */
        int temp5 = live1 * live10;
        int temp6 = live2 ^ live3;
        
        /* Another call site */
        func2(live4);
        
        return temp5 - temp6 + live5 + live6 + live7 + live8 + live9;
    }
}

/* Test 2: Function call in a switch case */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * y + z;
    int val2 = x ^ y ^ z;
    int val3 = (x << 3) | (y << 2) | (z << 1);
    int val4 = x * 17 + y * 13 + z * 7;
    int val5 = ~(x & y & z);
    int val6 = x + y * 2 + z * 3;
    int val7 = (x | y) & z;
    int val8 = x * x + y * y;
    
    switch (x & 0x3) {
        case 0:
            /* Use values before call */
            val1 = val1 * 2;
            val2 = val2 + 1;
            
            /* Function call that could be at BB_END before break */
            func1();
            
            /* The call might be BB_END before the break */
            result = val1 + val2;
            break;
            
        case 1:
            /* Different computations */
            val3 = val3 ^ val4;
            val5 = val5 | val6;
            
            /* Another call */
            func2(val7);
            
            result = val3 - val5 + val8;
            break;
            
        case 2:
            /* More register pressure */
            int temp1 = val1 * val2;
            int temp2 = val3 ^ val4;
            int temp3 = val5 + val6;
            
            func3();
            
            result = temp1 + temp2 * temp3;
            break;
            
        default:
            /* Complex computation without call */
            result = val1 * val2 * val3 * val4;
            break;
    }
    
    /* Use all values to keep them live */
    return result + val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
}

/* Test 3: Function call between complex operations with loop */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int seed) {
    /* Initialize many variables from loop computations */
    int vars[12];
    
    /* Compute many values in a loop to create register pressure */
    for (int i = 0; i < 12; i++) {
        vars[i] = seed * i + i * i;
        /* Additional computations to prevent simple optimizations */
        vars[i] ^= (seed << i);
        vars[i] += (i * 3) % 7;
    }
    
    /* More independent computations */
    int extra1 = seed * 3 + 1;
    int extra2 = seed ^ 0xABCD;
    int extra3 = (seed << 4) | (seed >> 4);
    int extra4 = ~seed;
    int extra5 = seed * seed;
    int extra6 = seed + 0x1234;
    int extra7 = seed & 0xF0F0;
    int extra8 = seed | 0x0F0F;
    
    /* Function call with many live values */
    func1();
    
    /* Use all computed values after the call */
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += vars[i];
    }
    
    sum += extra1 + extra2 + extra3 + extra4;
    sum += extra5 + extra6 + extra7 + extra8;
    
    return sum;
}

/* Test 4: Multiple calls in different basic blocks */
__attribute__((noinline, noipa))
int test_multiple_calls_bb_end(int a, int b) {
    int result = 0;
    
    /* First basic block with live values */
    int x1 = a + b;
    int x2 = a * b;
    int x3 = a ^ b;
    int x4 = a << 2;
    int x5 = b >> 1;
    
    if (a > b) {
        int y1 = x1 * 2;
        int y2 = x2 + x3;
        int y3 = x4 ^ x5;
        
        /* Call at potential BB_END before return */
        func2(y1);
        
        return y1 + y2 + y3;
    } else if (a < b) {
        int z1 = x1 + x2;
        int z2 = x3 * x4;
        int z3 = x5 & 0xFF;
        
        /* Another call at potential BB_END */
        func3();
        
        result = z1 - z2 + z3;
    } else {
        int w1 = x1 ^ x2;
        int w2 = x3 | x4;
        int w3 = x5 * 3;
        
        /* Third call site */
        func1();
        
        result = w1 + w2 * w3;
    }
    
    /* Use values to keep them live */
    return result + x1 + x2 + x3 + x4 + x5;
}

/* Test 5: Call through volatile function pointer */
__attribute__((noinline, noipa))
int test_volatile_call(int a, int b, int c, int d) {
    /* Volatile function pointer ensures call isn't optimized away */
    void (*volatile fp)(void) = func1;
    
    /* Many live values */
    int l1 = a + b + c + d;
    int l2 = a * b - c * d;
    int l3 = (a ^ b) | (c ^ d);
    int l4 = (a << b) & (c >> d);
    int l5 = ~(a & b & c & d);
    int l6 = a * 2 + b * 3 + c * 4 + d * 5;
    int l7 = (a | b) ^ (c | d);
    int l8 = a + (b << 1) + (c << 2) + (d << 3);
    int l9 = (a * b) ^ (c * d);
    int l10 = ~a + ~b - ~c + ~d;
    
    /* Call through volatile pointer */
    fp();
    
    /* Use all values */
    return l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
}

int main(void) {
    int total = 0;
    
    /* Run tests with different inputs to ensure varied execution paths */
    total += test_call_at_bb_end(1, 2, 3, 4, 5, 6, 7, 8);
    total += test_call_at_bb_end(10, 20, 30, 40, 50, 60, 70, 80);
    
    total += test_call_in_switch_case(5, 10, 15);
    total += test_call_in_switch_case(25, 50, 75);
    
    total += test_call_between_complex_ops(42);
    total += test_call_between_complex_ops(123);
    
    total += test_multiple_calls_bb_end(7, 13);
    total += test_multiple_calls_bb_end(20, 10);
    
    total += test_volatile_call(1, 2, 3, 4);
    total += test_volatile_call(5, 6, 7, 8);
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total == 0 ? 1 : 0;
}
