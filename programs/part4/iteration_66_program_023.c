/* test_reload_coverage.c
 * 
 * This program creates complex addressing scenarios to trigger
 * various reload types in GCC's reload pass, specifically targeting
 * the switch cases in chain_reload_insns() in reload1.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that might simplify addressing */
#define NOOPT __attribute__((optimize("O0")))

/* Complex nested structure to force address computations */
struct inner {
    int data[8];
    long offset;
};

struct outer {
    struct inner arrays[4];
    int base_index;
    volatile int* dynamic_ptr;
};

/* Global volatile variables to prevent optimizations */
volatile int global_index = 0;
volatile long global_offset = 0;
volatile struct outer* global_struct = NULL;

/* ============================================
   Test 1: RELOAD_FOR_INPUT_ADDRESS
   ============================================ */
NOOPT void test_input_address(struct outer* s, int idx1, int idx2) {
    /* Complex addressing: s->arrays[idx1].data[idx2] */
    /* Force input address reload by using the address in inline asm */
    int result;
    
    /* Use inline asm with memory input constraint */
    asm volatile (
        "movl %[input], %[output]\n\t"
        : [output] "=r" (result)
        : [input] "m" (s->arrays[idx1].data[idx2])
        : "memory"
    );
    
    /* Use result to prevent dead code elimination */
    global_index += result;
}

/* ============================================
   Test 2: RELOAD_FOR_OUTPUT_ADDRESS
   ============================================ */
NOOPT void test_output_address(struct outer* s, int idx, int value) {
    /* Complex output addressing: s->arrays[idx].data[global_index] */
    /* Force output address reload */
    
    asm volatile (
        "movl %[val], %[output]\n\t"
        : [output] "=m" (s->arrays[idx].data[global_index])
        : [val] "r" (value)
        : "memory"
    );
}

/* ============================================
   Test 3: RELOAD_FOR_INPUT_ADDRESS + RELOAD_FOR_OUTPUT_ADDRESS
   ============================================ */
NOOPT void test_mixed_address(struct outer* s, int idx) {
    /* Mixed input and output with complex addressing */
    int temp;
    
    /* Input address reload */
    asm volatile (
        "movl %[in], %[temp]\n\t"
        "addl $1, %[temp]\n\t"
        : [temp] "=r" (temp)
        : [in] "m" (s->arrays[idx].data[global_index])
        : "cc"
    );
    
    /* Output address reload with different complex address */
    asm volatile (
        "movl %[temp], %[out]\n\t"
        : [out] "=m" (s->arrays[idx + 1].data[temp])
        : [temp] "r" (temp)
        : "memory"
    );
}

/* ============================================
   Test 4: RELOAD_FOR_OPERAND_ADDRESS
   ============================================ */
NOOPT void helper_function(int* addr) {
    /* Force address computation before call */
    *addr += 1;
}

NOOPT void test_operand_address(struct outer* s, int i, int j) {
    /* Complex address passed to function */
    helper_function(&s->arrays[i].data[j]);
    
    /* Even more complex address computation */
    helper_function(&s->arrays[i].data[(i * j) + global_index]);
}

/* ============================================
   Test 5: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
   ============================================ */
NOOPT void test_addr_of_addr(struct outer* s, int idx) {
    int* addr1;
    int* addr2;
    
    /* Get address of array element (inpaddr) */
    asm volatile (
        "leaq %[array], %[addr]\n\t"
        : [addr] "=r" (addr1)
        : [array] "m" (s->arrays[idx].data[0])
        : 
    );
    
    /* Store to address of pointer (outaddr) */
    asm volatile (
        "movq %[addr], %[ptr]\n\t"
        : [ptr] "=m" (s->dynamic_ptr)
        : [addr] "r" (addr1)
        : "memory"
    );
}

/* ============================================
   Test 6: RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER
   ============================================ */
NOOPT void test_other_address(struct outer* s) {
    /* Complex addressing in loop with multiple operations */
    int sum = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            /* Multiple memory operations with complex addressing */
            int val1, val2;
            
            /* First complex address */
            asm volatile (
                "movl %[in1], %[val1]\n\t"
                : [val1] "=r" (val1)
                : [in1] "m" (s->arrays[i].data[j])
                : 
            );
            
            /* Second complex address with offset */
            asm volatile (
                "movl %[in2], %[val2]\n\t"
                : [val2] "=r" (val2)
                : [in2] "m" (s->arrays[(i + 1) % 4].data[(j + 1) % 8])
                : 
            );
            
            /* Operation using both values */
            asm volatile (
                "addl %[v2], %[v1]\n\t"
                : [v1] "+r" (val1)
                : [v2] "r" (val2)
                : "cc"
            );
            
            sum += val1;
        }
    }
    
    global_index = sum;
}

/* ============================================
   Test 7: Complex addressing with shifting
   ============================================ */
NOOPT void test_shifted_index(struct outer* s, int base) {
    /* Addressing with shifted index: array[index << 2] */
    /* This often requires additional reloads */
    
    for (int i = 0; i < 4; i++) {
        int shifted_idx = i << 2;
        
        /* Input with shifted index */
        int value;
        asm volatile (
            "movl %[input], %[val]\n\t"
            : [val] "=r" (value)
            : [input] "m" (s->arrays[base].data[shifted_idx % 8])
            : 
        );
        
        /* Output with different shifted index */
        asm volatile (
            "movl %[val], %[output]\n\t"
            : [output] "=m" (s->arrays[(base + 1) % 4].data[(shifted_idx + 1) % 8])
            : [val] "r" (value + 1)
            : "memory"
        );
    }
}

/* ============================================
   Test 8: Multiple register pressure
   ============================================ */
NOOPT void test_register_pressure(struct outer* s) {
    /* Use many variables to create register pressure */
    int r1 = s->base_index;
    int r2 = global_index;
    int r3 = s->arrays[0].offset;
    int r4 = s->arrays[1].offset;
    int r5 = s->arrays[2].offset;
    int r6 = s->arrays[3].offset;
    int r7 = r1 + r2;
    int r8 = r3 + r4;
    int r9 = r5 + r6;
    int r10 = r7 + r8;
    
    /* Complex addressing using many registers */
    asm volatile (
        "addl %[a], %[b]\n\t"
        "movl %[b], %[out]\n\t"
        : [out] "=m" (s->arrays[r1 % 4].data[r2 % 8])
        : [a] "r" (r9), [b] "r" (r10)
        : "memory", "cc"
    );
    
    /* More operations to keep values live */
    asm volatile (
        ""
        : 
        : "r" (r1), "r" (r2), "r" (r3), "r" (r4), 
          "r" (r5), "r" (r6), "r" (r7), "r" (r8)
        : 
    );
}

/* ============================================
   Main driver function
   ============================================ */
int main() {
    /* Allocate and initialize test structure */
    struct outer* s = (struct outer*)malloc(sizeof(struct outer));
    if (!s) return 1;
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            s->arrays[i].data[j] = i * 100 + j;
        }
        s->arrays[i].offset = i * 1000;
    }
    s->base_index = 1;
    s->dynamic_ptr = &global_index;
    
    global_struct = s;
    
    printf("Starting reload coverage tests...\n");
    
    /* Run all tests to trigger different reload types */
    test_input_address(s, 0, 1);
    test_output_address(s, 1, 42);
    test_mixed_address(s, 2);
    test_operand_address(s, 0, 2);
    test_addr_of_addr(s, 1);
    test_other_address(s);
    test_shifted_index(s, 0);
    test_register_pressure(s);
    
    /* Run multiple iterations with different indices */
    for (int iter = 0; iter < 10; iter++) {
        global_index = iter;
        global_offset = iter * 10;
        
        test_input_address(s, iter % 4, (iter * 3) % 8);
        test_output_address(s, (iter + 1) % 4, iter * 100);
        
        if (iter % 3 == 0) {
            test_mixed_address(s, iter % 3);
        }
    }
    
    /* Compute checksum to ensure all operations executed */
    int checksum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += s->arrays[i].data[j];
        }
        checksum += s->arrays[i].offset;
    }
    checksum += global_index;
    
    printf("Final checksum: %d\n", checksum);
    printf("Tests completed.\n");
    
    free(s);
    return 0;
}
