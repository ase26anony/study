/* test_delay_slots.c - Test program for GCC delay slot filling optimization */
#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__mips) || defined(__sparc__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_a = 42;
volatile int global_b = 17;

/* Memory barrier to prevent reordering across jumps */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple unconditional jump with arithmetic at target */
int test_unconditional_jump(int x, int y) {
    int result = x;
    
    /* Force a simple jump structure */
    if (x != 0) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* This path should not be taken with our test inputs */
    return -1;
    
target1:
    /* Candidate instruction for delay slot:
       Simple arithmetic that doesn't conflict with jump resources */
    result = result + 1;  /* Should use a temporary register */
    global_counter++;
    return result;
}

/* Test 2: Conditional jump based on parameter comparison */
int test_conditional_jump(int a, int b) {
    int temp1 = a;
    int temp2 = b;
    int result = 0;
    
    /* Create a simple conditional jump */
    if (temp1 > temp2) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    /* Alternative path */
    result = temp1 - temp2;
    return result;
    
target2:
    /* Candidate: Logical operation with local temporaries */
    result = temp1 & 0xFF;  /* Simple bitwise operation */
    result = result | 0x1;   /* Another simple operation */
    global_counter += 2;
    return result;
}

/* Test 3: Jump with multiple candidate instructions at target */
int test_multi_candidate(int val) {
    int local1 = val;
    int local2 = val * 2;
    int local3 = 0;
    
    /* Force jump with different condition */
    if (local1 < 100) {
        COMPILER_BARRIER();
        goto target3;
    }
    
    return local1;
    
target3:
    /* Multiple simple instructions that could fill delay slots */
    local3 = local1 + local2;    /* Addition - good candidate */
    local3 = local3 ^ 0x5555;    /* XOR - another candidate */
    global_counter += local3;
    return local3;
}

/* Test 4: Jump with safe memory operation at target */
int test_memory_operation(int *ptr) {
    int value = *ptr;
    int temp = 0;
    
    if (value != 0) {
        COMPILER_BARRIER();
        goto target4;
    }
    
    return 0;
    
target4:
    /* Safe memory operation on local variable */
    temp = value + 1;
    *ptr = temp;  /* Store to original pointer - might be safe if ptr doesn't alias */
    global_counter += temp;
    return temp;
}

/* Test 5: Nested jumps to create multiple opportunities */
int test_nested_jumps(int a, int b, int c) {
    int t1 = a, t2 = b, t3 = c;
    
    if (t1 > t2) {
        if (t2 > t3) {
            COMPILER_BARRIER();
            goto target5;
        }
    }
    
    return t1 + t2 + t3;
    
target5:
    /* Very simple instruction - ideal for delay slot */
    t1 = t1 * 2;  /* Multiplication by 2 often becomes shift */
    global_counter += t1;
    return t1;
}

/* Test 6: Function with switch-like jump table pattern */
int test_switch_like(int code) {
    int result = 0;
    
    switch (code & 3) {
        case 0:
            COMPILER_BARRIER();
            goto target6a;
        case 1:
            result = 1;
            break;
        case 2:
            COMPILER_BARRIER();
            goto target6b;
        default:
            return -1;
    }
    
    return result;
    
target6a:
    result = code << 2;  /* Shift operation */
    global_counter += result;
    return result;
    
target6b:
    result = code >> 1;  /* Another shift */
    global_counter -= result;
    return result;
}

/* Test 7: Loop with exit jump */
int test_loop_exit(int limit) {
    int i, sum = 0;
    
    for (i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            COMPILER_BARRIER();
            goto target7;
        }
    }
    
    return sum;
    
target7:
    /* Simple increment operation */
    sum = sum + 100;
    global_counter += sum;
    return sum;
}

/* Main driver that runs all tests */
int main(void) {
    int results[10];
    int final_result = 0;
    int test_array[5] = {10, 20, 30, 40, 50};
    
    printf("Testing delay slot filling patterns...\n");
    printf("Architecture has delay slots: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO");
    
    /* Run all tests with various inputs */
    results[0] = test_unconditional_jump(5, 3);
    results[1] = test_conditional_jump(100, 50);
    results[2] = test_conditional_jump(50, 100);
    results[3] = test_multi_candidate(25);
    results[4] = test_memory_operation(&test_array[2]);
    results[5] = test_nested_jumps(10, 5, 2);
    results[6] = test_nested_jumps(2, 5, 10);
    results[7] = test_switch_like(8);
    results[8] = test_switch_like(9);
    results[9] = test_loop_exit(50);
    
    /* Combine results to prevent optimization */
    for (int i = 0; i < 10; i++) {
        final_result ^= results[i];  /* XOR all results */
        printf("Test %d result: %d\n", i, results[i]);
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Final checksum: 0x%08x\n", final_result);
    
    return (final_result == 0) ? 0 : 1;
}
