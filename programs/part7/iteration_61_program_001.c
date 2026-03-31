/* test_delay_slot.c
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -o test test_delay_slot.c
 * Or for RISC-V: gcc -O2 -march=rv32gc -mabi=ilp32 -fno-omit-frame-pointer -o test test_delay_slot.c
 */

#include <stdio.h>
#include <stdint.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label - likely candidate for delay slot */
static int __attribute__((optimize("O2"))) test_arithmetic_delay(void) {
    volatile int a = 5, b = 3, c = 0;
    int result = 0;
    
    if (a > b) {
        goto arith_label;
    }
    
    return -1;  // Should not reach here
    
arith_label:
    /* Candidate for next_trial: simple arithmetic, no side effects */
    c = a + b;  // This should be movable to delay slot
    result = c * 2;
    
    /* Add another operation to prevent fall-through optimization issues */
    if (result > 10) {
        goto done;
    }
    
done:
    return result;
}

/* Test 2: Bitwise operations after label */
static int __attribute__((optimize("O3"))) test_bitwise_delay(void) {
    uint32_t x = 0xABCD1234;
    uint32_t y = 0x00FF00FF;
    uint32_t z = 0;
    int count = 0;
    
    for (int i = 0; i < 3; i++) {
        if (x != 0) {
            goto bitwise_label;
        }
        count++;
    }
    
    return -1;
    
bitwise_label:
    /* Candidate: bitwise operation, no memory access */
    z = x & y;  // Safe operation for delay slot
    z = z | 0x5500;
    
    /* Use result to prevent elimination */
    return (int)(z >> 16) + count;
}

/* Test 3: Stack-based memory operation (load/store) */
static int __attribute__((optimize("O2"))) test_memory_delay(void) {
    int array[4] = {1, 2, 3, 4};
    int temp = 0;
    int idx = 2;
    
    /* Create condition for jump */
    if (array[0] == 1) {
        goto memory_label;
    }
    
    return -1;
    
memory_label:
    /* Candidate: stack memory load - less likely to trap */
    temp = array[idx];  // Stack access, should be safe
    array[idx] = temp + 1;
    
    /* Compute checksum to use all values */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    return sum;
}

/* Test 4: Comparison operation setting condition codes */
static int __attribute__((optimize("O2"))) test_compare_delay(void) {
    int p = 100;
    int q = 50;
    int r = 0;
    
    /* Nested conditions to create interesting control flow */
    if (p > q) {
        if (q < 200) {
            goto compare_label;
        }
    }
    
    return -1;
    
compare_label:
    /* Candidate: comparison operation - sets flags but no side effects */
    r = (p > q) ? 1 : 0;  // Comparison, can be moved to delay slot
    
    /* Use in further computation */
    int val = p + q + r;
    return val;
}

/* Test 5: Multiple operations with register moves */
static int __attribute__((optimize("O2"))) test_register_delay(void) {
    register int v1 asm("t0") = 10;
    register int v2 asm("t1") = 20;
    register int v3 asm("t2") = 0;
    
    /* Force a simple jump */
    if (v1 != v2) {
        goto register_label;
    }
    
    return -1;
    
register_label:
    /* Candidate: register-to-register operations */
    v3 = v1 + v2;      // First operation - potential delay slot
    int v4 = v3 - 5;   // Second operation
    
    /* Create data dependency to prevent reordering */
    for (int i = 0; i < 2; i++) {
        v4 += i;
    }
    
    return v4;
}

/* Test 6: Loop with internal goto creating jump to label */
static int __attribute__((optimize("O3"))) test_loop_delay(void) {
    int counter = 0;
    int accumulator = 0;
    
    for (int i = 0; i < 5; i++) {
        if (i == 3) {
            goto loop_label;
        }
        counter++;
    }
    
    return -1;
    
loop_label:
    /* Candidate: simple increment operation */
    accumulator = counter + 1;  // Safe for delay slot
    
    /* Continue loop to create more optimization opportunities */
    for (int j = 0; j < 2; j++) {
        accumulator += j;
    }
    
    return accumulator;
}

/* Test 7: Switch-like pattern with goto labels */
static int __attribute__((optimize("O2"))) test_switch_delay(void) {
    int option = 2;
    int result = 0;
    
    if (option == 1) goto case1;
    if (option == 2) goto case2;
    if (option == 3) goto case3;
    
    return -1;

case1:
    result = 100;
    goto end;

case2:
    /* Candidate: arithmetic shift operation */
    result = option << 2;  // Shift operation - good candidate
    goto end;

case3:
    result = 300;
    goto end;

end:
    return result + 10;
}

/* Test 8: Function with multiple basic blocks and safe operations */
static int __attribute__((optimize("O3"))) test_complex_delay(void) {
    int x = 42, y = 17, z = 0;
    int *safe_ptr = &x;  // Always points to stack
    
    /* Multiple conditions leading to same label */
    if (x > 10 && y < 30) {
        if (x % 2 == 0) {
            goto complex_label;
        }
    }
    
    return -1;
    
complex_label:
    /* Candidate: safe memory store to stack location */
    *safe_ptr = y;  // Stack store, should not trap
    
    /* Follow with arithmetic */
    z = x + y;
    
    /* Prevent tail optimization */
    volatile int dummy = z;
    return dummy;
}

#pragma GCC pop_options

/* Main function that executes all tests */
int main(void) {
    int total = 0;
    
    /* Execute all test functions and accumulate results */
    total += test_arithmetic_delay();    // ~8
    total += test_bitwise_delay();       // ~0xAB55 + ?
    total += test_memory_delay();        // 1+2+4+4 = 11
    total += test_compare_delay();       // 100+50+1 = 151
    total += test_register_delay();      // 10+20-5+0+1 = 26
    total += test_loop_delay();          // 3+1+0+1 = 5
    total += test_switch_delay();        // 8+10 = 18
    total += test_complex_delay();       // 17+17 = 34
    
    printf("Total: %d\n", total);
    
    /* Verify all paths were executed */
    if (total > 0) {
        printf("All test patterns executed successfully.\n");
    }
    
    return 0;
}
