/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass:
 * - RELOAD_FOR_OTHER_ADDRESS
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OPERAND_ADDRESS
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[16];
    volatile int16_t volatile_data[32];
    char padding[7];
} __attribute__((packed, aligned(32)));

/* Global arrays to create addressing pressure */
static struct ComplexStruct global_array[4];
static volatile int64_t volatile_global[100];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing patterns */
void complex_addressing_loop(int iterations) {
    /* Local arrays with different alignments */
    struct ComplexStruct local_array[8] __attribute__((aligned(64)));
    int64_t dynamic_indices[16];
    volatile int32_t* volatile_ptr = (volatile int32_t*)&global_array[0].volatile_data[0];
    
    /* Initialize some data */
    for (int i = 0; i < 16; i++) {
        dynamic_indices[i] = (i * 7) % 16;
    }
    
    /* Force register variables to have values */
    reg_a = (int64_t)&local_array[0];
    reg_b = (int64_t)&global_array[0];
    reg_c = (int64_t)dynamic_indices;
    reg_d = iterations;
    reg_e = 0;
    reg_f = 0x12345678;
    
    /* Main loop with complex addressing */
    for (int i = 0; i < iterations; i++) {
        int64_t idx1, idx2, idx3;
        int64_t* addr1;
        volatile int16_t* addr2;
        
        /* Complex index calculations using register variables */
        asm volatile (
            "lea (%[base], %[idx], 8), %[out1]\n\t"
            "mov %[regc], %[out2]\n\t"
            "add %[rege], %[out2]\n\t"
            : [out1] "=r" (idx1), [out2] "=r" (idx2)
            : [base] "r" (reg_a), [idx] "r" (i), 
              [regc] "r" (reg_c), [rege] "r" (reg_e)
            : "cc"
        );
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        /* Mixed constraints: register, memory, and immediate */
        asm volatile (
            "mov (%[mem1]), %[tmp]\n\t"
            "add %[imm], %[tmp]\n\t"
            "mov %[tmp], (%[mem2])\n\t"
            : [tmp] "=&r" (idx3), [mem2] "+m" (*(volatile int64_t*)(reg_b + idx1))
            : [mem1] "m" (*(struct ComplexStruct*)(reg_a + i * sizeof(struct ComplexStruct))),
              [imm] "i" (42), "r" (reg_d)
            : "cc", "memory"
        );
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        /* Complex memory operand with address computation */
        addr1 = (int64_t*)((char*)reg_b + idx2 * 2);
        asm volatile (
            "movq $0xABCD, (%[addr])\n\t"
            : 
            : [addr] "r" (addr1), "m" (*addr1)
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OPERAND_ADDRESS */
        /* Multiple memory operands with different addressing modes */
        addr2 = (volatile int16_t*)(reg_b + idx3 % 32);
        asm volatile (
            "movw %w[val], (%[addr2])\n\t"
            "movzwq (%[addr2]), %[out]\n\t"
            : [out] "=r" (reg_e)
            : [addr2] "r" (addr2), [val] "ri" (i & 0xFFFF),
              "m" (*addr2), "m" (*(volatile int16_t*)(reg_b + 16))
            : "cc"
        );
        
        /* Nested addressing with multiple reload types */
        /* This should trigger RELOAD_FOR_OTHER_ADDRESS */
        {
            int64_t offset = i * 7 + reg_f;
            struct ComplexStruct* ptr = (struct ComplexStruct*)(reg_a + offset);
            
            asm volatile (
                "movq 32(%[ptr]), %%rax\n\t"
                "addq 64(%[ptr]), %%rax\n\t"
                "movq %%rax, %[result]\n\t"
                : [result] "=r" (reg_f)
                : [ptr] "r" (ptr), 
                  "m" (ptr->data[0]), "m" (ptr->data[4]),
                  "m" (ptr->indices[0]), "m" (ptr->volatile_data[0])
                : "rax", "cc", "memory"
            );
        }
        
        /* More complex pattern with immediate displacement */
        asm volatile (
            "imul $37, %[idx], %[tmp]\n\t"
            "add %[regb], %[tmp]\n\t"
            "movq $99, (%[tmp])\n\t"
            : [tmp] "=&r" (idx3)
            : [idx] "r" (i), [regb] "r" (reg_b)
            : "cc", "memory"
        );
        
        /* Access volatile global with complex addressing */
        volatile_global[(i + reg_e) % 100] = reg_f;
        
        /* Update register variables to force spills/reloads */
        asm volatile (
            "add $1, %[rega]\n\t"
            "sub $2, %[regb]\n\t"
            "xor %[regc], %[regc]\n\t"
            : [rega] "+r" (reg_a), [regb] "+r" (reg_b), [regc] "+r" (reg_c)
            : 
            : "cc"
        );
    }
}

/* Second function with different addressing patterns */
void mixed_operand_types(void) {
    int64_t temp_array[32] __attribute__((aligned(128)));
    volatile int64_t* volatile temp_ptr = (volatile int64_t*)temp_array;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        temp_array[i] = i * i;
    }
    
    /* Force various reload types with mixed constraints */
    for (int i = 0; i < 16; i++) {
        int64_t result;
        int64_t* ptr1 = &temp_array[i];
        int64_t* ptr2 = &temp_array[31 - i];
        
        /* Pattern for RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "mov (%[in1]), %%rax\n\t"
            "add (%[in2]), %%rax\n\t"
            "mov %%rax, (%[out])\n\t"
            : "=m" (*ptr1)
            : [in1] "r" (ptr1), [in2] "r" (ptr2),
              [out] "r" (ptr1), "m" (*ptr1), "m" (*ptr2)
            : "rax", "cc", "memory"
        );
        
        /* Complex immediate + register + memory pattern */
        asm volatile (
            "lea (%[base], %[index], 8), %[result]\n\t"
            "mov (%[result]), %[result]\n\t"
            "add $0x7FFF, %[result]\n\t"
            : [result] "=r" (result)
            : [base] "r" (temp_array), [index] "irm" (i * 2),
              "m" (temp_array[i * 2])
            : "cc"
        );
        
        /* Store result back with volatile access */
        *volatile_temp_ptr = result;
        volatile_temp_ptr++;
    }
}

/* Main function that sets up and calls the complex patterns */
int main(int argc, char** argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 32; j++) {
            global_array[i].volatile_data[j] = (i + j) * 3;
        }
    }
    
    /* Call functions with complex addressing */
    complex_addressing_loop(iterations);
    mixed_operand_types();
    
    /* Final volatile store to prevent dead code elimination */
    volatile_global[0] = reg_a + reg_b + reg_c + reg_d + reg_e + reg_f;
    
    return 0;
}
