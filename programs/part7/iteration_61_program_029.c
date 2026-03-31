/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -c test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -c test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization on specific functions */
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - most likely candidate */
static int __attribute__((optimize("O2"))) test_arithmetic_after_label(void) {
    volatile int a = 10, b = 20, c = 30;
    int result = 0;
    
    if (a < b) {
        goto target_label1;
    }
    
    /* Dead code to create jump opportunity */
    result = 100;
    return result;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic, no side effects */
    c = a + b;  /* This should be movable into delay slot */
    result = c * 2;
    
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_after_label(void) {
    uint32_t x = 0x12345678;
    uint32_t y = 0x87654321;
    uint32_t z = 0;
    int count = 5;
    
    /* Loop to encourage optimization */
    while (count-- > 0) {
        if (x != y) {
            goto bitwise_label;
        }
        x += 1;
    }
    
    return 0;
    
bitwise_label:
    /* Candidate: bitwise operation, no memory access */
    z = x & y;  /* Safe operation, can be split */
    return (int)(z >> 16);
}

/* Test 3: Stack-based memory operation (load/store) */
static int __attribute__((optimize("O2"))) test_stack_ops_after_label(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    int i = 0;
    
    for (i = 0; i < 3; i++) {
        if (array[i] < array[i+1]) {
            goto stack_label;
        }
    }
    
    return -1;
    
stack_label:
    /* Candidate: stack load operation - should not trap */
    temp = array[2];  /* Safe stack access */
    array[3] = temp + 1;  /* Safe stack store */
    
    return array[3];
}

/* Test 4: Comparison operation after label */
static int __attribute__((optimize("O2"))) test_compare_after_label(void) {
    int p = 100, q = 200;
    int r = 300, s = 400;
    int ret = 0;
    
    /* Nested conditions to create jump opportunities */
    if (p < q) {
        if (q < r) {
            if (r < s) {
                goto compare_label;
            }
        }
    }
    
    return 0;
    
compare_label:
    /* Candidate: comparison that sets condition codes */
    ret = (p == q);  /* Simple comparison, no side effects */
    return ret + (r > s);
}

/* Test 5: Register move pattern with multiple variables */
static int __attribute__((optimize("O2"))) test_register_move(void) {
    register int v1 asm("t0") = 1;
    register int v2 asm("t1") = 2;
    register int v3 asm("t2") = 3;
    register int v4 asm("t3") = 4;
    int decision = 0;
    
    /* Force register usage */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4));
    
    decision = v1 + v2;
    
    if (decision > v3) {
        goto move_label;
    }
    
    return v4;
    
move_label:
    /* Candidate: register-to-register move pattern */
    v4 = v3;  /* Simple move, no resource conflicts */
    return v4 + v1;
}

/* Test 6: Mixed operations in loop with label jump */
static int __attribute__((optimize("O3"))) test_loop_with_label_jump(void) {
    int counter = 10;
    int accum = 0;
    int a = 5, b = 7, c = 9;
    
    while (counter > 0) {
        if (counter == 5) {
            goto loop_label;
        }
        accum += counter;
        counter--;
    }
    
    return accum;
    
loop_label:
    /* Candidate: multiple simple operations */
    a = b + c;      /* First operation - could be split */
    accum = a * 2;  /* Second operation */
    
    return accum;
}

/* Test 7: Avoid trapping operations - use mask instead of division */
static int __attribute__((optimize("O2"))) test_safe_operations(void) {
    unsigned int mask = 0xFF;
    unsigned int value = 0xABCD;
    int threshold = 1000;
    
    if (value > threshold) {
        goto safe_label;
    }
    
    return value;
    
safe_label:
    /* Candidate: safe bitmask operation (won't trap) */
    value = value & mask;  /* No division, no pointer dereference */
    return (int)value;
}

/* Test 8: Multiple basic blocks with labels */
static int __attribute__((optimize("O2"))) test_multiple_labels(void) {
    int x = 0, y = 0, z = 0;
    int i;
    
    for (i = 0; i < 10; i++) {
        x += i;
        
        if (i == 3) {
            goto label_a;
        }
        
        if (i == 6) {
            goto label_b;
        }
    }
    
    return x;
    
label_a:
    y = x << 2;  /* Candidate operation after label A */
    goto join_point;
    
label_b:
    z = x >> 1;  /* Candidate operation after label B */
    goto join_point;
    
join_point:
    return y + z;
}

/* Main function that executes all tests to ensure code paths are taken */
int main(void) {
    int total = 0;
    
    /* Execute all test functions */
    total += test_arithmetic_after_label();
    total += test_bitwise_after_label();
    total += test_stack_ops_after_label();
    total += test_compare_after_label();
    total += test_register_move();
    total += test_loop_with_label_jump();
    total += test_safe_operations();
    total += test_multiple_labels();
    
    printf("Total result: %d\n", total);
    printf("(This output ensures all code paths are executed)\n");
    
    return total != 0 ? 0 : 1;
}
