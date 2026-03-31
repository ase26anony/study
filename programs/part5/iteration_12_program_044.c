/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload1.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload1_trigger.c
 */

#include <stdint.h>

/* Complex structure to force non-trivial addressing */
struct ComplexStruct {
    int64_t data[8];
    int32_t indices[4];
    volatile int16_t volatile_data[16];
    char padding[7];
} __attribute__((packed, aligned(32)));

/* Global arrays to create addressing pressure */
static struct ComplexStruct global_array[32];
static volatile int64_t volatile_buffer[256];

/* Explicit register variables - force specific register allocation */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");
register int64_t reg_f asm("r15");

/* Function with complex addressing modes */
void __attribute__((noinline)) 
complex_addressing(int iterations, int offset, int stride) {
    /* Local variables that will compete for registers */
    int64_t local_acc = 0;
    int64_t local_idx = offset;
    volatile int64_t* volatile_ptr = volatile_buffer;
    
    /* Multiple explicit register variables in use */
    reg_a = (int64_t)&global_array[0];
    reg_b = (int64_t)&volatile_buffer[0];
    reg_c = stride * 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex address computation involving multiple registers */
        int64_t base_addr = reg_a + (local_idx * 64);
        int64_t offset_addr = reg_b + (i * reg_c);
        
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
        /* Inline asm with memory constraint requiring address reload */
        asm volatile (
            "movq (%[mem1]), %[tmp]\n\t"
            "addq %[reg1], %[tmp]\n\t"
            "movq %[tmp], (%[mem2])"
            : [tmp] "=&r" (local_acc)
            : [mem1] "m" (*(struct ComplexStruct*)(base_addr)),
              [mem2] "m" (*(volatile int64_t*)(offset_addr)),
              [reg1] "r" (reg_c)
            : "memory"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        /* Multiple memory operands with different addressing requirements */
        int64_t* ptr1 = (int64_t*)(base_addr + 32);
        volatile int16_t* ptr2 = (volatile int16_t*)(offset_addr + 16);
        
        asm volatile (
            "movzwq (%[ptr2]), %%rax\n\t"
            "addq (%[ptr1]), %%rax\n\t"
            "movq %%rax, %[out]"
            : [out] "=r" (reg_d)
            : [ptr1] "r" (ptr1),
              [ptr2] "r" (ptr2),
              "m" (*(int64_t*)ptr1),
              "m" (*(volatile int16_t*)ptr2)
            : "rax", "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Output memory operand with complex addressing */
        int64_t output_addr = reg_b + (local_idx * 8) + 128;
        
        asm volatile (
            "leaq (%[in1], %[in2], 4), %%rax\n\t"
            "movq %%rax, (%[out_addr])"
            : 
            : [in1] "r" (reg_d),
              [in2] "r" (reg_e),
              [out_addr] "r" (output_addr),
              "m" (*(volatile int64_t*)output_addr)
            : "rax", "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPADDR_ADDR */
        /* Complex operand address computation */
        struct ComplexStruct* cs_ptr = (struct ComplexStruct*)reg_a;
        int32_t index = cs_ptr->indices[i & 3];
        
        asm volatile (
            "imulq $7, %[idx], %[idx]\n\t"
            "addq %[idx], %[base]"
            : [base] "+r" (reg_a),
              [idx] "+r" (index)
            : 
            : "cc"
        );
        
        /* Mix register usage to force spills and reloads */
        reg_e = reg_d + local_acc;
        reg_f = reg_a ^ reg_b;
        
        /* Update loop variables with complex expressions */
        local_idx = (local_idx * 13 + 7) & 0xFF;
        volatile_ptr += stride;
        
        /* Additional memory access with volatile to prevent optimization */
        asm volatile("" : : "m" (*volatile_ptr) : "memory");
    }
    
    /* Final use of all register variables */
    asm volatile (
        "addq %[a], %[b]\n\t"
        "addq %[c], %[d]\n\t"
        "addq %[e], %[f]"
        : 
        : [a] "r" (reg_a),
          [b] "r" (reg_b),
          [c] "r" (reg_c),
          [d] "r" (reg_d),
          [e] "r" (reg_e),
          [f] "r" (reg_f)
        : "cc"
    );
}

/* Second function with different addressing patterns */
void __attribute__((noinline, optimize("O0")))
mixed_operand_types(int count) {
    /* Immediate, register, and memory operands mixed */
    int64_t array[16];
    volatile int64_t v_array[16];
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        array[i] = i * i;
        v_array[i] = i * 3;
    }
    
    /* Use explicit register variables */
    register int64_t r1 asm("r10") = (int64_t)array;
    register int64_t r2 asm("r11") = (int64_t)v_array;
    
    for (int i = 0; i < count; i++) {
        /* Pattern requiring RELOAD_FOR_OTHER_ADDRESS */
        /* Memory operand with index and base register */
        int64_t offset = i * 8;
        
        asm volatile (
            "movq (%[base], %[offset]), %%rax\n\t"
            "addq $0x12345678, %%rax\n\t"
            "movq %%rax, (%[vbase], %[offset], 2)"
            : 
            : [base] "r" (r1),
              [offset] "r" (offset),
              [vbase] "r" (r2),
              "m" (*(int64_t*)(r1 + offset)),
              "m" (*(volatile int64_t*)(r2 + offset * 2))
            : "rax", "memory", "cc"
        );
        
        /* Complex addressing with scaled index */
        int64_t scale = (i & 3) + 1;
        
        asm volatile (
            "imulq %[scale], %[offset]\n\t"
            "movq (%[base], %[offset]), %%rbx"
            : 
            : [base] "r" (r1),
              [offset] "r" (offset),
              [scale] "r" (scale),
              "m" (*(int64_t*)(r1 + offset * scale))
            : "rbx", "cc"
        );
        
        /* Update register variables with complex expressions */
        r1 += 8;
        r2 -= 8;
    }
}

/* Main function to drive everything */
int main() {
    /* Initialize global data */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            global_array[i].data[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            global_array[i].indices[j] = (i + j) * 2;
        }
    }
    
    /* Call functions with different parameters to trigger various reloads */
    complex_addressing(100, 5, 3);
    mixed_operand_types(50);
    
    /* Additional loop with different stride */
    complex_addressing(50, 10, 7);
    
    return 0;
}
