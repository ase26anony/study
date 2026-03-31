#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loop patterns
__attribute__((noinline)) 
int test_for_loop_signed(int iterations) {
    volatile int dummy1 = 123, dummy2 = 456, dummy3 = 789;
    int sum = 0;
    
    // Create register pressure before the loop
    int r1 = dummy1 * 2;
    int r2 = dummy2 + r1;
    int r3 = dummy3 - r2;
    int r4 = r1 * r2;
    int r5 = r3 + r4;
    int r6 = r5 ^ r2;
    int r7 = r6 * 3;
    int r8 = r7 - r4;
    int r9 = r8 / 2;
    int r10 = r9 | r5;
    
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
        
        // Additional register pressure inside loop
        r1 = r2 + r3;
        r2 = r4 ^ r5;
        r3 = r6 * r7;
    }
    
    // Use counter after loop to prevent elimination
    int result = sum + r10;
    if (iterations > 0) {
        result += 1;  // Ensure counter comparison isn't dead
    }
    
    return result;
}

__attribute__((noinline))
int test_while_loop_unsigned(unsigned int n) {
    volatile int v1 = 100, v2 = 200, v3 = 300;
    int accumulator = 0;
    
    // Register pressure setup
    unsigned int temp1 = v1 + v2;
    unsigned int temp2 = v3 * 2;
    unsigned int temp3 = temp1 | temp2;
    unsigned int temp4 = temp3 ^ 0xFF;
    unsigned int temp5 = temp4 << 2;
    unsigned int temp6 = temp5 >> 1;
    
    // Target pattern: while (n-- > 0)
    while (n-- > 0) {
        accumulator += (int)n;
        
        // Conditional break based on counter
        if (n % 7 == 0) {
            accumulator += 100;
            if (n < 10) {
                // Early exit possibility
                break;
            }
        }
        
        // More register manipulation
        temp1 = temp2 + temp3;
        temp2 = temp4 ^ temp5;
        temp3 = temp6 * 3;
    }
    
    // Post-loop use of counter
    return accumulator + (int)temp6 + (n == 0 ? 1 : 0);
}

__attribute__((noinline))
int test_do_while_loop(int count) {
    volatile long a = 111, b = 222, c = 333;
    long result = 0;
    
    // Create many register-bound variables
    long r1 = a + b;
    long r2 = c * a;
    long r3 = r1 ^ r2;
    long r4 = r3 << 1;
    long r5 = r4 >> 2;
    long r6 = r5 | 0xABCD;
    long r7 = r6 & 0x1234;
    long r8 = r7 * 3;
    long r9 = r8 / 2;
    long r10 = r9 + r1;
    
    // Target pattern: do { ... } while (--count);
    if (count > 0) {
        do {
            result += r10;
            
            // Control flow with counter check
            if (count % 4 == 0) {
                result -= r5;
            } else if (count % 6 == 0) {
                result += r3;
            }
            
            // Modify register variables
            r1 = r2 + r3;
            r2 = r4 - r5;
            r3 = r6 ^ r7;
        } while (--count);
    }
    
    // Ensure counter is used after loop
    return (int)(result % 1000) + (count == 0 ? 10 : 0);
}

__attribute__((noinline))
int test_manual_decrement(int limit) {
    volatile int x = 50, y = 60, z = 70;
    int total = 0;
    
    // Heavy register pressure
    int reg1 = x * y;
    int reg2 = y + z;
    int reg3 = z - x;
    int reg4 = reg1 ^ reg2;
    int reg5 = reg3 | reg4;
    int reg6 = reg5 << 1;
    int reg7 = reg6 >> 2;
    int reg8 = reg7 * 3;
    int reg9 = reg8 / 2;
    int reg10 = reg9 + 1;
    
    // Target pattern: counter modified inside loop with explicit comparison
    int counter = limit;
    while (1) {
        total += reg10;
        
        // Complex control flow
        if (counter & 1) {
            total += reg5;
        } else {
            total -= reg3;
        }
        
        // Manual decrement
        counter -= 1;  // Should produce (plus reg -1)
        
        // Explicit zero comparison
        if (counter == 0) {
            break;
        }
        
        // Rotate register values
        int tmp = reg1;
        reg1 = reg2;
        reg2 = reg3;
        reg3 = reg4;
        reg4 = reg5;
        reg5 = reg6;
        reg6 = reg7;
        reg7 = reg8;
        reg8 = reg9;
        reg9 = reg10;
        reg10 = tmp;
    }
    
    // Use counter after loop
    return total + (counter * 2);
}

__attribute__((noinline))
int test_unsigned_for_loop(unsigned int start) {
    volatile unsigned int u1 = 1000, u2 = 2000, u3 = 3000;
    unsigned int checksum = 0;
    
    // Unsigned register variables
    unsigned int ur1 = u1 + u2;
    unsigned int ur2 = u3 * 2;
    unsigned int ur3 = ur1 | ur2;
    unsigned int ur4 = ur3 ^ 0xFFFF;
    unsigned int ur5 = ur4 << 3;
    unsigned int ur6 = ur5 >> 1;
    unsigned int ur7 = ur6 + ur1;
    unsigned int ur8 = ur7 * 3;
    unsigned int ur9 = ur8 / 4;
    unsigned int ur10 = ur9 & 0xFF;
    
    // Target pattern: for (unsigned i = N; i > 0; i--)
    for (unsigned int i = start; i > 0; i--) {
        checksum += i;
        
        // Conditional based on counter
        if (i % 11 == 0) {
            checksum += ur5;
        }
        
        if (i == start / 2) {
            checksum += ur10 * 2;
        }
        
        // Register shuffling
        ur1 = ur2 + ur3;
        ur2 = ur4 ^ ur5;
        ur3 = ur6 | ur7;
    }
    
    // Post-loop counter use
    return (int)checksum + (start > 0 ? ur10 : 0);
}

int main() {
    int total_result = 0;
    
    // Call all test functions with different iteration counts
    total_result += test_for_loop_signed(42);
    total_result += test_while_loop_unsigned(37);
    total_result += test_do_while_loop(29);
    total_result += test_manual_decrement(53);
    total_result += test_unsigned_for_loop(31);
    
    // Additional calls with different values
    total_result += test_for_loop_signed(19);
    total_result += test_while_loop_unsigned(23);
    total_result += test_do_while_loop(17);
    total_result += test_manual_decrement(13);
    total_result += test_unsigned_for_loop(11);
    
    printf("Total result: %d\n", total_result);
    
    // Use result to prevent optimization
    if (total_result > 1000) {
        return 0;
    } else {
        return 1;
    }
}
