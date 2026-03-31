/* delay_slot_test.c - Test program to trigger delay slot filling logic in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Architecture-specific delay slot targeting */
#if defined(__mips__) || defined(__sparc__) || defined(__mips) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Create a simple jump pattern */
    if (x != 0) {
        /* Force a simplejump_p pattern */
        goto target1;
    }
    
    /* Some code to avoid tail merging */
    result = y * 2;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't trap or conflict */
    result = x + 1;  /* Should be eligible for delay slot */
    return result;
}

/* Test 2: Conditional jump based on comparison */
int test_conditional_jump(int a, int b) {
    int temp = a;
    
    /* Create conditional jump */
    if (a > b) {
        /* Compiler barrier to prevent optimization */
        __asm__ volatile ("" : : : "memory");
        goto target2;
    }
    
    /* Alternative path */
    temp = b - a;
    return temp;
    
target2:
    /* Safe instruction: bitwise operation, no memory access */
    temp = a ^ 0xFF;  /* Should not trap */
    return temp;
}

/* Test 3: Jump with multiple safe instructions at target */
int test_multiple_instructions(int val) {
    int local1 = val;
    int local2 = 0;
    
    if (val % 2 == 0) {
        /* Force jump */
        goto target3;
    }
    
    local1 = val * 3;
    return local1;
    
target3:
    /* Multiple simple instructions - one might be moved */
    local2 = local1 + 5;      /* First candidate */
    local1 = local2 * 2;      /* Second candidate */
    return local1;
}

/* Test 4: Jump with register-only operations (avoid memory) */
int test_register_only(int p, int q) {
    register int r1 asm("$t0") = p;  /* Suggest temporary register */
    register int r2 asm("$t1") = q;
    
    if (p < q) {
        __asm__ volatile ("" : : : "memory");
        goto target4;
    }
    
    r1 = q - p;
    return r1;
    
target4:
    /* Register-only operations to avoid resource conflicts */
    r2 = r1 + r2;    /* Uses only temporary registers */
    r1 = r2 & 0x0F;  /* Simple logical operation */
    return r1;
}

/* Test 5: Nested jumps to create multiple opportunities */
int test_nested_jumps(int x, int y, int z) {
    int result = x;
    
    if (x > y) {
        if (y > z) {
            /* Inner conditional jump */
            goto inner_target;
        }
        result = x + y;
        return result;
    }
    
    result = z * 2;
    return result;
    
inner_target:
    /* Very simple instruction - high chance of being moved */
    result = y + 1;  /* Just increment */
    return result;
}

/* Test 6: Function with switch to create jump table */
int test_switch_jump(int code) {
    int output = 0;
    
    switch (code & 3) {
        case 0:
            output = 1;
            break;
        case 1:
            /* This should create a jump to the target */
            goto switch_target;
        case 2:
            output = 3;
            break;
        default:
            output = 4;
            break;
    }
    
    return output;
    
switch_target:
    /* Safe arithmetic at target */
    output = (code << 1) | 1;  /* Shift and OR - no trap */
    return output;
}

/* Test 7: Avoid using special registers (like $ra on MIPS) */
int test_no_special_regs(int a, int b) {
    /* Use only caller-saved/temporary registers in target */
    int t1 = a;
    int t2 = b;
    
    if (a == b) {
        __asm__ volatile ("" : : : "memory");
        goto safe_target;
    }
    
    t1 = a * b;
    return t1;
    
safe_target:
    /* Operations that shouldn't conflict with jump resources */
    t2 = t1 + 7;
    t1 = t2 ^ t1;  /* XOR is safe */
    return t1;
}

/* Test 8: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        if (i == limit / 2) {
            /* Early exit jump */
            goto loop_target;
        }
        sum += i;
    }
    
    return sum;
    
loop_target:
    /* Instruction at loop exit target */
    sum = sum * 2 + 1;
    return sum;
}

/* Driver function to run all tests */
int main(void) {
    int checksum = 0;
    int i;
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        checksum += test_unconditional_jump(i, i*2);
        checksum += test_conditional_jump(i, i+1);
        checksum += test_multiple_instructions(i);
        checksum += test_register_only(i, i+2);
        checksum += test_nested_jumps(i, i-1, i+1);
        checksum += test_switch_jump(i);
        checksum += test_no_special_regs(i, 10-i);
        checksum += test_loop_exit(i+5);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "Yes" : "No (generic fallback)");
    
    return 0;
}
