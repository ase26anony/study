/* test_delay_slot.c
 * Designed to trigger delay slot optimization conditions in GCC's reorg.cc
 * Compile with: gcc -O2 -march=mips32 -G0 -fno-delayed-branch -fno-omit-frame-pointer test_delay_slot.c -o test_delay_slot
 */

#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC push_options
#pragma GCC optimize ("O2")

/* Test 1: Simple arithmetic after label */
__attribute__((noinline))
static int test1(int a, int b) {
    int x, y, z;
    volatile int result = 0;
    
    x = a + 1;
    y = b * 2;
    
    if (x > y) {
        goto target_label1;
    }
    
    /* Some code to avoid fall-through optimization */
    z = x + y;
    result += z;
    return result;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    z = x - y;  /* next_trial: simple integer operation */
    result += z;
    
    /* Prevent tail call optimization */
    asm volatile("" : "+r"(result));
    return result + 1;
}

/* Test 2: Register move operation after label */
__attribute__((noinline))
static int test2(int p, int q) {
    int temp1, temp2, temp3;
    volatile int sum = 0;
    
    temp1 = p | 0xFF;
    temp2 = q & 0x0F;
    
    if (temp1 != temp2) {
        goto target_label2;
    }
    
    temp3 = temp1 ^ temp2;
    sum += temp3;
    return sum;
    
target_label2:
    /* Candidate: register move via assignment */
    temp3 = temp1;  /* next_trial: simple move operation */
    sum += temp3;
    
    /* Use result to prevent elimination */
    asm volatile("" : "+r"(sum));
    return sum * 2;
}

/* Test 3: Bit manipulation after label */
__attribute__((noinline))
static int test3(int val) {
    int mask, shifted, result;
    volatile int output = 0;
    
    mask = 0xABCD;
    shifted = val << 2;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 3; i++) {
        if ((shifted & mask) != 0) {
            goto target_label3;
        }
        shifted >>= 1;
    }
    
    result = shifted + mask;
    output += result;
    return output;
    
target_label3:
    /* Candidate: bitwise operation */
    result = shifted | mask;  /* next_trial: bitwise OR */
    output += result;
    
    /* Force register usage */
    asm volatile("" : "+r"(output));
    return output - 1;
}

/* Test 4: Stack-based memory operation (safe load/store) */
__attribute__((noinline))
static int test4(int base) {
    int local_array[4];
    int idx, load_val, store_val;
    volatile int total = 0;
    
    /* Initialize local array */
    for (idx = 0; idx < 4; idx++) {
        local_array[idx] = base + idx;
    }
    
    idx = base & 3;
    
    if (local_array[idx] > 100) {
        goto target_label4;
    }
    
    load_val = local_array[0];
    total += load_val;
    return total;
    
target_label4:
    /* Candidate: stack memory load (shouldn't trap) */
    load_val = local_array[idx];  /* next_trial: stack load operation */
    total += load_val;
    
    /* Also store to stack */
    store_val = load_val + 1;
    local_array[idx] = store_val;
    total += store_val;
    
    asm volatile("" : "+r"(total));
    return total;
}

/* Test 5: Comparison operation after label */
__attribute__((noinline))
static int test5(int a, int b, int c) {
    int cmp1, cmp2, diff;
    volatile int res = 0;
    
    cmp1 = a * b;
    cmp2 = c + 10;
    
    /* Multiple jumps to same label */
    if (cmp1 > cmp2) {
        goto target_label5;
    }
    
    if (cmp1 < cmp2) {
        goto target_label5;
    }
    
    diff = cmp2 - cmp1;
    res += diff;
    return res;
    
target_label5:
    /* Candidate: comparison operation (sets condition codes) */
    diff = (cmp1 == cmp2);  /* next_trial: comparison */
    res += diff;
    
    /* Additional safe arithmetic */
    diff = cmp1 - cmp2;
    res += diff;
    
    asm volatile("" : "+r"(res));
    return res;
}

/* Test 6: Multiple simple instructions after label */
__attribute__((noinline))
static int test6(int seed) {
    int x, y, z, w;
    volatile int accum = 0;
    
    x = seed + 1;
    y = seed * 2;
    z = seed / 3;  /* Division but with constant divisor */
    
    /* Nested condition with goto */
    if (x > 0) {
        if (y < 100) {
            if (z != 0) {
                goto target_label6;
            }
        }
    }
    
    w = x + y + z;
    accum += w;
    return accum;
    
target_label6:
    /* Multiple simple instructions that could be split */
    w = x & y;      /* next_trial: bitwise AND */
    accum += w;
    w = z << 1;     /* Another simple operation */
    accum += w;
    
    asm volatile("" : "+r"(accum));
    return accum;
}

/* Test 7: Unconditional jump to label */
__attribute__((noinline))
static int test7(int val) {
    int a, b, c;
    volatile int out = 0;
    
    a = val;
    b = val + 10;
    
    /* Unconditional goto */
    goto target_label7;
    
    /* Unreachable code */
    c = a + b;
    out += c;
    return out;
    
target_label7:
    /* Candidate: simple increment */
    c = a + 1;  /* next_trial: increment */
    out += c;
    
    /* Jump back to create loop-like structure */
    if (c < 100) {
        a = c;
        goto target_label7;
    }
    
    asm volatile("" : "+r"(out));
    return out;
}

#pragma GCC pop_options

/* Main function that executes all tests */
int main(void) {
    int total_result = 0;
    volatile int i;
    
    /* Execute tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total_result += test1(i, i * 2);
        total_result += test2(i, i + 5);
        total_result += test3(i * 3);
        total_result += test4(i * 4);
        total_result += test5(i, i + 1, i + 2);
        total_result += test6(i + 3);
        total_result += test7(i);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total_result);
    
    /* Use result in conditional to ensure all code paths are considered */
    if (total_result > 0) {
        printf("All tests executed successfully.\n");
    } else {
        printf("Warning: Result is zero.\n");
    }
    
    return total_result != 0 ? 0 : 1;
}
