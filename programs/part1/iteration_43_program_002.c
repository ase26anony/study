/*
 * Test program to trigger delay slot filling logic in GCC's reorg pass
 * Specifically targets the uncovered block in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_a = 42;
volatile int global_b = 17;
volatile int global_c = 99;

/* Barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
int test_simple_jump_arithmetic(int x, int y) {
    int result = x;
    
    if (x > y) {
        COMPILER_BARRIER();
        /* This should generate a simple jump to label */
        goto target1;
    }
    
    /* Fall through path */
    result = y - x;
    return result;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = x + 1;  /* Uses x which is set before jump */
    return result;
}

/* Test 2: Unconditional jump via goto with logical operation at target */
int test_unconditional_jump_logical(int a, int b) {
    int temp = a & 0xFF;
    
    if (a != b) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    return temp | b;
    
target2:
    /* Candidate: Logical operation with temporary variable */
    temp = temp ^ 0x55;  /* Uses temp which is local and not live across */
    return temp;
}

/* Test 3: Nested conditional with multiple candidates */
int test_nested_conditional(int x, int y, int z) {
    int local1 = x * 2;
    int local2 = y + 3;
    
    if (x > 0) {
        if (y < 10) {
            COMPILER_BARRIER();
            goto target3;
        }
        local1 = z;
    }
    
    return local1 + local2;
    
target3:
    /* Multiple simple instructions at target */
    local1 = local1 + 5;    /* First candidate */
    local2 = local2 - 2;    /* Second candidate */
    return local1 * local2;
}

/* Test 4: Jump with register-only operations at target */
int test_register_only(int a, int b) {
    register int r1 asm("$t0") = a;  /* Suggest register usage */
    register int r2 asm("$t1") = b;
    
    if (r1 != r2) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return r1;
    
target4:
    /* Pure register operation - good candidate for delay slot */
    r1 = r1 + r2;
    return r1;
}

/* Test 5: Function with early return jump */
int test_early_return(int val) {
    int temp = val;
    
    if (temp < 0) {
        COMPILER_BARRIER();
        goto early_exit;
    }
    
    /* Some computation */
    temp = temp * 2;
    if (temp > 100) {
        return temp / 2;
    }
    
    return temp;
    
early_exit:
    /* Simple increment at target */
    temp = temp + 1;
    return temp;
}

/* Test 6: Multiple basic blocks with jumps to common target */
int test_common_target(int a, int b, int c) {
    int res = 0;
    
    if (a > b) {
        res = a - b;
        if (res > c) {
            COMPILER_BARRIER();
            goto common_target;
        }
    } else if (a < b) {
        res = b - a;
        COMPILER_BARRIER();
        goto common_target;
    }
    
    return res + c;
    
common_target:
    /* Instruction that should be safe to move */
    res = res & 0x0F;  /* Mask operation */
    return res;
}

/* Test 7: Loop with conditional exit jump */
int test_loop_exit(int n) {
    int i, sum = 0;
    
    for (i = 0; i < n; i++) {
        sum += i;
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto exit_point;
        }
    }
    
    return sum;
    
exit_point:
    /* Simple operation at exit point */
    sum = sum >> 1;  /* Shift operation */
    return sum;
}

/* Test 8: Switch-like jump via goto */
int test_switch_like(int code) {
    int result = 0;
    
    if (code == 1) {
        result = 10;
        COMPILER_BARRIER();
        goto process;
    } else if (code == 2) {
        result = 20;
        COMPILER_BARRIER();
        goto process;
    } else if (code == 3) {
        result = 30;
        /* No barrier, different path */
        return result;
    }
    
    return -1;
    
process:
    /* Process the result */
    result = result * 2;
    return result;
}

/* Architecture-specific targeting */
#if HAS_DELAY_SLOTS
/* Test specifically crafted for delay slot architectures */
int test_delay_slot_target(int a, int b) {
    int t1, t2;
    
    /* Use explicit register variables to guide allocation */
    register int r_a asm("$t2") = a;
    register int r_b asm("$t3") = b;
    
    /* Create a simple jump scenario */
    if (r_a > r_b) {
        /* Force a simple jump instruction */
        __asm__ volatile(
            "nop \n\t"
            : : : "memory"
        );
        goto ds_target;
    }
    
    t1 = r_a + r_b;
    return t1;
    
ds_target:
    /* Ideal delay slot candidate:
       - Uses temporary registers not live across the jump
       - Simple arithmetic
       - No memory access
       - No function calls
       - No exceptions */
    t2 = r_a - r_b;
    return t2;
}
#endif

/* Main driver */
int main() {
    int checksum = 0;
    
    printf("Testing delay slot filling patterns\n");
    printf("Architecture has delay slots: %s\n", HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests */
    checksum += test_simple_jump_arithmetic(global_a, global_b);
    checksum += test_unconditional_jump_logical(global_b, global_c);
    checksum += test_nested_conditional(global_a, global_b, global_c);
    checksum += test_register_only(5, 3);
    checksum += test_early_return(25);
    checksum += test_common_target(10, 5, 3);
    checksum += test_loop_exit(50);
    checksum += test_switch_like(2);
    
#if HAS_DELAY_SLOTS
    checksum += test_delay_slot_target(global_a, global_b);
    printf("Delay slot specific test included\n");
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify results are consistent */
    if (checksum != 0) {
        printf("Tests completed successfully\n");
    }
    
    return 0;
}
