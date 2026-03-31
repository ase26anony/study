#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loop optimizations
__attribute__((noinline)) int test_for_loop_signed(int n) {
    volatile int dummy1, dummy2, dummy3, dummy4, dummy5;
    int sum = 0;
    
    // Create register pressure before the loop
    dummy1 = n * 2;
    dummy2 = n + 1;
    dummy3 = dummy1 ^ dummy2;
    dummy4 = dummy3 << 2;
    dummy5 = dummy4 | n;
    
    // Target pattern: for (int i = N; i != 0; i--)
    for (int i = n; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Additional register pressure inside loop
        volatile int temp = sum;
        temp = temp ^ i;
        (void)temp;
    }
    
    // Use counter after loop to prevent elimination
    sum += (n - i);  // i is 0 here, but prevents dead code elimination
    
    return sum + dummy5;
}

__attribute__((noinline)) int test_while_loop_unsigned(unsigned int n) {
    volatile unsigned int d1, d2, d3, d4, d5;
    int sum = 0;
    unsigned int count = n;
    
    // Register pressure setup
    d1 = n * 3;
    d2 = n + 0xABCD;
    d3 = d1 & d2;
    d4 = d3 >> 1;
    d5 = d4 ^ n;
    
    // Target pattern: while (count-- > 0)
    while (count-- > 0) {
        // Control flow with conditional break
        if (count == n / 2) {
            sum += 1000;
        }
        
        sum += (int)count;
        
        // More register pressure
        volatile unsigned int tmp = count;
        tmp = tmp * 7;
        (void)tmp;
    }
    
    // Post-loop use of counter
    sum += (int)(n - count);  // count is UINT_MAX after final decrement
    
    return sum + (int)d5;
}

__attribute__((noinline)) int test_do_while_loop(int n) {
    volatile long r1, r2, r3, r4, r5;
    int sum = 0;
    int iterations = n > 0 ? n : 10;
    
    // Heavy register pressure
    r1 = n * n;
    r2 = n + 0x1234;
    r3 = r1 | r2;
    r4 = r3 & 0xFFFF;
    r5 = r4 - n;
    
    // Target pattern: do { ... } while (--iterations);
    do {
        // Nested control flow
        if (iterations % 4 == 0) {
            sum += iterations * 3;
        } else if (iterations % 4 == 1) {
            sum += iterations;
        } else {
            sum += 1;
        }
        
        // Volatile operations for register pressure
        volatile int v = iterations;
        v = v ^ sum;
        (void)v;
    } while (--iterations);
    
    // Use counter value after loop
    sum -= iterations;  // iterations is 0
    
    return sum + (int)r5;
}

__attribute__((noinline)) int test_counter_modified_in_body(int n) {
    volatile int pressure[10];
    int sum = 0;
    int counter = n;
    
    // Create array access for register pressure
    for (int j = 0; j < 10; j++) {
        pressure[j] = n + j;
    }
    
    // Target pattern: while (counter != 0) with decrement in body
    while (counter != 0) {
        sum += counter;
        
        // Decrement inside body (not in condition)
        counter -= 1;
        
        // Conditional break based on value
        if (counter == n / 3) {
            sum += 500;
        }
        
        // Use pressure array
        volatile int idx = counter % 10;
        sum += pressure[idx];
    }
    
    // Post-loop computation
    sum += counter * 2;  // counter is 0
    
    return sum;
}

__attribute__((noinline)) int test_mixed_loop_types(unsigned short n) {
    volatile int a, b, c, d, e, f, g, h;
    int sum = 0;
    unsigned int loop1 = n;
    int loop2 = (int)n;
    
    // Maximum register pressure
    a = n * 11;
    b = n + 0xDEAD;
    c = a ^ b;
    d = c << 3;
    e = d | 0xBEEF;
    f = e - n;
    g = f / 2;
    h = g & 0xFF;
    
    // First loop: for with i--
    for (unsigned int i = loop1; i != 0; i--) {
        if (i % 5 == 0) {
            sum += (int)i * 2;
        }
        sum += (int)i;
        
        volatile unsigned int t = i;
        t = t * 13;
        (void)t;
    }
    
    // Second loop: while with --count
    int count = loop2;
    while (count > 0) {
        sum += count;
        --count;
        
        if (count % 7 == 0) {
            sum += 77;
        }
    }
    
    // Use all counters
    sum += (int)loop1 - count;
    
    return sum + h;
}

int main() {
    int total = 0;
    
    // Call all test functions with different iteration counts
    total += test_for_loop_signed(50);      // Moderate iteration count
    total += test_while_loop_unsigned(75);  // Different count
    total += test_do_while_loop(25);        // Smaller count
    total += test_counter_modified_in_body(40);
    total += test_mixed_loop_types(30);
    
    // Add some variation with different inputs
    total += test_for_loop_signed(33);
    total += test_while_loop_unsigned(42);
    
    printf("Total result: %d\n", total);
    
    // Use result to prevent optimization
    if (total > 1000) {
        return 0;
    } else {
        return 1;
    }
}
