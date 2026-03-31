/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OUTPUT_ADDRESS
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex data structure to force non-trivial addressing */
struct DataBlock {
    int32_t values[16];
    int64_t counters[8];
    void* pointers[4];
    volatile int32_t status;
};

/* Global volatile structure to prevent optimizations */
volatile struct DataBlock global_block;

/* Explicit register variables - force specific register allocation */
register int32_t* reg_ptr1 asm("r10");
register int32_t* reg_ptr2 asm("r11");
register int64_t reg_index asm("r12");
register int64_t reg_offset asm("r13");
register int64_t reg_temp asm("r14");

/* Function with complex addressing patterns */
void complex_addressing_loop(struct DataBlock* blocks, int count) {
    /* Initialize register variables */
    reg_ptr1 = &blocks[0].values[0];
    reg_ptr2 = &global_block.values[0];
    reg_index = 0;
    reg_offset = 8;
    
    /* Loop with nested address computations */
    for (int i = 0; i < count; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Complex addressing with multiple register components */
        asm volatile (
            /* Memory operand with complex address calculation */
            "movl (%[base], %[idx], 4), %%eax\n\t"
            "addl %%eax, (%[dest], %[off], 2)\n\t"
            : 
            : [base] "r" (reg_ptr1),      /* Base register */
              [idx] "r" (reg_index),      /* Index register - may need address reload */
              [dest] "r" (reg_ptr2),      /* Destination base */
              [off] "r" (reg_offset)      /* Offset register */
            : "eax", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OPADDR_ADDR */
        /* Mixed operand types with immediate and memory */
        int32_t immediate = 42;
        volatile int32_t* volatile_ptr = &blocks[i].status;
        
        asm volatile (
            /* Operation requiring address of volatile memory */
            "movl %[imm], (%[mem])\n\t"
            "addl $1, %[idx]\n\t"
            : [idx] "+r" (reg_index)
            : [imm] "irm" (immediate),    /* Immediate, register, or memory */
              [mem] "r" (volatile_ptr)    /* Register holding address */
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        /* Output operand with address computation */
        int64_t* output_addr;
        asm volatile (
            /* Compute address and store in output */
            "leaq (%[base], %[idx], 8), %[out]\n\t"
            : [out] "=r" (output_addr)
            : [base] "r" (reg_ptr1),
              [idx] "r" (reg_index)
            : "cc"
        );
        
        /* Use the computed address */
        asm volatile (
            "movq %[val], (%[addr])\n\t"
            :
            : [val] "r" (reg_index),
              [addr] "r" (output_addr)
            : "memory"
        );
        
        /* Complex array indexing with multiple dimensions */
        /* This creates pressure for various reload types */
        int32_t* volatile volatile_base = (int32_t*)&global_block;
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            /* Multiple memory accesses with different addressing modes */
            "movl (%[src], %[idx1], 4), %%ebx\n\t"
            "imull %%ebx, %%ebx\n\t"
            "movl %%ebx, (%[dst], %[idx2], 2)\n\t"
            /* Additional address computation */
            "leaq 16(%[src], %[idx1], 4), %[tmp]\n\t"
            :
            : [src] "r" (volatile_base),
              [dst] "r" (reg_ptr2),
              [idx1] "r" (reg_index),
              [idx2] "r" (reg_offset),
              [tmp] "=r" (reg_temp)
            : "ebx", "memory", "cc"
        );
        
        /* Modify offset for next iteration */
        reg_offset = (reg_offset + 1) & 7;
        
        /* Compiler barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

/* Function with inline assembly using explicit register constraints */
void register_pressure_test(void) {
    /* Additional explicit register variables */
    register int64_t reg_a asm("r15");
    register int64_t reg_b asm("rbx");
    register int64_t reg_c asm("rbp");
    
    reg_a = 0x12345678;
    reg_b = 0x87654321;
    reg_c = 0x55555555;
    
    /* Force many register-to-register moves with clobbers */
    /* This creates pressure for RELOAD_FOR_OPERAND_ADDRESS */
    for (int i = 0; i < 10; i++) {
        asm volatile (
            /* Complex operation using multiple fixed registers */
            "movq %[a], %%rax\n\t"
            "addq %[b], %%rax\n\t"
            "xorq %[c], %%rax\n\t"
            "movq %%rax, %[a]\n\t"
            /* Clobber many registers to force spills */
            :
            : [a] "r" (reg_a),
              [b] "r" (reg_b),
              [c] "r" (reg_c)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "cc"
        );
        
        /* Access memory using computed address */
        volatile int64_t* addr = (volatile int64_t*)(reg_a & 0xFFFFFFFF);
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[addr])\n\t"
            :
            : [addr] "r" (addr)
            : "rax", "memory", "cc"
        );
    }
}

/* Main function setting up the test scenario */
int main(void) {
    /* Allocate and initialize data blocks */
    struct DataBlock* blocks = (struct DataBlock*)malloc(4 * sizeof(struct DataBlock));
    if (!blocks) return 1;
    
    /* Initialize global block */
    for (int i = 0; i < 16; i++) {
        global_block.values[i] = i * 2;
    }
    global_block.status = 0;
    
    /* Initialize local blocks */
    for (int b = 0; b < 4; b++) {
        for (int i = 0; i < 16; i++) {
            blocks[b].values[i] = b * 100 + i;
        }
        blocks[b].status = -1;
    }
    
    /* Run tests designed to trigger specific reload types */
    
    /* Test 1: Complex addressing with multiple reload types */
    complex_addressing_loop(blocks, 8);
    
    /* Test 2: Register pressure with explicit register variables */
    register_pressure_test();
    
    /* Test 3: Mixed immediate/memory/register operands */
    {
        volatile int32_t* ptrs[4];
        for (int i = 0; i < 4; i++) {
            ptrs[i] = &blocks[i].status;
        }
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with array of pointers */
        for (int i = 0; i < 4; i++) {
            int32_t imm = i * 100;
            asm volatile (
                /* Mixed constraints forcing address reload */
                "movl %[imm], (%[ptr])\n\t"
                "addl $1, %[idx]\n\t"
                : [idx] "+r" (reg_index)
                : [imm] "irm" (imm),          /* Can be immediate or register */
                  [ptr] "r" (ptrs[i])         /* Address from array */
                : "memory", "cc"
            );
        }
    }
    
    /* Test 4: Nested structure addressing */
    {
        struct Nested {
            struct DataBlock inner[2];
            volatile int64_t meta;
        } nested;
        
        /* Complex addressing through nested structures */
        for (int i = 0; i < 2; i++) {
            volatile int32_t* status_ptr = &nested.inner[i].status;
            int64_t offset = i * sizeof(struct DataBlock);
            
            asm volatile (
                /* Address computation involving structure offset */
                "leaq (%[base], %[off]), %%rax\n\t"
                "movl $999, 64(%%rax)\n\t"  /* Hardcoded offset for status field */
                :
                : [base] "r" (&nested),
                  [off] "r" (offset)
                : "rax", "memory", "cc"
            );
        }
    }
    
    free(blocks);
    
    /* Return something based on global status */
    return global_block.status != 0;
}
