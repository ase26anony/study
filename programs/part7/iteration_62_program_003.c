#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loop optimization
__attribute__((noinline)) int test_for_loop_signed(int n) {
    volatile int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    int sum = 0;
    
    // Create register pressure before loop
    int r1 = n * 2, r2 = n * 3, r3 = n * 4;
    int r4 = n * 5, r5 = n * 6, r6 = n * 7;
    dummy1 = r1 + r2; dummy2 = r3 + r4; dummy3 = r5 + r6;
    
    // Target pattern: for (int i = N; i != 0; i--)
    for (int i = n; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 5 == 0) {
            sum -= i;
        } else {
            sum += 1;
        }
        
        // Additional register pressure inside loop
        int t1 = sum * 2, t2 = sum * 3;
        dummy1 = t1; dummy2 = t2;
    }
    
    // Use counter after loop to prevent elimination
    sum += (n - abs(i)); // i should be 0 here
    
    // More register pressure after loop
    r1 = sum * 11; r2 = sum * 13; r3 = sum * 17;
    return sum + r1 + r2 + r3;
}

__attribute__((noinline)) int test_while_loop_unsigned(unsigned int n) {
    volatile int dummy1 = 0, dummy2 = 0;
    int sum = 0;
    
    // Register pressure
    unsigned int r1 = n * 11, r2 = n * 13, r3 = n * 17;
    unsigned int r4 = n * 19, r5 = n * 23;
    dummy1 = r1 + r2; dummy2 = r3 + r4 + r5;
    
    // Target pattern: while (n-- > 0)
    unsigned int count = n;
    while (count-- > 0) {
        // Control flow with conditional break
        if (sum > 1000) {
            sum /= 2;
        }
        
        if (count % 4 == 0) {
            sum += count * 3;
        } else {
            sum += 2;
        }
        
        // Volatile operations to maintain side effects
        dummy1 = sum;
        dummy2 = count;
    }
    
    // Use counter after loop
    sum += (int)(n - count); // count should be UINT_MAX
    
    return sum;
}

__attribute__((noinline)) int test_do_while_loop(int n) {
    volatile int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    int sum = 0;
    
    // Heavy register pressure
    int r1 = n * 29, r2 = n * 31, r3 = n * 37;
    int r4 = n * 41, r5 = n * 43, r6 = n * 47;
    int r7 = n * 53, r8 = n * 59, r9 = n * 61;
    dummy1 = r1 + r2 + r3; dummy2 = r4 + r5 + r6; dummy3 = r7 + r8 + r9;
    
    // Target pattern: do { ... } while (--count);
    int count = n;
    if (count > 0) {
        do {
            // Complex control flow
            switch (sum % 5) {
                case 0: sum += count * 2; break;
                case 1: sum += 7; break;
                case 2: sum -= count; break;
                case 3: sum += 11; break;
                default: sum += 13; break;
            }
            
            // Additional computation with temporaries
            int t1 = sum * 2, t2 = sum * 3, t3 = sum * 5;
            dummy1 = t1; dummy2 = t2; dummy3 = t3;
            
        } while (--count);
    }
    
    // Post-loop use of counter
    sum += (n - count) * 3;
    
    return sum;
}

__attribute__((noinline)) int test_manual_decrement(int n) {
    volatile int dummy1 = 0;
    int sum = 0;
    
    // Register pressure
    int r1 = n * 67, r2 = n * 71, r3 = n * 73;
    int r4 = n * 79, r5 = n * 83;
    dummy1 = r1 + r2 + r3 + r4 + r5;
    
    // Target pattern: counter modified inside body with explicit compare
    int i = n;
    while (1) {
        // Loop body with computation
        sum += i * i;
        
        // Control flow
        if (i % 7 == 0) {
            sum += 17;
        }
        
        // Manual decrement
        i -= 1;
        
        // Explicit zero comparison (should generate COMPARE with PLUS)
        if (i == 0) {
            break;
        }
        
        dummy1 = sum + i;
    }
    
    // Use counter after loop
    sum += i; // i should be 0
    
    return sum;
}

__attribute__((noinline)) int test_mixed_loop(unsigned int n) {
    volatile int dummy1 = 0, dummy2 = 0;
    int sum = 0;
    
    // Maximum register pressure
    unsigned int regs[10];
    for (int j = 0; j < 10; j++) {
        regs[j] = n * (j + 101);
        dummy1 += regs[j];
    }
    
    // Multiple decrementing loops in same function
    // First loop: unsigned decrement
    unsigned int cnt1 = n;
    while (cnt1 != 0) {
        sum += (cnt1 % 10);
        cnt1--;
        dummy2 = cnt1;
    }
    
    // Second loop: signed decrement with > 0
    int cnt2 = (int)n / 2;
    for (; cnt2 > 0; cnt2--) {
        if (sum % 3 == 0) {
            sum += cnt2 * 5;
        } else {
            sum += 3;
        }
        dummy1 = sum;
    }
    
    // Use both counters
    sum += (int)(n - cnt1) + (n/2 - cnt2);
    
    return sum;
}

int main() {
    int total = 0;
    
    // Call all test functions with different iteration counts
    // to trigger doloop optimization patterns
    total += test_for_loop_signed(50);      // 50 iterations
    total += test_while_loop_unsigned(40);  // 40 iterations  
    total += test_do_while_loop(30);        // 30 iterations
    total += test_manual_decrement(60);     // 60 iterations
    total += test_mixed_loop(25);           // 25 + 12 iterations
    
    printf("Total result: %d\n", total);
    
    // Additional calls with different values to ensure
    // compiler sees multiple execution paths
    if (total > 1000) {
        total += test_for_loop_signed(20);
        total += test_while_loop_unsigned(15);
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
