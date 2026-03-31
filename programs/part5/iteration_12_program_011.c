/* reload_test.c - Complex addressing mode test for GCC reload pass */
#include <stdint.h>
#include <stdlib.h>

/* Force specific registers on x86-64 */
#ifdef __x86_64__
#define REG1 "r12"
#define REG2 "r13"
#define REG3 "r14"
#define REG4 "r15"
#else
/* Generic fallback for other architectures */
#define REG1 "r10"
#define REG2 "r11"
#define REG3 "r12"
#define REG4 "r13"
#endif

/* Complex structure to force memory addressing */
struct ComplexStruct {
    int64_t data[8];
    volatile int32_t volatile_field;
    struct ComplexStruct* next;
    double floating[4];
};

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[16];
static volatile int64_t volatile_buffer[256];

/* Function with complex addressing patterns */
void __attribute__((noinline))
test_reload_patterns(int iterations) {
    /* Explicit register variables - force register allocation conflicts */
    register int64_t index_reg asm(REG1) = 0;
    register struct ComplexStruct* base_ptr asm(REG2) = &global_array[0];
    register int64_t offset_reg asm(REG3) = 8;
    register volatile int64_t* volatile_ptr asm(REG4) = volatile_buffer;
    
    /* Additional variables for more complexity */
    int64_t temp_array[32];
    volatile int64_t* volatile volatile_array = volatile_buffer;
    
    /* Loop with nested addressing computations */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory addressing requiring RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            /* Use multiple memory operands with complex addressing */
            "movq (%[base], %[index], 8), %%rax\n\t"
            "addq %%rax, (%[vol_ptr], %[offset], 4)\n\t"
            /* Clobber many registers to force spills */
            :
            : [base] "r" (base_ptr),
              [index] "r" (index_reg),
              [vol_ptr] "r" (volatile_ptr),
              [offset] "r" (offset_reg)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "memory", "cc"
        );
        
        /* Pattern 2: Mixed operand types with immediate and memory */
        int64_t immediate_val = 0x12345678;
        asm volatile (
            /* Complex addressing with immediate offset */
            "leaq 0x1000(%[base], %[index], 2), %%r10\n\t"
            "movq (%%r10), %%r11\n\t"
            "imulq $0x7, %%r11, %%r11\n\t"
            "movq %%r11, %[temp]\n\t"
            : [temp] "=m" (temp_array[i & 31])
            : [base] "r" (base_ptr),
              [index] "r" (index_reg),
              [immed] "i" (immediate_val)
            : "r10", "r11", "memory"
        );
        
        /* Pattern 3: Nested address computation for RELOAD_FOR_INPUT_ADDRESS */
        struct ComplexStruct* nested_ptr = base_ptr + (index_reg & 7);
        asm volatile (
            /* Access through pointer with offset */
            "movq 16(%[nested]), %%rax\n\t"
            "addq %%rax, 24(%[nested])\n\t"
            /* Force address reload for the memory operand */
            :
            : [nested] "r" (nested_ptr),
              "m" (*(struct ComplexStruct(*)[8])nested_ptr)
            : "rax", "memory"
        );
        
        /* Pattern 4: Multiple output operands with conflicting constraints */
        int64_t out1, out2;
        asm volatile (
            /* Two outputs with different addressing modes */
            "movq (%[addr1]), %[out1]\n\t"
            "lea (%[addr2], %[idx], 4), %[out2]\n\t"
            : [out1] "=r" (out1),
              [out2] "=r" (out2)
            : [addr1] "r" (&volatile_array[index_reg & 255]),
              [addr2] "r" (temp_array),
              [idx] "r" (index_reg)
            : "memory"
        );
        
        /* Pattern 5: For RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        volatile int64_t* complex_addr = volatile_buffer + (index_reg * 3 + offset_reg) / 2;
        asm volatile (
            /* Operation requiring address computation for operand */
            "lock xaddq %%rax, (%[complex])\n\t"
            : 
            : [complex] "r" (complex_addr),
              "a" (1LL)
            : "memory", "cc"
        );
        
        /* Update registers to create varying addressing patterns */
        index_reg = (index_reg * 1664525 + 1013904223) & 0xFF;
        offset_reg = (offset_reg + 7) & 0xF;
        
        /* Pattern 6: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int64_t* inout_ptr = temp_array + (i & 15);
        asm volatile (
            /* Input and output with address computations */
            "movq (%[in]), %%r10\n\t"
            "notq %%r10\n\t"
            "movq %%r10, (%[out])\n\t"
            :
            : [in] "r" (inout_ptr),
              [out] "r" (inout_ptr + 16)
            : "r10", "memory"
        );
        
        /* Pattern 7: Force RELOAD_FOR_OUTPUT_ADDRESS */
        struct ComplexStruct* output_target = &global_array[(i * 3) % 16];
        asm volatile (
            /* Store with complex addressing */
            "movq %%r15, 32(%[target])\n\t"
            : 
            : [target] "r" (output_target),
              "r15" (index_reg)
            : "memory"
        );
        
        /* Access volatile structure through pointer with offset */
        volatile struct ComplexStruct* volatile_struct = 
            (volatile struct ComplexStruct*)&global_array[i % 16];
        volatile_struct->volatile_field = i;
        
        /* Complex loop-carried dependency */
        base_ptr = (struct ComplexStruct*)((char*)base_ptr + (index_reg & 3) * 64);
    }
}

/* Secondary function with different patterns */
void __attribute__((noinline))
test_other_reloads(void) {
    /* More explicit register variables */
    register double fp_reg asm("xmm0");
    register int64_t addr_reg asm("rbx");
    
    double fp_array[8];
    volatile double* volatile_fp = (volatile double*)fp_array;
    
    /* Pattern for RELOAD_FOR_OTHER_ADDRESS with floating point */
    for (int i = 0; i < 8; i++) {
        fp_reg = (double)i * 3.14159;
        addr_reg = (int64_t)&volatile_fp[i];
        
        asm volatile (
            /* Complex addressing with floating point */
            "movsd (%[addr]), %%xmm1\n\t"
            "addsd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, (%[addr])\n\t"
            :
            : [addr] "r" (addr_reg),
              "x" (fp_reg)
            : "xmm1", "memory"
        );
    }
}

/* Main function with setup and multiple test calls */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize data */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        global_array[i].floating[0] = (double)i;
        global_array[i].next = &global_array[(i + 1) % 16];
    }
    
    for (int i = 0; i < 256; i++) {
        volatile_buffer[i] = i * 2;
    }
    
    /* Call test functions multiple times with different parameters */
    test_reload_patterns(iterations);
    test_other_reloads();
    
    /* Additional loop with varying parameters */
    for (int scale = 1; scale <= 4; scale++) {
        register int scale_reg asm("r8") = scale;
        asm volatile (
            "/* Scale factor: %0 */\n\t"
            :
            : "r" (scale_reg)
        );
        
        test_reload_patterns(iterations / scale);
    }
    
    return 0;
}
