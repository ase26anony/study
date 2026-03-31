#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barrier functions
int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

int __attribute__((noinline)) get_input(void) {
    static int counter = 0;
    return counter++ & 7;  // Returns 0-7
}

// Test function for MIPS target
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int arg1, int arg2) {
    // Create independent temporary variables
    int temp_a = arg1 * 3;
    int temp_b = arg2 + 7;
    int temp_c = 0;
    int temp_d = 0;
    
    // Create a non-trivial condition using input-dependent values
    if (arg1 > arg2 && (arg1 - arg2) < 10) {
        // This should compile to a simple jump to label
        goto target_label_1;
    }
    
    // Some other code to create basic blocks
    temp_a = use_value(temp_a);
    temp_b = temp_b * 2;
    
    // This is the target label
target_label_1:
    // Safe, non-jump instruction after label
    // Uses independent variables not involved in the jump condition
    temp_c = temp_a + temp_b;  // Simple arithmetic
    
    // Use the result to prevent dead code elimination
    return temp_c;
}

// Test function for SPARC target
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y, int z) {
    // Multiple independent temporaries
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = z ^ 0xAA;
    int result = 0;
    
    // Complex enough condition to avoid constant folding
    volatile int vol = get_input();
    if ((x + y) > z && vol > 2) {
        goto target_label_2;
    }
    
    // Additional basic blocks
    for (int i = 0; i < 2; i++) {
        t1 = t1 + i;
    }
    
target_label_2:
    // Safe instruction: bitwise operation on independent variables
    result = t2 & t3;
    
    return result;
}

// Generic test function
int test_case_3(int a, int b, int c) {
    int local1 = a + b;
    int local2 = b * c;
    int local3 = 0;
    int local4 = 0;
    
    // Create condition that's not trivially true/false
    if ((a ^ b) != 0 && (b % 3) == 1) {
        goto target_label_3;
    }
    
    // Some arithmetic to create scheduling opportunities
    local1 = local1 - c;
    local2 = local2 >> 1;
    
target_label_3:
    // Safe instruction using only local temporaries
    local3 = local1 - local2;
    
    // Use the value
    local4 = use_value(local3);
    return local4;
}

// Another variant with more complex control flow
int test_case_4(int base) {
    int var1 = base + 100;
    int var2 = base * 2;
    int var3 = 0;
    int var4 = 0;
    
    // Nested conditions to create more basic blocks
    if (base > 0) {
        if (base < 50) {
            volatile int check = get_input();
            if (check & 1) {
                goto target_label_4;
            }
        }
        var1 = var1 - 10;
    }
    
    var2 = var2 + 5;
    
target_label_4:
    // Safe: multiplication of independent variables
    var3 = var1 * var2;
    
    return var3;
}

// Test with loop around the target pattern
int test_case_5(int iterations) {
    int sum = 0;
    int tmp1 = 10;
    int tmp2 = 20;
    int tmp3 = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Reset temporaries each iteration
        tmp1 = i + 5;
        tmp2 = i * 2;
        
        if ((i & 3) == 0 && i > 0) {  // Every 4th iteration, but not first
            goto loop_target;
        }
        
        tmp1 = tmp1 + 1;
        continue;
        
    loop_target:
        // Safe instruction in delay slot position
        tmp3 = tmp1 - tmp2;
        sum += tmp3;
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Call test functions with varying inputs
    for (int i = 0; i < 10; i++) {
        checksum += test_case_1(i, i + 1);
        checksum += test_case_2(i, i * 2, i * 3);
        checksum += test_case_3(i, i + 2, i + 3);
        checksum += test_case_4(i);
        checksum += test_case_5(3);  // Small iteration count
    }
    
    // Also test with specific values that might trigger the path
    checksum += test_case_1(5, 3);   // arg1 > arg2
    checksum += test_case_2(10, 5, 12);  // x+y > z
    checksum += test_case_3(7, 8, 9);    // (a^b) != 0
    checksum += test_case_4(25);         // 0 < base < 50
    checksum += test_case_5(8);          // More iterations
    
    global_accumulator = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return checksum != 0 ? 0 : 1;
}
