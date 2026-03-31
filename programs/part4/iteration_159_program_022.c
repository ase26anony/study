#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barrier functions
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int use_result(int x) {
    global_accumulator += x;
    return x;
}

// Test function 1: MIPS target with simple arithmetic after label
__attribute__((target("arch=mips32")))
int test_mips_delay_slot_1(int a, int b) {
    volatile int cond = a > b;  // Prevent constant propagation
    int temp1 = a + 7;
    int temp2 = b * 3;
    int temp3 = 0;
    
    // Other basic blocks to create scheduling context
    for (int i = 0; i < 3; i++) {
        temp1 += i;
    }
    
    if (cond) {
        // Simple jump to label
        goto target_label_1;
    }
    
    temp2 = temp1 - 5;
    return use_result(temp2);
    
target_label_1:
    // Safe, non-jump instruction using independent temporaries
    temp3 = temp1 & 0xFF;  // Simple bitwise operation
    return use_result(temp3);
}

// Test function 2: SPARC target with logical operations
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot_2(int x, int y) {
    volatile int flag = (x & 1) != 0;
    int local_a = x + y;
    int local_b = x - y;
    int local_c = 0;
    int local_d = 42;
    
    // Additional control flow
    if (y > 100) {
        local_a = get_input(local_a);
    }
    
    // Create more complex CFG
    for (int i = 0; i < 2; i++) {
        local_b += i;
    }
    
    if (flag) {
        // Simple conditional jump
        goto sparc_target;
    }
    
    local_d = local_a | local_b;
    return use_result(local_d);
    
sparc_target:
    // Safe instruction: arithmetic with constants
    local_c = local_b + 15;  // No trapping, uses independent variable
    return use_result(local_c);
}

// Test function 3: Generic with multiple temporaries
int test_generic_delay_slot_3(int p, int q) {
    volatile int check = p != q;
    int t1 = p * 2;
    int t2 = q / 2;  // Safe division (no zero)
    int t3 = 0;
    int t4 = 1;
    
    // Multiple basic blocks
    if (p > 0) {
        t1 = t1 + p;
    } else {
        t1 = t1 - p;
    }
    
    // Simple jump pattern
    if (check) {
        goto generic_label;
    }
    
    t4 = t1 ^ t2;
    return use_result(t4);
    
generic_label:
    // Safe logical operation with constant
    t3 = t2 << 2;  // Shift operation, no trapping
    return use_result(t3);
}

// Test function 4: More complex scheduling context
__attribute__((target("arch=mips32")))
int test_mips_complex_4(int val) {
    volatile int threshold = 50;
    int accum = val;
    int scratch1 = 0;
    int scratch2 = 100;
    int result = 0;
    
    // Create loop with multiple iterations
    for (int i = 0; i < 4; i++) {
        accum += i * 10;
        if (accum > 200) {
            scratch1 = accum % 100;
        }
    }
    
    // Multiple condition checks
    if (val < threshold) {
        if ((val & 3) == 0) {
            goto mips_target_2;
        }
        scratch2 = scratch1 + val;
    }
    
    result = scratch2 * 2;
    return use_result(result);
    
mips_target_2:
    // Safe arithmetic with independent variable
    result = scratch1 + 8;  // Uses scratch1 defined before jump
    return use_result(result);
}

// Test function 5: SPARC with multiple independent temporaries
__attribute__((target("arch=sparc")))
int test_sparc_multi_temp_5(int a, int b, int c) {
    volatile int choice = (a + b + c) % 3;
    int x = a * b;
    int y = c + 10;
    int z = 0;
    int w = 25;
    
    // Multiple operations before jump
    x = x >> 1;
    y = y & 0x3F;
    
    // Simple jump based on volatile condition
    if (choice == 1) {
        goto sparc_label_2;
    }
    
    w = x | y;
    return use_result(w);
    
sparc_label_2:
    // Safe operation: multiplication with constant
    z = y * 3;  // No overflow check needed for coverage
    return use_result(z);
}

int main() {
    int checksum = 0;
    
    // Seed for pseudo-random but reproducible behavior
    srand(42);
    
    // Call test functions with varying inputs
    checksum += test_mips_delay_slot_1(rand() % 100, rand() % 100);
    checksum += test_sparc_delay_slot_2(rand() % 100, rand() % 100);
    checksum += test_generic_delay_slot_3(rand() % 100 + 1, rand() % 100 + 1);  // +1 to avoid division by zero
    checksum += test_mips_complex_4(rand() % 100);
    checksum += test_sparc_multi_temp_5(rand() % 50, rand() % 50, rand() % 50);
    
    // Add global accumulator
    checksum += global_accumulator;
    
    printf("Test checksum: %d\n", checksum);
    printf("If compiled with -O2 -march=mips32 or -O3 -mcpu=v9,\n");
    printf("the reorg pass should attempt to fill delay slots.\n");
    
    return 0;
}
