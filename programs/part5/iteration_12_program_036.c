/* reload1_coverage.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload1_coverage.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexData {
    int64_t a;
    int32_t b[8];
    volatile int16_t c[4][4];
    char d[32];
};

/* Explicit register variables - will force specific register allocation */
register int64_t reg_var1 asm("r10");
register int64_t reg_var2 asm("r11");
register int64_t reg_var3 asm("r12");
register int64_t reg_var4 asm("r13");
register int64_t reg_var5 asm("r14");

/* Global arrays to create addressing complexity */
static volatile struct ComplexData global_array[16];
static int64_t global_index_matrix[8][8];
static volatile int32_t *volatile volatile_ptrs[32];

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables with different types */
    int64_t index1, index2, index3;
    int32_t temp_result;
    volatile int16_t *volatile_ptr;
    struct ComplexData *data_ptr;
    
    /* Initialize explicit register variables */
    reg_var1 = 0x12345678;
    reg_var2 = 0x87654321;
    reg_var3 = (int64_t)&global_array[0];
    reg_var4 = 8;
    reg_var5 = 16;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex index calculations using multiple registers */
        index1 = (reg_var1 + i) % 16;
        index2 = (reg_var2 * i) % 8;
        index3 = (reg_var3 + reg_var4 * i) % 32;
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex inline asm with memory constraint and register clobbers */
        asm volatile (
            /* Operation using memory operand with complex addressing */
            "movq (%[base], %[idx1], 8), %%r15\n\t"
            "addq %%r15, %[out]\n\t"
            /* Clobber many registers to force spills/reloads */
            :
            [out] "+r" (temp_result)
            :
            [base] "r" (&global_index_matrix[0][0]),
            [idx1] "r" (index1 * 8 + index2)
            : "r15", "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        /* Access volatile structure through pointer with offset */
        data_ptr = &global_array[index1];
        volatile_ptr = &data_ptr->c[index2 % 4][index3 % 4];
        
        asm volatile (
            /* Load from volatile memory with address computation */
            "movzwl (%[mem], %[offset], 2), %%eax\n\t"
            "addl %%eax, %[result]\n\t"
            :
            [result] "+r" (temp_result)
            :
            [mem] "r" (volatile_ptr),
            [offset] "r" (reg_var5)
            : "rax", "memory", "cc"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Mixed operand types with immediate and memory */
        int64_t immediate_val = 0xDEADBEEF;
        
        asm volatile (
            /* Complex operation with multiple constraints */
            "imulq %[imm], %[out]\n\t"
            "addq (%[addr]), %[out]\n\t"
            :
            [out] "+r" (reg_var1)
            :
            [imm] "irm" (immediate_val),  /* Immediate, register, or memory */
            [addr] "r" (&global_array[index2].a)  /* Fixed register constraint */
            : "memory", "cc"
        );
        
        /* Pattern 4: RELOAD_FOR_OPADDR_ADDR */
        /* Nested address computation in loop */
        volatile int32_t **ptr_to_ptr = &volatile_ptrs[index3 % 32];
        
        asm volatile (
            /* Double indirection with address reload */
            "movq (%[ptrptr]), %%rbx\n\t"
            "movl (%%rbx, %[idx], 4), %%ecx\n\t"
            "addl %%ecx, %[sum]\n\t"
            :
            [sum] "+r" (temp_result)
            :
            [ptrptr] "r" (ptr_to_ptr),
            [idx] "r" (index1)
            : "rbx", "rcx", "memory", "cc"
        );
        
        /* Pattern 5: Multiple conflicting register uses */
        /* Force reloads by using same registers for different purposes */
        asm volatile (
            "movq %[val1], %%r10\n\t"
            "movq %[val2], %%r11\n\t"
            "leaq (%%r10, %%r11, 4), %%r12\n\t"
            :
            :
            [val1] "r" (reg_var1),
            [val2] "r" (reg_var2)
            : "r10", "r11", "r12", "cc"
        );
        
        /* Update register variables to create live value conflicts */
        reg_var2 = reg_var1 + temp_result;
        reg_var3 = reg_var3 + index1;
        reg_var4 = reg_var4 * 2;
        
        /* Complex array access with multiple dimensions */
        /* This creates addressing modes that may need various reload types */
        int64_t complex_index = (reg_var1 * i + reg_var2 * index1) % 64;
        int64_t *target_addr = (int64_t *)&global_array[complex_index % 16].d[complex_index % 32];
        
        asm volatile (
            /* Memory operation with computed address */
            "movq (%[addr]), %%rdx\n\t"
            "xorq %%rdx, %[out]\n\t"
            :
            [out] "+r" (reg_var5)
            :
            [addr] "r" (target_addr)
            : "rdx", "memory", "cc"
        );
    }
    
    /* Final barrier to prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Helper function to create more reload scenarios */
void nested_reload_scenario(int depth) {
    if (depth <= 0) return;
    
    /* Local variables bound to potentially conflicting registers */
    register int64_t local_reg asm("r15");
    local_reg = depth * 0x1000;
    
    /* Complex addressing with structure fields */
    struct {
        volatile int64_t a;
        int32_t b[4];
        volatile int16_t c;
    } local_struct;
    
    /* Force address computation for structure field */
    int64_t offset = depth * sizeof(int32_t);
    
    asm volatile (
        /* Access structure field with computed offset */
        "movl %[offset](%[base]), %%eax\n\t"
        "addl %%eax, %[sum]\n\t"
        :
        [sum] "+r" (local_reg)
        :
        [base] "r" (&local_struct),
        [offset] "r" (offset)
        : "rax", "memory", "cc"
    );
    
    /* Recursive call to create stack frame complexity */
    nested_reload_scenario(depth - 1);
    
    /* More complex asm after recursion */
    volatile int64_t *volatile_mem = (volatile int64_t *)&global_array[depth % 16];
    
    asm volatile (
        "movq (%[mem], %[idx], 8), %%rcx\n\t"
        "addq %%rcx, %[out]\n\t"
        :
        [out] "+r" (local_reg)
        :
        [mem] "r" (volatile_mem),
        [idx] "r" (depth * 2)
        : "rcx", "memory", "cc"
    );
}

/* Main function to set up and trigger reload patterns */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        global_array[i].a = i * 0x100;
        for (int j = 0; j < 8; j++) {
            global_array[i].b[j] = i + j;
        }
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                global_array[i].c[j][k] = (i + j + k) & 0xFFFF;
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            global_index_matrix[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        volatile_ptrs[i] = &global_array[i % 16].b[0];
    }
    
    /* Trigger various reload scenarios */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Multiple calls with different parameters to create varied patterns */
    trigger_reloads(iterations);
    nested_reload_scenario(5);
    trigger_reloads(iterations / 2);
    
    /* Final complex pattern mixing everything */
    register int64_t final_result asm("r10");
    final_result = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Mixed constraints: immediate, register, and memory */
        int64_t imm = 0xCAFEBABE;
        volatile int64_t *mem_loc = &global_array[i].a;
        
        asm volatile (
            "movq %[imm], %%rax\n\t"
            "addq (%[mem]), %%rax\n\t"
            "addq %%rax, %[out]\n\t"
            :
            [out] "+r" (final_result)
            :
            [imm] "irm" (imm),
            [mem] "r" (mem_loc)
            : "rax", "memory", "cc"
        );
        
        /* Force address reload for operand address */
        struct ComplexData *elem = &global_array[(i * 7) % 16];
        
        asm volatile (
            "leaq %[offset](%[base]), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "xorq %%rcx, %[out]\n\t"
            :
            [out] "+r" (final_result)
            :
            [base] "r" (elem),
            [offset] "r" (offsetof(struct ComplexData, d))
            : "rbx", "rcx", "memory", "cc"
        );
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(final_result));
    
    return (int)(final_result & 0x7FFFFFFF);
}
