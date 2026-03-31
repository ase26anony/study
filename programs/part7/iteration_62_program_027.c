#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structure
__attribute__((noinline)) int test_for_loop_signed(int n) {
    volatile int dummy1 = 1, dummy2 = 2, dummy3 = 3, dummy4 = 4;
    volatile int dummy5 = 5, dummy6 = 6, dummy7 = 7, dummy8 = 8;
    volatile int dummy9 = 9, dummy10 = 10, dummy11 = 11, dummy12 = 12;
    
    int sum = 0;
    // Pattern: for (int i = N; i != 0; i--)
    for (int i = n; i != 0; i--) {
        // Control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 5 == 0) {
            sum -= i;
        } else {
            sum += 1;
        }
        
        // Use volatile variables to prevent optimization
        dummy1 = dummy2 + dummy3;
        dummy4 = dummy5 * dummy6;
    }
    
    // Post-loop use of counter
    int result = sum + (i == 0 ? 100 : 0);
    
    // More register pressure
    volatile int final_dummy = dummy1 + dummy2 + dummy3 + dummy4 + 
                               dummy5 + dummy6 + dummy7 + dummy8;
    return result + final_dummy;
}

__attribute__((noinline)) int test_for_loop_unsigned(unsigned int n) {
    volatile int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    volatile int r5 = 5, r6 = 6, r7 = 7, r8 = 8;
    volatile int r9 = 9, r10 = 10, r11 = 11, r12 = 12;
    
    int sum = 0;
    // Pattern: for (unsigned i = N; i > 0; i--)
    for (unsigned int i = n; i > 0; i--) {
        // Complex control flow
        switch (i % 4) {
            case 0: sum += 10; break;
            case 1: sum += 5; break;
            case 2: sum -= 3; break;
            case 3: sum += i; break;
        }
        
        // Conditional break based on counter
        if (i == n / 2) {
            sum += 50;
        }
        
        // Register pressure
        r1 = r2 + r3;
        r4 = r5 * r6;
        r7 = r8 ^ r9;
    }
    
    // Post-loop use
    unsigned int final_i = i;
    int result = sum + (final_i == 0 ? 200 : 0);
    
    return result + r1 + r4 + r7;
}

__attribute__((noinline)) int test_while_loop(int n) {
    volatile int v1 = 10, v2 = 20, v3 = 30, v4 = 40;
    volatile int v5 = 50, v6 = 60, v7 = 70, v8 = 80;
    
    int sum = 0;
    int count = n;
    
    // Pattern: while (count-- > 0)
    while (count-- > 0) {
        // Control flow with counter
        if (count % 2 == 0) {
            sum += count * 3;
        } else {
            sum += 7;
        }
        
        // More complex if-else chain
        if (sum > 1000) {
            sum -= 500;
        } else if (sum < -100) {
            sum += 200;
        }
        
        // Register pressure
        v1 = v2 - v3;
        v4 = v5 / 2;
        v6 = v7 % 3;
    }
    
    // Post-loop use of counter
    int result = sum + (count == -1 ? 300 : 0);
    
    return result + v1 + v4 + v6 + v8;
}

__attribute__((noinline)) int test_do_while_loop(unsigned int n) {
    volatile int t1 = 100, t2 = 200, t3 = 300, t4 = 400;
    volatile int t5 = 500, t6 = 600, t7 = 700, t8 = 800;
    volatile int t9 = 900, t10 = 1000, t11 = 1100, t12 = 1200;
    
    int sum = 0;
    unsigned int counter = n;
    
    // Pattern: do { ... } while (--counter);
    if (counter > 0) {  // Ensure at least one iteration
        do {
            // Control flow
            if (counter % 7 == 0) {
                sum += 70;
            } else {
                sum += counter;
            }
            
            // Nested if
            if (sum % 11 == 0) {
                sum += 11;
            }
            
            // Register pressure
            t1 = t2 + t3;
            t4 = t5 - t6;
            t7 = t8 * 2;
            t9 = t10 / 5;
        } while (--counter);
    }
    
    // Post-loop use
    int result = sum + (counter == 0 ? 400 : 0);
    
    return result + t1 + t4 + t7 + t9;
}

__attribute__((noinline)) int test_counter_modified_in_body(int n) {
    volatile int a1 = 1, a2 = 2, a3 = 3, a4 = 4;
    volatile int a5 = 5, a6 = 6, a7 = 7, a8 = 8;
    volatile int a9 = 9, a10 = 10, a11 = 11, a12 = 12;
    
    int sum = 0;
    int i = n;
    
    // Pattern: counter modified inside body with separate comparison
    while (i != 0) {
        sum += i * 2;
        
        // Control flow
        if (sum > 100) {
            sum -= 50;
        }
        
        // Counter modification inside body (i -= 1)
        i -= 1;
        
        // More register pressure
        a1 = a2 * a3;
        a4 = a5 + a6;
        a7 = a8 - a9;
        a10 = a11 ^ a12;
    }
    
    // Post-loop use
    int result = sum + (i == 0 ? 500 : 0);
    
    return result + a1 + a4 + a7 + a10;
}

__attribute__((noinline)) int test_mixed_signedness(int n) {
    volatile int m1 = 15, m2 = 25, m3 = 35, m4 = 45;
    volatile int m5 = 55, m6 = 65, m7 = 75, m8 = 85;
    
    int sum = 0;
    unsigned int u_counter = (unsigned int)n;
    int s_counter = n;
    
    // Two loops with different signedness
    for (; u_counter > 0; u_counter--) {
        sum += (int)u_counter;
        m1 = m2 + m3;
        m4 = m5 - m6;
    }
    
    for (; s_counter != 0; s_counter--) {
        sum += s_counter * 3;
        m7 = m8 * 2;
    }
    
    // Post-loop use of both counters
    int result = sum + (u_counter == 0 ? 600 : 0) + (s_counter == 0 ? 600 : 0);
    
    return result + m1 + m4 + m7;
}

int main() {
    int total = 0;
    
    // Call all test functions with moderate iteration counts
    total += test_for_loop_signed(50);      // 50 iterations
    total += test_for_loop_unsigned(75);    // 75 iterations
    total += test_while_loop(40);           // 40 iterations
    total += test_do_while_loop(30);        // 30 iterations
    total += test_counter_modified_in_body(60); // 60 iterations
    total += test_mixed_signedness(25);     // 25 + 25 iterations
    
    printf("Total result: %d\n", total);
    
    // Prevent optimization of entire program
    volatile int prevent_opt = total;
    if (prevent_opt == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
