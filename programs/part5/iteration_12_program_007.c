/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPERAND_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct ComplexData {
    int64_t matrix[4][8];
    volatile int32_t counters[16];
    struct {
        uintptr_t base;
        int32_t offsets[12];
    } nested;
};

/* Explicit register variables - force specific register allocation */
register uintptr_t reg_base asm("r12");
register int32_t reg_index1 asm("r10");
register int32_t reg_index2 asm("r11");
register int64_t reg_temp asm("r13");

/* Global volatile to prevent optimization */
volatile struct ComplexData global_data;
volatile int32_t* volatile global_ptr_array[8];

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables that will conflict with explicit registers */
    int32_t local_index = 0;
    int64_t local_sum = 0;
    volatile int32_t* volatile ptr;
    
    /* Initialize explicit register variables */
    reg_base = (uintptr_t)&global_data;
    reg_index1 = 1;
    reg_index2 = 2;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory addressing requiring RELOAD_FOR_OTHER_ADDRESS */
        /* Multiple constraints that conflict: memory operand with register address */
        asm volatile (
            "/* Complex addressing pattern 1 */\n\t"
            "movq (%[base], %[idx1], 8), %[temp]\n\t"
            "addq %[temp], %[sum]\n\t"
            : [sum] "+r" (local_sum)
            : [base] "r" (reg_base),
              [idx1] "r" (reg_index1),
              [temp] "r" (reg_temp),
              "m" (*(struct ComplexData*)(reg_base + reg_index1 * 8)) /* Force address reload */
            : "memory", "cc"
        );
        
        /* Pattern 2: Mixed operand types with immediate and memory */
        /* Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        int32_t immediate = 42;
        asm volatile (
            "/* Mixed operand pattern */\n\t"
            "imull %[imm], %[idx2]\n\t"
            "movl %[idx2], (%[base], %[idx1], 4)\n\t"
            : [idx2] "+r" (reg_index2)
            : [base] "r" (reg_base),
              [idx1] "r" (reg_index1),
              [imm] "i" (immediate),
              "m" (global_data.counters[reg_index1]) /* Memory constraint with computed address */
            : "memory", "cc"
        );
        
        /* Pattern 3: Nested address computation in loop */
        /* Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        ptr = &global_data.counters[reg_index2];
        asm volatile (
            "/* Nested address computation */\n\t"
            "leaq (%[base], %[idx2], 4), %[temp]\n\t"
            "addq (%[ptr]), %[temp]\n\t"
            "movq %[temp], (%[base], %[idx1], 8)\n\t"
            : [temp] "=&r" (reg_temp)
            : [base] "r" (reg_base),
              [idx1] "r" (reg_index1),
              [idx2] "r" (reg_index2),
              [ptr] "rm" (ptr), /* Register or memory constraint causing conflicts */
              "m" (global_data.matrix[reg_index1][reg_index2])
            : "memory", "cc"
        );
        
        /* Pattern 4: Multiple explicit register variables with overlapping usage */
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "/* Multiple register pressure */\n\t"
            "movq %[base], %%r14\n\t"
            "addq %[idx1], %%r14\n\t"
            "movq (%%r14), %[temp]\n\t"
            "addq %[temp], %[sum]\n\t"
            : [sum] "+r" (local_sum),
              [temp] "=r" (reg_temp)
            : [base] "r" (reg_base),
              [idx1] "r" (reg_index1),
              "m" (*(volatile int64_t*)(reg_base + reg_index1))
            : "memory", "cc", "r14"
        );
        
        /* Pattern 5: Complex struct member access with volatile */
        /* Forces RELOAD_FOR_OTHER_ADDRESS specifically */
        int32_t offset = global_data.nested.offsets[reg_index1 & 7];
        asm volatile (
            "/* Complex struct addressing */\n\t"
            "movslq %[offset], %[temp]\n\t"
            "addq %[base], %[temp]\n\t"
            "movq (%[temp]), %[temp]\n\t"
            : [temp] "=&r" (reg_temp)
            : [base] "r" (reg_base),
              [offset] "rm" (offset), /* Can be register or memory */
              "m" (global_data.nested), /* Whole struct access */
              "m" (*(struct ComplexData*)reg_base) /* Force address calculation */
            : "memory", "cc"
        );
        
        /* Update indices to create varying address patterns */
        reg_index1 = (reg_index1 * 3 + 1) & 7;
        reg_index2 = (reg_index2 * 5 + 1) & 7;
        local_index++;
    }
    
    /* Final pattern: Array of pointers with complex addressing */
    for (int i = 0; i < 4; i++) {
        global_ptr_array[i] = &global_data.counters[i * 2];
        
        asm volatile (
            "/* Array of pointers pattern */\n\t"
            "movq (%[arr], %[idx], 8), %[temp]\n\t"
            "movl (%[temp]), %k[idx]\n\t"
            : [idx] "+r" (reg_index1),
              [temp] "=r" (reg_temp)
            : [arr] "r" (global_ptr_array),
              "m" (global_ptr_array[reg_index1]), /* Memory with computed index */
              "m" (*(volatile int32_t**)global_ptr_array)
            : "memory", "cc"
        );
    }
}

/* Secondary function to create more reload contexts */
void nested_reload_context(int depth) {
    if (depth <= 0) return;
    
    /* Local array with computed indexing */
    int64_t local_array[16];
    register int32_t idx asm("r14") = depth;
    
    for (int i = 0; i < 8; i++) {
        /* Pattern requiring RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "/* Nested context addressing */\n\t"
            "leaq (%[array], %[idx], 8), %[temp]\n\t"
            "movq $0x12345678, (%[temp])\n\t"
            : [temp] "=&r" (reg_temp)
            : [array] "r" (local_array),
              [idx] "r" (idx),
              "m" (local_array[idx]) /* Computed memory operand */
            : "memory"
        );
        
        idx = (idx * 7 + 3) & 15;
    }
    
    nested_reload_context(depth - 1);
}

int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        global_data.counters[i] = i;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_data.matrix[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 12; i++) {
        global_data.nested.offsets[i] = i * 4;
    }
    global_data.nested.base = (uintptr_t)&global_data;
    
    /* Trigger the reload patterns */
    trigger_reloads(iterations);
    
    /* Create nested contexts for more reload opportunities */
    nested_reload_context(3);
    
    /* Final complex pattern mixing everything */
    {
        volatile struct ComplexData* volatile ptrs[4];
        for (int i = 0; i < 4; i++) {
            ptrs[i] = &global_data;
            
            asm volatile (
                "/* Final mixed pattern */\n\t"
                "movq (%[ptrs], %[idx], 8), %[base]\n\t"
                "movq 32(%[base], %[idx], 4), %[temp]\n\t"
                : [base] "=r" (reg_base),
                  [temp] "=r" (reg_temp)
                : [ptrs] "r" (ptrs),
                  [idx] "r" (i),
                  "m" (ptrs[i]), /* Memory operand with address in register */
                  "m" (*(struct ComplexData**)ptrs) /* Another memory constraint */
                : "memory"
            );
        }
    }
    
    return 0;
}
