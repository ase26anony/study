/* reload_test.c - Complex addressing mode test to trigger reload types */
#include <stdint.h>
#include <stdlib.h>

/* Force specific register allocations */
register uint64_t reg_a asm("r10");
register uint64_t reg_b asm("r11");
register uint64_t reg_c asm("r12");
register uint64_t reg_d asm("r13");
register uint64_t reg_e asm("r14");

/* Volatile structures to prevent optimization */
volatile struct ComplexStruct {
    uint64_t data[8];
    volatile uint64_t* ptr_array[4];
    volatile uint64_t counter;
} g_struct;

/* Multi-dimensional array with volatile elements */
volatile uint64_t g_multi_array[4][8][16];

/* Function with complex addressing patterns */
void complex_addressing_test(uint64_t iter_count) {
    /* Local variables bound to registers */
    register uint64_t idx1 asm("r15") = 0;
    register uint64_t idx2 asm("rbx") = 0;
    register uint64_t base_addr asm("rbp") = (uint64_t)&g_struct;
    
    /* Additional pressure variables */
    uint64_t temp1, temp2, temp3;
    volatile uint64_t* volatile_ptr = &g_struct.counter;
    
    /* Loop with nested address computations */
    for (uint64_t i = 0; i < iter_count; i++) {
        /* Complex computation involving multiple registers */
        idx1 = (i * 7) & 0xF;
        idx2 = (i * 3) & 0x7;
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS */
        /* Multiple memory operands with register constraints */
        asm volatile (
            "movq %[base], %%r8\n\t"
            "addq %[idx1], %%r8\n\t"
            "movq (%%r8), %[out1]\n\t"
            "leaq (%[idx2], %[idx1], 8), %%r9\n\t"
            "addq %%r9, %[out2]"
            : [out1] "=r" (temp1), [out2] "+r" (temp2)
            : [base] "r" (base_addr), [idx1] "r" (idx1), [idx2] "r" (idx2)
            : "r8", "r9", "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* Memory operand with complex addressing mode */
        asm volatile (
            "movq %[ptr], %%r8\n\t"
            "movq (%%r8, %[idx1], 8), %%r9\n\t"
            "addq %%r9, %[out]"
            : [out] "+r" (temp3)
            : [ptr] "r" (volatile_ptr), [idx1] "r" (idx1)
            : "r8", "r9", "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OPERAND_ADDRESS */
        /* Mixed immediate, register, and memory constraints */
        uint64_t immediate_val = 0x12345678;
        asm volatile (
            "imulq %[imm], %[reg]\n\t"
            "addq %[mem], %[reg]"
            : [reg] "+r" (reg_a)
            : [imm] "i" (0x1000), [mem] "m" (*(volatile uint64_t*)(base_addr + idx1))
            : "cc", "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output memory operand with address computation */
        volatile uint64_t output_loc;
        asm volatile (
            "movq %[val], %%r8\n\t"
            "movq %%r8, %[out]\n\t"
            "leaq 0x100(%%r8, %[idx2], 4), %%r9"
            : [out] "=m" (output_loc)
            : [val] "r" (reg_b), [idx2] "r" (idx2)
            : "r8", "r9", "memory"
        );
        
        /* Pattern 5: Complex multi-dimensional array access */
        /* Forces RELOAD_FOR_OPADDR_ADDR and other reloads */
        uint64_t* volatile array_ptr = (uint64_t*)&g_multi_array[0][0][0];
        uint64_t offset = (idx1 * 128) + (idx2 * 16) + i;
        
        asm volatile (
            "movq %[array], %%r8\n\t"
            "movq %[offset], %%r9\n\t"
            "movq (%%r8, %%r9, 8), %%r10\n\t"
            "addq %%r10, %[sum]"
            : [sum] "+r" (reg_c)
            : [array] "r" (array_ptr), [offset] "r" (offset)
            : "r8", "r9", "r10", "memory"
        );
        
        /* Use explicit register variables in conflicting ways */
        asm volatile (
            "xchgq %[regd], %[rege]"
            : [regd] "+r" (reg_d), [rege] "+r" (reg_e)
            :
            : "cc"
        );
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

/* Secondary function with different patterns */
void secondary_test(uint64_t n) {
    /* Struct with pointer members */
    struct Nested {
        volatile uint64_t* ptr1;
        volatile uint64_t* ptr2;
        uint64_t data[4];
    } nested;
    
    nested.ptr1 = &g_struct.counter;
    nested.ptr2 = (volatile uint64_t*)&g_multi_array;
    
    register uint64_t r1 asm("r10");
    register uint64_t r2 asm("r11");
    
    for (uint64_t j = 0; j < n; j++) {
        /* Pattern for RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq %[p1], %%r8\n\t"
            "movq (%%r8), %%r9\n\t"
            "movq %[p2], %%r10\n\t"
            "addq (%%r10, %[idx], 8), %%r9\n\t"
            "movq %%r9, %[out]"
            : [out] "=r" (r1)
            : [p1] "r" (nested.ptr1), [p2] "r" (nested.ptr2), 
              [idx] "r" (j & 3)
            : "r8", "r9", "r10", "memory"
        );
        
        /* Mixed constraints causing various reloads */
        uint64_t immediate = 0xABCD;
        asm volatile (
            "lea (%[imm], %[reg], 4), %[reg]"
            : [reg] "+r" (r2)
            : [imm] "i" (0x1000)
            : "cc"
        );
    }
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 8; i++) {
        g_struct.data[i] = i * 0x100;
    }
    g_struct.counter = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                g_multi_array[i][j][k] = (i << 16) | (j << 8) | k;
            }
        }
    }
    
    /* Initialize register variables */
    reg_a = 0x11111111;
    reg_b = 0x22222222;
    reg_c = 0x33333333;
    reg_d = 0x44444444;
    reg_e = 0x55555555;
    
    /* Run tests with moderate iteration counts */
    complex_addressing_test(100);
    secondary_test(50);
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    /* Use results to prevent dead code elimination */
    uint64_t result = reg_a + reg_b + reg_c + reg_d + reg_e;
    
    /* Return something based on the result */
    return (result > 0) ? 0 : 1;
}
