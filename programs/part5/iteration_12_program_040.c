/* reload_trigger.c
 * Designed to trigger specific reload types in GCC's reload1.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 reload_trigger.c -o reload_trigger
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[16];
    volatile int16_t volatile_data[32];
    char padding[7];
};

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[4];
static volatile int64_t volatile_global[100];

/* Explicit register variables - force specific register allocation */
register int64_t reg_var1 asm("r10");
register int64_t reg_var2 asm("r11");
register int64_t reg_var3 asm("r12");
register int64_t reg_var4 asm("r13");
register void* addr_reg asm("r14");

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables with different types */
    int64_t local_array[32];
    volatile int32_t* volatile_ptr = (volatile int32_t*)&global_array[0].volatile_data[0];
    struct ComplexStruct* struct_ptr = &global_array[1];
    
    /* Initialize explicit register variables */
    reg_var1 = 0x12345678;
    reg_var2 = 0x87654321;
    reg_var3 = (int64_t)&global_array[2];
    reg_var4 = 16;
    addr_reg = (void*)&volatile_global[0];
    
    /* Loop with complex addressing computations */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Complex memory addressing requiring RELOAD_FOR_OTHER_ADDRESS */
        /* This inline asm uses multiple memory operands with different addressing modes */
        asm volatile (
            /* Load from memory using complex addressing */
            "movq (%[base], %[idx], 8), %%r15\n\t"
            /* Store to another memory location with different addressing */
            "movq %%r15, (%[dest], %[offset], 4)\n\t"
            /* Use explicit register variable in address computation */
            "addq %%r12, %[dest]"
            : /* No outputs - side effects only */
            : [base] "r" (&global_array[0].data[0]),  /* Base address in register */
              [idx] "r" (i & 7),                     /* Index in register */
              [dest] "r" (&local_array[0]),          /* Destination base */
              [offset] "r" (reg_var4 & 3)           /* Offset from explicit register var */
            : "r15", "memory", "cc"
        );
        
        /* Pattern 2: Mixed operand types forcing RELOAD_FOR_INPUT_ADDRESS */
        /* Immediate, register, and memory constraints in one asm */
        int64_t immediate_val = 0xDEADBEEF;
        asm volatile (
            "leaq (%[imm], %[reg], 4), %%rax\n\t"
            "movq %%rax, (%[mem])"
            : 
            : [imm] "i" (256),                    /* Immediate constraint */
              [reg] "r" (reg_var1),               /* Register from explicit variable */
              [mem] "m" (*(volatile int64_t*)addr_reg)  /* Memory with volatile */
            : "rax", "memory", "cc"
        );
        
        /* Pattern 3: Nested address computation for RELOAD_FOR_INPADDR_ADDRESS */
        /* Compute address of a structure field with multiple components */
        int32_t index = (i * 3 + 1) & 15;
        asm volatile (
            /* Complex address calculation involving multiple registers */
            "movslq %[index], %%rbx\n\t"
            "leaq (%[struct], %%rbx, 4), %%rcx\n\t"
            "movl (%%rcx), %%edx\n\t"
            "addl %%edx, %[accum]"
            : [accum] "+r" (reg_var2)
            : [struct] "r" (&struct_ptr->indices[0]),
              [index] "r" (index)
            : "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Pattern 4: Force RELOAD_FOR_OPERAND_ADDRESS with volatile accesses */
        /* Multiple volatile accesses with address computations */
        volatile int16_t* vol_ptr = &global_array[i & 3].volatile_data[reg_var1 & 31];
        asm volatile (
            "movw %w[val], (%[ptr])"
            :
            : [ptr] "r" (vol_ptr),               /* Pointer in register */
              [val] "ri" (i & 0xFFFF)           /* Register or immediate */
            : "memory"
        );
        
        /* Pattern 5: Output address reloads (RELOAD_FOR_OUTPUT_ADDRESS) */
        int64_t output_buffer[4];
        asm volatile (
            /* Compute address and store result */
            "imulq $3, %[in], %%rax\n\t"
            "movq %%rax, (%[out])"
            :
            : [in] "r" (reg_var3),              /* Input in register */
              [out] "r" (&output_buffer[i & 3]) /* Output address in register */
            : "rax", "memory", "cc"
        );
        
        /* Pattern 6: RELOAD_FOR_OPADDR_ADDR with explicit register clobbering */
        /* Use and clobber the explicit register variables */
        asm volatile (
            "xchgq %%r10, %%r11\n\t"    /* Swap reg_var1 and reg_var2 */
            "addq %%r12, %%r10\n\t"     /* Add reg_var3 to reg_var1 */
            "subq $8, %%r13"            /* Modify reg_var4 */
            : 
            : 
            : "r10", "r11", "r12", "r13", "cc"
        );
        
        /* Pattern 7: RELOAD_FOR_OTHER_ADDRESS with complex constraints */
        /* Memory operand with multiple alternative constraints */
        int64_t temp;
        asm volatile (
            "movq %[addr], %%r8\n\t"
            "movq (%%r8), %%r9\n\t"
            "addq %%r9, %[result]"
            : [result] "+r" (temp)
            : [addr] "irm" ((int64_t)&global_array[0].data[i & 7])  /* Immediate, reg, or memory */
            : "r8", "r9", "memory", "cc"
        );
        
        /* Pattern 8: Force RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output operand with memory constraint and complex addressing */
        int64_t* out_addr = &local_array[(i + reg_var4) & 31];
        asm volatile (
            "movq %[in], %%r8\n\t"
            "movq %%r8, %[out]"
            : [out] "=m" (*out_addr)    /* Memory output with computed address */
            : [in] "r" (reg_var2)       /* Input from explicit register */
            : "r8", "memory"
        );
        
        /* Modify explicit register variables to change addressing patterns */
        reg_var1 += i;
        reg_var3 += 8;
        reg_var4 = (reg_var4 * 5 + 1) & 31;
        
        /* Access volatile memory through computed address */
        volatile_global[(reg_var1 + i) % 100] = reg_var2;
    }
    
    /* Final complex pattern mixing all addressing modes */
    struct ComplexStruct* final_ptr = &global_array[3];
    asm volatile (
        /* Multiple memory accesses with different addressing */
        "movq 0(%[ptr]), %%rax\n\t"
        "movq 64(%[ptr]), %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movq %%rax, 128(%[ptr])\n\t"
        /* Use explicit register in address computation */
        "leaq (%[ptr], %[reg], 2), %%rcx\n\t"
        "movq (%%rcx), %%rdx"
        :
        : [ptr] "r" (final_ptr),
          [reg] "r" (reg_var1)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
}

/* Helper function to create register pressure */
void create_register_pressure(int64_t a, int64_t b, int64_t c, int64_t d,
                              int64_t e, int64_t f, int64_t g, int64_t h) {
    /* Use all parameters in complex addressing */
    volatile int64_t* ptrs[] = {
        (volatile int64_t*)&global_array[0].data[a & 7],
        (volatile int64_t*)&global_array[1].data[b & 7],
        (volatile int64_t*)&global_array[2].data[c & 7],
        (volatile int64_t*)&global_array[3].data[d & 7]
    };
    
    for (int i = 0; i < 4; i++) {
        asm volatile (
            "movq %[val], (%[ptr])"
            :
            : [ptr] "r" (ptrs[i]),
              [val] "r" (e + f + g + h)
            : "memory"
        );
    }
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 32; j++) {
            global_array[i].volatile_data[j] = (i + j) * 2;
        }
    }
    
    /* Call functions with complex parameter passing to force reloads */
    trigger_reloads(10);
    
    /* Create additional register pressure */
    create_register_pressure(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* More complex patterns in main */
    register void* r15_var asm("r15");
    r15_var = &volatile_global[50];
    
    /* Pattern requiring RELOAD_FOR_OTHER_ADDRESS */
    asm volatile (
        "movq (%[base], %[index], 8), %%rax\n\t"
        "addq $1, %%rax\n\t"
        "movq %%rax, (%[dest])"
        :
        : [base] "r" (&global_array[0].data[0]),
          [index] "r" (reg_var1 & 7),
          [dest] "r" (r15_var)
        : "rax", "memory", "cc"
    );
    
    return 0;
}
