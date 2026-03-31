/* reload1_trigger.c - Complex program to exercise GCC's reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Volatile structure to prevent optimization */
struct VolatileStruct {
    volatile int64_t a;
    volatile int32_t b;
    volatile int16_t c[8];
    volatile char d[32];
};

/* Global arrays to create addressing complexity */
static struct VolatileStruct global_array[16];
static int64_t global_matrix[8][8][8];
static volatile int32_t *volatile volatile_ptr_array[4];

/* Function with complex addressing patterns */
void __attribute__((noinline)) 
complex_addressing(int iterations) {
    /* Explicit register variables - force specific register allocation */
    register int64_t index_reg asm("r12") = 0;
    register int64_t base_reg asm("r13") = (int64_t)&global_array[0];
    register int64_t offset_reg asm("r14") = 8;
    register int64_t temp_reg asm("r15") = 0;
    
    /* Additional variables for mixed operand types */
    int64_t stack_array[16];
    volatile int64_t *volatile_ptr = &global_array[0].a;
    const int64_t immediate_const = 0x12345678ABCDEF00ULL;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        stack_array[i] = i * 3;
        global_array[i].a = i * 5;
        global_array[i].b = i * 7;
    }
    
    /* Complex loop with nested addressing computations */
    for (int i = 0; i < iterations; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Pattern 1: Memory operand with complex address computation */
        asm volatile (
            "movq (%[base], %[index], 8), %[temp]\n\t"
            "addq %[imm], %[temp]\n\t"
            "movq %[temp], (%[base], %[index], 8)"
            : [temp] "=&r" (temp_reg)
            : [base] "r" (base_reg),
              [index] "r" (index_reg),
              [imm] "irm" (immediate_const + i),
              "m" (*(struct VolatileStruct*)((char*)base_reg + index_reg * 8))
            : "memory", "cc"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        /* Pattern 2: Address of memory operand needs reload */
        int64_t* addr_of_mem;
        asm volatile (
            "lea (%[ptr], %[offset], 4), %[addr]\n\t"
            "movq (%[addr]), %[temp]"
            : [addr] "=&r" (addr_of_mem),
              [temp] "=&r" (temp_reg)
            : [ptr] "r" (volatile_ptr),
              [offset] "r" (offset_reg),
              "m" (*volatile_ptr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Pattern 3: Output with complex address */
        int64_t* output_addr;
        asm volatile (
            "movq %[val], (%[out_addr])\n\t"
            "incq %[out_addr]"
            : "=m" (*(int64_t*)output_addr),
              [out_addr] "+&r" (output_addr)
            : [val] "r" (temp_reg),
              "0" (*(int64_t*)output_addr)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* Pattern 4: Multiple address computations in one asm */
        int64_t addr1, addr2;
        asm volatile (
            "movq %%r12, %[a1]\n\t"
            "addq %%r13, %[a1]\n\t"
            "movq %%r14, %[a2]\n\t"
            "imulq $3, %[a2]"
            : [a1] "=&r" (addr1),
              [a2] "=&r" (addr2)
            : 
            : "r12", "r13", "r14", "cc"
        );
        
        /* Force RELOAD_FOR_OTHER_ADDRESS - complex case */
        /* Pattern 5: Multiple memory operands with different addressing modes */
        asm volatile (
            "movq (%[base1], %[idx1]), %%rax\n\t"
            "addq (%[base2], %[idx2], 8), %%rax\n\t"
            "movq %%rax, (%[base3])"
            : 
            : [base1] "r" (&stack_array[0]),
              [idx1] "r" (i & 7),
              [base2] "r" (&global_matrix[0][0][0]),
              [idx2] "r" ((i * 3) & 7),
              [base3] "r" (&global_array[i & 15].a),
              "m" (stack_array[i & 7]),
              "m" (global_matrix[i & 7][(i >> 3) & 7][(i >> 6) & 7]),
              "m" (global_array[i & 15].a)
            : "rax", "memory", "cc"
        );
        
        /* Mix in multi-dimensional array access with computed indices */
        int64_t idx_a = (i * 5) & 7;
        int64_t idx_b = (i * 3) & 7;
        int64_t idx_c = (i * 7) & 7;
        
        /* This creates complex address computation that may need RELOAD_FOR_OTHER_ADDRESS */
        volatile int64_t result = global_matrix[idx_a][idx_b][idx_c];
        
        /* Use explicit register variables in address computation */
        asm volatile (
            "movq (%[mat], %[idx_a], 64), %%rax\n\t"      /* 8*8 elements */
            "movq (%%rax, %[idx_b], 8), %%rax\n\t"        /* 8 elements */
            "addq (%%rax, %[idx_c], 8), %[reg]"
            : [reg] "+&r" (index_reg)
            : [mat] "r" (&global_matrix[0][0][0]),
              [idx_a] "r" (idx_a),
              [idx_b] "r" (idx_b),
              [idx_c] "r" (idx_c),
              "m" (global_matrix[idx_a][idx_b][idx_c])
            : "rax", "memory", "cc"
        );
        
        /* Update registers to create live range conflicts */
        offset_reg = (offset_reg * 3 + 1) & 31;
        base_reg = (int64_t)&global_array[(i + 1) & 15];
        
        /* Compiler barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Final use of register variables to keep them live */
    asm volatile (
        "addq %[reg1], %[reg2]"
        : [reg2] "+&r" (base_reg)
        : [reg1] "r" (index_reg)
        : "cc"
    );
}

/* Second function with different patterns */
void __attribute__((noinline, optimize("O0")))
mixed_operand_types(void) {
    /* Force various reload types with mixed constraints */
    register void* addr_reg asm("r10") = &global_array[4];
    register int64_t data_reg asm("r11") = 0xDEADBEEF;
    
    /* Immediate, register, and memory constraints mixed */
    for (int i = 0; i < 8; i++) {
        /* Pattern causing RELOAD_FOR_INPUT and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq %[imm], (%[addr], %[idx], 8)\n\t"
            "lock xaddq %[data], (%[addr])"
            : 
            : [imm] "irm" (0x1000 + i * 0x100),
              [addr] "r" (addr_reg),
              [idx] "r" ((int64_t)i),
              [data] "r" (data_reg),
              "m" (*(struct VolatileStruct*)((char*)addr_reg + i * 8)),
              "m" (*(struct VolatileStruct*)addr_reg)
            : "memory", "cc"
        );
        
        /* Complex address chain */
        void* chain_addr;
        asm volatile (
            "lea (%[base], %[idx], 4), %[chain]\n\t"
            "lea 16(%[chain]), %[chain]\n\t"
            "movq (%[chain]), %[data]"
            : [chain] "=&r" (chain_addr),
              [data] "=&r" (data_reg)
            : [base] "r" (addr_reg),
              [idx] "r" ((int64_t)i * 2),
              "m" (*(char*)addr_reg)
            : "memory"
        );
    }
}

/* Main function */
int main(int argc, char** argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Initialize volatile pointer array */
    for (int i = 0; i < 4; i++) {
        volatile_ptr_array[i] = &global_array[i].b;
    }
    
    /* Call functions with complex addressing */
    complex_addressing(iterations);
    mixed_operand_types();
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(global_array), "r"(global_matrix), "r"(volatile_ptr_array) : "memory");
    
    return (int)global_array[0].a;
}
