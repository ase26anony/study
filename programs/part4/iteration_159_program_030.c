#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barriers
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) use_result(int x) {
    global_accumulator += x;
    return x;
}

// Test function 1: MIPS target with simple arithmetic after label
__attribute__((target("arch=mips32")))
int test_mips_delay_slot_1(int a, int b) {
    volatile int temp1 = a;
    volatile int temp2 = b;
    int local1 = temp1;
    int local2 = temp2;
    int result = 0;
    
    // Create independent temporaries for the delay slot
    int delay_temp1 = local1 * 2;
    int delay_temp2 = local2 + 1;
    
    // Simple conditional jump to label
    if (local1 > local2) {
        goto target_label_1;
    }
    
    // Some other code to create basic blocks
    result = local1 + local2;
    goto end_1;
    
target_label_1:
    // Safe, non-jump instruction - simple arithmetic
    // Uses independent temporaries to avoid resource conflicts
    delay_temp1 = delay_temp2 & 0xFF;
    result = delay_temp1;
    
end_1:
    return use_result(result);
}

// Test function 2: SPARC target with logical operations
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot_2(int x, int y) {
    volatile int vx = x;
    volatile int vy = y;
    int a = vx;
    int b = vy;
    int res = 0;
    
    // Independent variables for delay slot
    int slot_var1 = a | 0x01;
    int slot_var2 = b ^ 0x80;
    
    // Create multiple basic blocks first
    if (a < 0) {
        res = -a;
    } else {
        // Another conditional jump to label
        if (b != 0) {
            goto compute_label_2;
        }
        res = a * 2;
    }
    
    if (res > 100) {
        return use_result(res);
    }
    
compute_label_2:
    // Safe instruction: logical operation with independent variable
    slot_var1 = slot_var2 << 2;
    res = slot_var1;
    
    return use_result(res);
}

// Test function 3: Generic with multiple jumps
int test_generic_delay_slot_3(int val) {
    volatile int input = val;
    int x = input;
    int result = 0;
    
    // Multiple independent temporaries
    int t1 = x + 100;
    int t2 = x - 50;
    int t3 = x * 3;
    
    // Complex control flow with simple jump at the end
    if (x > 1000) {
        result = x / 2;  // Note: division might trap, but this block won't execute for our test
    } else if (x > 100) {
        result = x + 200;
    } else {
        // This is our target simple jump
        if (x > 0) {
            goto process_small;
        }
        result = 0;
    }
    
    if (result < 0) {
        return use_result(-result);
    }
    
process_small:
    // Safe: use independent temporary with bitwise operation
    t3 = t1 | t2;
    result = t3 & 0x7FFF;
    
    return use_result(result);
}

// Test function 4: Nested conditions with safe delay slot
__attribute__((target("arch=mips32")))
int test_nested_delay_slot_4(int p, int q) {
    volatile int vp = p;
    volatile int vq = q;
    int a = vp;
    int b = vq;
    int out = 0;
    
    // Variables only used after label
    int post_label_var1 = a + b;
    int post_label_var2 = a - b;
    
    // Multiple levels of conditionals
    if (a > 0) {
        if (b > 0) {
            out = a * b;
        } else {
            // Target jump
            if (a > b) {
                goto adjust_values;
            }
            out = a + b;
        }
    } else {
        out = b - a;
    }
    
    // More code to prevent tail optimization
    out = out % 256;
    
    if (out == 0) {
        return use_result(1);
    }
    
adjust_values:
    // Safe: arithmetic with independent variables
    post_label_var1 = post_label_var2 + 42;
    out = post_label_var1;
    
    return use_result(out);
}

// Test function 5: Loop with internal jump to label
__attribute__((target("arch=sparc")))
int test_loop_delay_slot_5(int n) {
    volatile int vn = n;
    int count = (vn & 0x7F) + 1;  // Ensure positive, small count
    int sum = 0;
    int i;
    
    // Independent variable for delay slot
    int delay_var = count * 2;
    
    for (i = 0; i < count; i++) {
        sum += i;
        
        // Occasionally jump to label
        if (i == count / 2) {
            goto halfway_label;
        }
    }
    
    sum = sum * 2;
    return use_result(sum);
    
halfway_label:
    // Safe: modify independent variable
    delay_var = delay_var ^ 0xAA;
    sum = sum + delay_var;
    
    // Continue loop
    for (; i < count; i++) {
        sum += i * 2;
    }
    
    return use_result(sum);
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    srand(seed);
    
    // Call test functions with different inputs
    int result1 = test_mips_delay_slot_1(rand() % 100, rand() % 100);
    int result2 = test_sparc_delay_slot_2(rand() % 200, rand() % 200);
    int result3 = test_generic_delay_slot_3(rand() % 300);
    int result4 = test_nested_delay_slot_4(rand() % 150, rand() % 150);
    int result5 = test_loop_delay_slot_5(rand() % 50);
    
    // Compute checksum
    int checksum = result1 + result2 + result3 + result4 + result5;
    checksum += global_accumulator;
    
    printf("Test results: %d, %d, %d, %d, %d\n", 
           result1, result2, result3, result4, result5);
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return (checksum != 0) ? 0 : 1;
}
