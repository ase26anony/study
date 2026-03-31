/* reload1_trigger.c
 * Designed to trigger GCC's reload pass logic for various address reload types
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload1_trigger.c -o reload1_trigger
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force non-trivial addressing */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[4];
    void* pointers[2];
    volatile int32_t volatile_field;
};

/* Explicit register variables - will force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[16];
static volatile int64_t volatile_buffer[256];

/* Function with complex addressing modes */
void complex_addressing_loop(int iterations) {
    /* Local variables that will conflict with explicit registers */
    int64_t local_base = (int64_t)&global_array[0];
    int64_t local_offset = 0;
    int64_t local_index = 0;
    
    /* Mix of immediate, register, and memory operands */
    const int64_t immediate_const = 0x12345678;
    int64_t* dynamic_ptr = (int64_t*)malloc(32 * sizeof(int64_t));
    
    /* Force register pressure by using many variables */
    int64_t temp1, temp2, temp3, temp4;
    
    /* Loop with nested address computations */
    for (int i = 0; i < iterations; i++) {
        /* Complex address computation involving multiple registers */
        int64_t array_index = (reg_a + i * 16) % 16;
        int64_t struct_offset = (reg_b + i * 32) % 256;
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS
         * Memory constraint with register-based address calculation
         */
        asm volatile (
            "movq (%[addr]), %[temp]\n\t"
            "addq %[imm], %[temp]\n\t"
            "movq %[temp], (%[addr2])"
            : [temp] "=&r" (temp1)
            : [addr] "r" (&volatile_buffer[array_index]), 
              [imm] "irm" (immediate_const),
              [addr2] "r" (&global_array[struct_offset / 32].volatile_field)
            : "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS
         * Multiple memory operands with different base registers
         */
        struct ComplexStruct* ptr1 = &global_array[i % 8];
        struct ComplexStruct* ptr2 = &global_array[(i + 1) % 8];
        
        asm volatile (
            "leaq (%[base], %[idx], 8), %[temp]\n\t"
            "movq (%[ptr]), %[temp2]\n\t"
            "addq %[temp2], (%[temp])"
            : [temp] "=&r" (temp2), [temp2] "=&r" (temp3)
            : [base] "r" (ptr1), 
              [idx] "r" (reg_c),
              [ptr] "m" (ptr2->pointers[0])
            : "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
         * Mixed constraints with explicit register clobbering
         */
        asm volatile (
            "movq %[val1], %%r10\n\t"  /* Clobbers our explicit register */
            "movq %[val2], %%r11\n\t"  /* Clobbers another explicit register */
            "imulq %[val3], %%r10\n\t"
            "addq %%r10, %[out]"
            : [out] "+rm" (temp4)
            : [val1] "irm" (reg_d),
              [val2] "irm" (reg_e),
              [val3] "irm" (i)
            : "r10", "r11", "cc"
        );
        
        /* Pattern 4: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS
         * Output memory operand with complex address
         */
        int64_t* output_addr = &dynamic_ptr[(reg_a + reg_b) % 32];
        
        asm volatile (
            "movq %[in], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[out]"
            : [out] "=m" (*output_addr)
            : [in] "irm" (local_index)
            : "rax", "memory"
        );
        
        /* Update explicit register variables - forces spills/reloads */
        asm volatile (
            "addq %[inc], %[reg]\n\t"
            "subq $1, %[reg2]"
            : [reg] "+r" (reg_a), [reg2] "+r" (reg_b)
            : [inc] "irm" (i)
            : "cc"
        );
        
        /* Complex array access with multiple index calculations */
        local_index = (reg_c * 3 + reg_d * 7 + i) % 128;
        int64_t* volatile_ptr = &volatile_buffer[local_index];
        
        /* Pattern 5: Multiple address reloads in one statement */
        asm volatile (
            "movq (%[src]), %%rax\n\t"
            "addq (%[src2]), %%rax\n\t"
            "movq %%rax, (%[dst])\n\t"
            "addq %%rax, (%[dst2])"
            : 
            : [src] "r" (volatile_ptr),
              [src2] "r" (&global_array[i % 4].data[local_index % 8]),
              [dst] "r" (&global_array[(i + 2) % 4].volatile_field),
              [dst2] "r" (dynamic_ptr)
            : "rax", "memory", "cc"
        );
        
        /* Rotate explicit registers to increase pressure */
        asm volatile (
            "xchgq %[r1], %[r2]\n\t"
            "xchgq %[r3], %[r4]"
            : [r1] "+r" (reg_c), [r2] "+r" (reg_d), [r3] "+r" (reg_a), [r4] "+r" (reg_e)
            :
            : "cc"
        );
    }
    
    free(dynamic_ptr);
}

/* Secondary function with different addressing patterns */
void nested_addressing(int depth) {
    if (depth <= 0) return;
    
    /* Multi-dimensional array access */
    int64_t md_array[4][8][16];
    static int64_t static_array[64];
    
    /* Complex index calculation using explicit registers */
    int64_t idx1 = (reg_a * depth) % 4;
    int64_t idx2 = (reg_b * depth * 2) % 8;
    int64_t idx3 = (reg_c * depth * 3) % 16;
    
    /* Pattern: Deeply nested address computation */
    asm volatile (
        "movq (%[base], %[i1], 128), %%rax\n\t"      /* idx1 * 128 */
        "leaq (%%rax, %[i2], 16), %%rbx\n\t"         /* + idx2 * 16 */
        "leaq (%%rbx, %[i3], 8), %%rcx\n\t"          /* + idx3 * 8 */
        "movq (%%rcx), %[result]"
        : [result] "=r" (md_array[idx1][idx2][idx3])
        : [base] "r" (md_array),
          [i1] "r" (idx1),
          [i2] "r" (idx2),
          [i3] "r" (idx3)
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Recursive call with modified registers */
    asm volatile ("addq $1, %0" : "+r" (reg_a));
    nested_addressing(depth - 1);
}

int main() {
    /* Initialize explicit register variables */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        global_array[i].volatile_field = i;
    }
    
    /* Trigger complex addressing patterns */
    complex_addressing_loop(100);
    
    /* Trigger nested addressing patterns */
    nested_addressing(10);
    
    /* Final mixed pattern with all reload types */
    volatile int64_t* final_ptr = &volatile_buffer[0];
    
    asm volatile (
        /* Multiple address calculations in sequence */
        "movq %[reg1], %%rax\n\t"
        "leaq (%[base], %%rax, 8), %%rbx\n\t"    /* RELOAD_FOR_INPUT_ADDRESS */
        "movq %[reg2], %%rcx\n\t"
        "leaq (%[base2], %%rcx, 4), %%rdx\n\t"   /* RELOAD_FOR_OTHER_ADDRESS */
        "movq (%%rbx), %%rsi\n\t"
        "addq (%%rdx), %%rsi\n\t"
        "movq %%rsi, %[out1]\n\t"
        "leaq (%[outptr], %%rax, 1), %%rdi\n\t"  /* RELOAD_FOR_OUTPUT_ADDRESS */
        "movq %%rsi, (%%rdi)"
        : [out1] "=rm" (volatile_buffer[0])
        : [reg1] "irm" (reg_a),
          [reg2] "irm" (reg_b),
          [base] "r" (global_array),
          [base2] "r" (&global_array[8]),
          [outptr] "r" (final_ptr)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
    );
    
    return 0;
}
