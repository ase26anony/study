#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loop patterns
__attribute__((noinline))
int test_for_loop_signed(int iterations) {
    volatile int dummy1, dummy2, dummy3, dummy4, dummy5;
    int sum = 0;
    
    // Create register pressure before the loop
    dummy1 = iterations * 2;
    dummy2 = iterations + 123;
    dummy3 = dummy1 ^ dummy2;
    dummy4 = dummy3 * 7;
    dummy5 = dummy4 - 19;
    
    // Target pattern: for (int i = N; i != 0; i--)
    for (int i = iterations; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 5 == 0) {
            sum -= i;
        } else {
            sum += 1;
        }
        
        // Additional operation to prevent trivial loop
        dummy1 = (dummy1 + i) & 0xFF;
    }
    
    // Use counter after loop (prevent elimination)
    sum += dummy5 + (iterations - i); // i should be 0 here
    
    return sum;
}

__attribute__((noinline))
int test_while_loop_unsigned(unsigned int n) {
    volatile int reg1, reg2, reg3, reg4, reg5, reg6;
    int accumulator = 0;
    
    // Create register pressure
    reg1 = n * 3;
    reg2 = n + 456;
    reg3 = reg1 | reg2;
    reg4 = reg3 / 2;
    reg5 = reg4 ^ 0x55;
    reg6 = reg5 << 2;
    
    // Target pattern: while (n-- > 0)
    while (n-- > 0) {
        // Control flow with conditional break
        if (accumulator > 1000) {
            accumulator /= 2;
        }
        
        if (n == iterations / 2) {
            accumulator += 50; // Special case at midpoint
        }
        
        accumulator += (n % 4) + 1;
        reg1 = (reg1 + accumulator) & 0xFF;
    }
    
    // Post-loop use of variables
    accumulator += reg6 + (int)n; // n should be 0xFFFFFFFF
    
    return accumulator;
}

__attribute__((noinline))
int test_do_while_decrement(int count) {
    volatile int pressure1, pressure2, pressure3, pressure4;
    volatile int pressure5, pressure6, pressure7, pressure8;
    int result = 0;
    
    // Heavy register pressure
    pressure1 = count * 5;
    pressure2 = count - 789;
    pressure3 = pressure1 & pressure2;
    pressure4 = pressure3 | 0xAA;
    pressure5 = pressure4 * 3;
    pressure6 = pressure5 ^ 0xCC;
    pressure7 = pressure6 + 111;
    pressure8 = pressure7 >> 1;
    
    // Target pattern: do { ... } while (--count);
    if (count > 0) { // Ensure we enter at least once
        do {
            // Complex enough body
            if (count % 2 == 0) {
                result += count * 3;
            } else {
                result += count;
            }
            
            // Modify pressure variables
            pressure1 = (pressure1 + result) % 256;
            pressure2 = pressure2 ^ result;
            
            // Early exit possibility
            if (result > 5000) {
                result -= 2500;
            }
        } while (--count);
    }
    
    // Use all pressure variables and counter
    result += pressure1 + pressure2 + pressure3 + pressure4;
    result += pressure5 + pressure6 + pressure7 + pressure8;
    result += count; // count should be 0
    
    return result;
}

__attribute__((noinline))
int test_manual_decrement_unsigned(unsigned int limit) {
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int total = 0;
    
    // Maximum register pressure
    v1 = limit * 2; v2 = limit + 111; v3 = v1 ^ v2;
    v4 = v3 * 5; v5 = v4 - 222; v6 = v5 | 0x33;
    v7 = v6 / 2; v8 = v7 + 333; v9 = v8 ^ 0x66;
    v10 = v9 << 1;
    
    // Target pattern: manual decrement with separate compare
    unsigned int counter = limit;
    while (counter != 0) {
        // Body with control flow
        if (total < 0) {
            total = -total; // Should never happen but adds complexity
        }
        
        switch (counter % 4) {
            case 0: total += 10; break;
            case 1: total += 20; break;
            case 2: total += 30; break;
            case 3: total += 40; break;
        }
        
        // Manual decrement (i = i - 1)
        counter = counter - 1;
        
        // Use volatile vars
        v1 = (v1 + counter) & 0xFF;
    }
    
    // Post-loop usage
    total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    total += (int)counter; // counter should be 0
    
    return total;
}

__attribute__((noinline))
int test_nested_decrement(int outer_iter) {
    volatile int a1, a2, a3, a4, a5;
    int outer_sum = 0;
    
    a1 = outer_iter * 7;
    a2 = outer_iter + 444;
    a3 = a1 & a2;
    a4 = a3 | 0x88;
    a5 = a4 ^ 0x44;
    
    // Outer loop
    for (int i = outer_iter; i > 0; i--) {
        int inner_sum = 0;
        
        // Inner decrementing loop
        int j = 5;
        while (j != 0) {
            inner_sum += j * i;
            j--; // Decrement in body
            
            // Small control flow
            if (inner_sum > 100) {
                inner_sum -= 50;
            }
        }
        
        outer_sum += inner_sum;
        a1 = (a1 + i) & 0xFF;
    }
    
    outer_sum += a1 + a2 + a3 + a4 + a5;
    outer_sum += outer_iter; // Use outer counter
    
    return outer_sum;
}

int main() {
    int total_result = 0;
    
    // Call all test functions with different iteration counts
    total_result += test_for_loop_signed(25);
    total_result += test_while_loop_unsigned(30);
    total_result += test_do_while_decrement(20);
    total_result += test_manual_decrement_unsigned(35);
    total_result += test_nested_decrement(15);
    
    // Additional calls with different values
    total_result += test_for_loop_signed(42);
    total_result += test_while_loop_unsigned(17);
    
    printf("Total result: %d\n", total_result);
    
    // Use result to prevent dead code elimination
    if (total_result > 0) {
        return 0;
    } else {
        return 1;
    }
}
