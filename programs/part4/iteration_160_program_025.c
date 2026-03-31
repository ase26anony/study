/* test-caller-save.c - Test program to trigger caller-save insertion logic */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of these functions */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_counter += x; }
NOINLINE void func3(void) { /* empty */ }

/* Test 1: Function call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int v1 = a + 1;
    int v2 = b * 2;
    int v3 = c & 0xFF;
    int v4 = d | 0x55;
    int v5 = e ^ 0xAA;
    int v6 = f + 100;
    int v7 = a * b;
    int v8 = c + d;
    int v9 = e - f;
    int v10 = a ^ b ^ c;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = func1;
    
    /* Complex condition to create basic block structure */
    if (v1 > v2) {
        /* More computations to increase register pressure */
        int t1 = v3 + v4;
        int t2 = v5 * v6;
        int t3 = v7 & v8;
        int t4 = v9 | v10;
        
        /* Call at the end of basic block before return */
        fp();  /* This should be BB_END before insertion */
        
        /* Return uses all live values - forces them to be preserved */
        return t1 + t2 + t3 + t4 + v1 + v2;
    } else {
        /* Alternative path with different computations */
        int t5 = v4 - v3;
        int t6 = v6 / (v5 ? v5 : 1);
        int t7 = v8 ^ v9;
        int t8 = v10 & v1;
        
        /* Another call at end of basic block */
        func2(v2);
        
        return t5 + t6 + t7 + t8 + v2 + v3;
    }
}

/* Test 2: Function call in switch case at end of basic block */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int a = x * 3;
    int b = y + 7;
    int c = z & 0xF0;
    int d = x ^ y ^ z;
    int e = a + b;
    int f = c * 2;
    int g = d | 0x33;
    int h = x - y;
    int i = z + 5;
    int j = a & b;
    
    switch (x % 4) {
        case 0:
            /* Use all values before call */
            result = a + b;
            func1();  /* Call at end of case before break */
            break;
            
        case 1:
            result = c - d;
            /* More computations */
            int t1 = e * f;
            int t2 = g ^ h;
            void (*volatile fp2)(void) = func3;
            fp2();  /* Call at end of basic block */
            result += t1 + t2;
            break;
            
        case 2:
            result = i | j;
            /* Even more live values */
            int t3 = a + c + e;
            int t4 = b + d + f;
            int t5 = g * h;
            func2(result);
            result = t3 + t4 + t5;
            break;
            
        default:
            result = a + b + c + d + e + f + g + h + i + j;
            func3();
            break;
    }
    
    /* Use all values after switch to ensure they're live across calls */
    return result + a + b + c;
}

/* Test 3: Complex loop with values live across call */
NOINLINE int test_call_between_complex_ops(int n) {
    int arr[10];
    int sum = 0;
    
    /* Initialize with computations */
    for (int i = 0; i < 10; i++) {
        arr[i] = n * i + (i % 3);
    }
    
    /* Create many computed values */
    int v1 = arr[0] * 2;
    int v2 = arr[1] + 100;
    int v3 = arr[2] & 0xFF;
    int v4 = arr[3] | 0xAA;
    int v5 = arr[4] ^ 0x55;
    int v6 = arr[5] * 3;
    int v7 = arr[6] + 200;
    int v8 = arr[7] & 0xCC;
    int v9 = arr[8] | 0x33;
    int v10 = arr[9] ^ 0x99;
    
    /* Nested loop to increase register pressure */
    for (int i = 0; i < 5; i++) {
        v1 += i;
        v2 -= i * 2;
        v3 |= i;
        v4 ^= arr[i % 10];
        
        /* Function call in the middle of loop */
        if (i == 2) {
            void (*volatile fp)(void) = func1;
            fp();  /* Call with many live values */
        }
        
        v5 += arr[(i + 1) % 10];
        v6 *= (i + 1);
        v7 &= ~i;
        v8 |= 1 << i;
    }
    
    /* Use all values in final computation */
    sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Another call at potentially the end of a basic block */
    if (sum > 1000) {
        func2(sum % 100);
        return sum;
    } else {
        func3();
        return sum * 2;
    }
}

/* Test 4: Multiple calls in different basic blocks */
NOINLINE int test_multiple_calls(int x, int y) {
    int result = 0;
    
    /* Phase 1: Compute many values */
    int a1 = x + y;
    int b1 = x * y;
    int c1 = x ^ y;
    int d1 = x - y;
    int e1 = x & y;
    int f1 = x | y;
    int g1 = x << 2;
    int h1 = y >> 1;
    int i1 = (x + 1) * (y - 1);
    int j1 = (x ^ 0x55) + (y ^ 0xAA);
    
    /* Call 1 - in middle of computations */
    func1();
    
    /* Phase 2: More computations using previous results */
    int a2 = a1 + b1;
    int b2 = c1 * d1;
    int c2 = e1 ^ f1;
    int d2 = g1 | h1;
    int e2 = i1 & j1;
    
    /* Conditional with call at end of basic block */
    if (a2 > b2) {
        int t1 = a2 * 3;
        int t2 = b2 + 5;
        func2(t1);  /* Call at end of if-block */
        result = t1 + t2;
    } else {
        int t3 = c2 - 7;
        int t4 = d2 ^ 0xFF;
        func3();  /* Call at end of else-block */
        result = t3 * t4;
    }
    
    /* Final use of all values */
    return result + a1 + b1 + c1 + d1 + e1 + f1 + g1 + h1 + i1 + j1;
}

int main(void) {
    int total = 0;
    
    /* Seed for pseudo-random but reproducible behavior */
    int seed = 42;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(seed + i, i * 2, i * 3, i * 4, i * 5, i * 6);
        total += test_call_in_switch_case(seed - i, i + 1, i * 7);
        total += test_call_between_complex_ops(seed + i * 11);
        total += test_multiple_calls(seed + i * 3, seed - i * 2);
    }
    
    /* Store result to volatile to prevent optimization */
    global_result = total;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
