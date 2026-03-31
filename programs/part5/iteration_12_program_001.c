/* reload1_trigger.c
 * Designed to trigger complex reload scenarios in GCC's reload pass
 * Specifically targets RELOAD_FOR_OTHER_ADDRESS and related reload types
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct DataBlock {
    int64_t values[8];
    int32_t indices[16];
    volatile int16_t flags[32];
    char padding[64];
};

/* Global data to force addressing through pointers */
volatile struct DataBlock global_block[4];
volatile int64_t global_array[256];

/* Explicit register variables - force specific register allocation */
register int64_t* reg_ptr1 asm("r12");
register int64_t* reg_ptr2 asm("r13");
register int64_t index_reg asm("r14");
register int64_t temp_reg asm("r15");

/* Helper to prevent optimization */
static void escape(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

int main(void) {
    /* Local variables with complex types */
    struct DataBlock local_blocks[3];
    volatile int64_t* volatile_ptr = (volatile int64_t*)global_array;
    int64_t* normal_ptr = (int64_t*)global_array;
    
    /* Initialize explicit register variables */
    reg_ptr1 = (int64_t*)&global_block[0];
    reg_ptr2 = (int64_t*)&global_block[1];
    index_reg = 0;
    temp_reg = 0;
    
    /* Complex addressing mode 1: Multiple memory operands with register constraints */
    for (int i = 0; i < 100; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq (%[base1], %[idx1], 8), %%rax\n\t"
            "addq %%rax, (%[base2], %[idx2], 8)\n\t"
            : 
            : [base1] "r" (reg_ptr1), [idx1] "r" (index_reg),
              [base2] "r" (reg_ptr2), [idx2] "r" (index_reg + 4)
            : "rax", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS with mixed constraints */
        int64_t offset = i * 16;
        asm volatile (
            "leaq (%[ptr], %[off]), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            : 
            : [ptr] "r" (normal_ptr), [off] "r" (offset)
            : "rbx", "rcx", "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS with immediate displacement */
        asm volatile (
            "movq %[imm], %%rdx\n\t"
            "addq %%rdx, (%[addr])\n\t"
            : 
            : [imm] "i" (0x1000), [addr] "r" (&global_array[128])
            : "rdx", "memory", "cc"
        );
        
        /* Complex nested addressing with multiple reload types */
        struct DataBlock* block_ptr = &local_blocks[i % 3];
        int32_t complex_index = (i * 7) % 16;
        
        /* This should trigger RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "movslq %[cindex], %%rsi\n\t"
            "movl (%[block], %%rsi, 4), %%eax\n\t"
            "cltq\n\t"
            "movq (%[global], %%rax, 8), %%rdi\n\t"
            : 
            : [block] "r" (&block_ptr->indices[0]),
              [cindex] "rm" (complex_index),
              [global] "r" (global_array)
            : "rsi", "rax", "rdi", "memory", "cc"
        );
        
        /* Update register variables to force spills/reloads */
        index_reg += 8;
        temp_reg = index_reg * 2;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS with volatile */
        asm volatile (
            "movq %[temp], (%[out])\n\t"
            : 
            : [temp] "r" (temp_reg), [out] "r" (volatile_ptr + i)
            : "memory"
        );
        
        /* Mix with explicit memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Second loop with different addressing patterns */
    for (int j = 0; j < 50; j++) {
        /* Force RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t* output_addr = &global_array[j * 2];
        int64_t input_val = j * 100;
        
        asm volatile (
            "movq %[in], %%r8\n\t"
            "movq %%r8, (%[out_addr])\n\t"
            : 
            : [in] "rm" (input_val), [out_addr] "r" (output_addr)
            : "r8", "memory"
        );
        
        /* Complex struct addressing with multiple fields */
        volatile struct DataBlock* vblock = &global_block[j % 4];
        int flag_index = (j * 3) % 32;
        
        /* Should trigger various address reloads */
        asm volatile (
            "movw $1, (%[flags], %[fidx], 2)\n\t"
            "movq (%[values], %[vidx], 8), %%r9\n\t"
            : 
            : [flags] "r" (&vblock->flags[0]),
              [fidx] "r" ((int64_t)flag_index),
              [values] "r" (&vblock->values[0]),
              [vidx] "r" ((int64_t)(j % 8))
            : "r9", "memory"
        );
        
        /* Force register shuffling */
        asm volatile (
            "xchgq %%r12, %%r13\n\t"
            "xchgq %%r14, %%r15\n\t"
            : 
            : 
            : "r12", "r13", "r14", "r15", "cc"
        );
    }
    
    /* Final complex pattern with all constraints mixed */
    {
        int64_t immediate = 0xDEADBEEF;
        int64_t* ptr_array[4] = {
            (int64_t*)&global_block[0],
            (int64_t*)&global_block[1],
            (int64_t*)&global_block[2],
            (int64_t*)&global_block[3]
        };
        
        /* This complex asm should trigger multiple reload types */
        asm volatile (
            "movq %[imm], %%r10\n\t"
            "movq %[idx], %%r11\n\t"
            "movq (%[arr], %%r11, 8), %%r12\n\t"
            "addq %%r10, (%%r12, %%r11, 8)\n\t"
            : 
            : [imm] "irm" (immediate),
              [idx] "r" (index_reg % 4),
              [arr] "r" (ptr_array)
            : "r10", "r11", "r12", "memory", "cc"
        );
    }
    
    return 0;
}
