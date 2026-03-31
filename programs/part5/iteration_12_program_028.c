/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Force specific registers to be used */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Complex structure to force memory addressing */
struct ComplexStruct {
    uint64_t data[8];
    uint64_t *ptr_array[4];
    volatile uint64_t volatile_data;
    uint64_t padding[3];
};

/* Multi-dimensional array for nested addressing */
volatile uint64_t multi_array[4][8][16];

/* Global variables to prevent optimization */
volatile uint64_t global_counter = 0;
struct ComplexStruct global_structs[8];

int main(void) {
    /* Initialize register variables */
    reg_a = 0x12345678;
    reg_b = 0x87654321;
    reg_c = 0xABCDEF01;
    reg_d = 0xFEDCBA98;
    reg_e = 0x13579BDF;
    
    /* Local arrays with complex addressing requirements */
    uint64_t local_array[256];
    volatile uint64_t *volatile_ptr = (volatile uint64_t *)local_array;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].data[j] = i + j;
        }
        global_structs[i].volatile_data = i * 100;
    }
    
    /* Loop 1: Complex addressing with multiple reload types */
    for (uint64_t i = 0; i < 100; i++) {
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        uint64_t index1 = (reg_a + i) % 256;
        uint64_t index2 = (reg_b + i * 2) % 256;
        uint64_t index3 = (reg_c + i * 3) % 256;
        
        /* Complex inline assembly with memory constraints */
        asm volatile (
            /* Operation requiring address reloads */
            "movq (%[ptr1]), %%r15\n\t"
            "addq (%[ptr2]), %%r15\n\t"
            "movq %%r15, (%[ptr3])\n\t"
            /* Additional computation forcing register spills */
            "imulq %[imm], %%r15\n\t"
            "addq %%r15, %[out]"
            : [out] "+r" (reg_d)          /* Output operand in specific register */
            : [ptr1] "r" (&local_array[index1]),   /* Address needs reload */
              [ptr2] "r" (&local_array[index2]),   /* Another address reload */
              [ptr3] "r" (&local_array[index3]),   /* Third address reload */
              [imm] "i" (7)                        /* Immediate operand */
            : "r15", "memory", "cc"
        );
        
        /* Mix with explicit register variable usage */
        asm volatile (
            "addq %%r10, %%r11\n\t"
            "subq %%r12, %%r13"
            : /* no outputs */
            : /* no inputs */
            : "r10", "r11", "r12", "r13", "cc"
        );
    }
    
    /* Loop 2: Nested address computations with volatile accesses */
    for (int i = 0; i < 32; i++) {
        /* Complex index calculation using multiple registers */
        uint64_t idx = (reg_a * reg_b + reg_c * i) % 4;
        uint64_t idy = (reg_d ^ reg_e + i * 5) % 8;
        uint64_t idz = (reg_a + reg_b + reg_c + i) % 16;
        
        /* Access volatile multi-dimensional array - forces address reloads */
        uint64_t val = multi_array[idx][idy][idz];
        
        /* Inline assembly with mixed constraints */
        asm volatile (
            /* Operation requiring operand address reload */
            "leaq (%[base], %[idx], 8), %%rax\n\t"
            "movq (%%rax), %%rbx\n\t"
            "addq %[addend], %%rbx\n\t"
            "movq %%rbx, %[result]"
            : [result] "=rm" (val)        /* Can be register or memory */
            : [base] "r" (&multi_array[0][0][0]),  /* Base address */
              [idx] "r" (idx * 128 + idy * 16 + idz), /* Complex index */
              [addend] "irm" (global_counter)      /* Mixed: immediate, reg, or mem */
            : "rax", "rbx", "memory", "cc"
        );
        
        /* Force RELOAD_FOR_OPADDR_ADDR */
        asm volatile (
            "movq %[struct_ptr], %%rdi\n\t"
            "movq 32(%%rdi), %%rsi\n\t"    /* Access ptr_array */
            "movq (%%rsi, %[offset], 8), %%rdx\n\t"
            "addq %%rdx, %[accum]"
            : [accum] "+r" (reg_e)
            : [struct_ptr] "r" (&global_structs[idx]),
              [offset] "r" (idy)
            : "rdi", "rsi", "rdx", "memory", "cc"
        );
        
        global_counter++;
    }
    
    /* Loop 3: Stress test with all reload types */
    for (int outer = 0; outer < 10; outer++) {
        /* Use all register variables in complex expressions */
        uint64_t *base_ptr = local_array + (reg_a % 64);
        volatile uint64_t *vol_base = (volatile uint64_t *)base_ptr;
        
        /* Multiple inline asm statements with overlapping clobbers */
        for (int inner = 0; inner < 20; inner++) {
            uint64_t offset = (reg_b + reg_c * inner) & 0xFF;
            
            /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
            asm volatile (
                /* Input address reload */
                "movq (%[in_addr]), %%r8\n\t"
                /* Output address reload */
                "movq %%r8, (%[out_addr])\n\t"
                /* Additional computation */
                "addq %[inc], %%r8\n\t"
                "movq %%r8, %[temp]"
                : [temp] "=r" (reg_a)
                : [in_addr] "r" (vol_base + offset),   /* Input address */
                  [out_addr] "r" (&local_array[offset]), /* Output address */
                  [inc] "i" (1)                         /* Immediate */
                : "r8", "memory", "cc"
            );
            
            /* Force RELOAD_FOR_OPERAND_ADDRESS */
            asm volatile (
                "movq %[addr], %%r9\n\t"
                "lock xaddq %%r10, (%%r9)"
                : 
                : [addr] "r" (&global_counter)  /* Operand address */
                : "r9", "memory", "cc"
            );
        }
        
        /* Shuffle register variables to force spills */
        asm volatile (
            "xchgq %%r10, %%r11\n\t"
            "xchgq %%r12, %%r13\n\t"
            "xchgq %%r14, %%r15"
            : 
            : 
            : "r10", "r11", "r12", "r13", "r14", "r15", "cc"
        );
    }
    
    /* Final computation to use all values */
    uint64_t result = reg_a + reg_b + reg_c + reg_d + reg_e + global_counter;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(result));
    
    return (int)(result & 0x7FFFFFFF);
}
