/* Program to trigger GCC's delay slot filling logic for uncovered lines in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force the compiler to keep branches as-is */
#define NO_INLINE __attribute__((noinline))

/* Create predictable branch patterns */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

/* Use different register sets to avoid resource conflicts */
#define DECLARE_REG_VAR(type, name, reg) register type name asm(reg)

/* Simple arithmetic operations that won't trap */
static inline int safe_add(int a, int b) { return a + b; }
static inline int safe_sub(int a, int b) { return a - b; }

NO_INLINE void process_block(int *result) {
    /* Use different register sets for branch vs candidate operations */
    DECLARE_REG_VAR(int, branch_a, "$t0");
    DECLARE_REG_VAR(int, branch_b, "$t1");
    DECLARE_REG_VAR(int, cand_x, "$t2");
    DECLARE_REG_VAR(int, cand_y, "$t3");
    DECLARE_REG_VAR(int, cand_z, "$t4");
    
    /* Initialize with values that create varying branch outcomes */
    branch_a = *result & 0xFF;
    branch_b = (*result >> 8) & 0xFF;
    cand_x = (*result >> 16) & 0xFF;
    cand_y = (*result >> 24) & 0xFF;
    cand_z = 0;
    
    /* Create multiple branch patterns to explore different delay slot filling attempts */
    
    /* Pattern 1: Branch with nop filler */
    if (UNLIKELY(branch_a > branch_b)) {
        asm volatile("nop" ::: "memory");
        /* Target label with simple arithmetic that can be moved into delay slot */
        target_label_1:
        cand_z = safe_add(cand_x, 1);  /* Simple trap-free operation */
    } else {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
    }
    
    /* Pattern 2: Different condition, different number of nops */
    if (LIKELY(branch_a != branch_b)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        target_label_2:
        cand_z = safe_sub(cand_y, 2);  /* Another simple trap-free operation */
    }
    
    /* Pattern 3: More complex pattern with multiple potential candidates */
    int temp = cand_z;
    if (UNLIKELY((branch_a + branch_b) > 100)) {
        asm volatile("nop" ::: "memory");
        target_label_3:
        temp = safe_add(temp, cand_x);  /* Uses different registers than branch condition */
        /* Add another independent operation to give filler more options */
        cand_y = safe_add(cand_y, 3);
    }
    
    /* Update result with computations to keep values live */
    *result = (cand_z << 24) | (temp << 16) | (branch_b << 8) | branch_a;
}

NO_INLINE void process_block_2(int *result) {
    /* Alternative pattern with different register usage */
    DECLARE_REG_VAR(int, cond_a, "$s0");
    DECLARE_REG_VAR(int, cond_b, "$s1");
    DECLARE_REG_VAR(int, work_c, "$s2");
    DECLARE_REG_VAR(int, work_d, "$s3");
    
    cond_a = *result & 0xFFFF;
    cond_b = (*result >> 16) & 0xFFFF;
    work_c = 0;
    work_d = 0;
    
    /* Create a loop with varying branch behavior */
    for (int i = 0; i < 3; i++) {
        /* Volatile counter to prevent loop unrolling */
        volatile int loop_counter = i;
        
        if (UNLIKELY(cond_a < cond_b + loop_counter)) {
            /* Multiple nops to create filler opportunities */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            delay_slot_target:
            /* Perfect delay slot candidate: simple, independent operation */
            work_c = safe_add(work_c, work_d + 1);
        } else {
            asm volatile("nop" ::: "memory");
        }
        
        /* Modify condition variables to change branch outcomes */
        cond_a = safe_add(cond_a, i);
        cond_b = safe_sub(cond_b, 1);
    }
    
    *result = work_c + work_d;
}

int main() {
    volatile int iterations = 100;  /* Prevent optimization */
    int result = 0x12345678;        /* Initial seed */
    
    /* Main loop with varying branch patterns */
    for (int i = 0; i < iterations; i++) {
        /* Mix different processing patterns */
        if (i & 1) {
            process_block(&result);
        } else {
            process_block_2(&result);
        }
        
        /* Modify result to create different branch conditions */
        result = safe_add(result, i);
        result ^= 0x5A5A5A5A;  /* XOR to create varying patterns */
        
        /* Prevent tail recursion optimization */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", result & 0xFF);
    return (result & 0xFF) == 0 ? 0 : 1;
}
