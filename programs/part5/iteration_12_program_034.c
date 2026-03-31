/* reload1_trigger.c
 * Designed to trigger specific reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 -c reload1_trigger.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex structure to force various addressing modes */
struct ComplexStruct {
    int64_t a;
    int32_t b[4];
    volatile int16_t c[8];
    char d[32];
};

/* Global arrays to create addressing complexity */
static struct ComplexStruct global_array[16];
static volatile int64_t volatile_buffer[256];

/* Explicit register variables - force specific register usage */
register int64_t reg_a asm("r10");
register int64_t reg_b asm("r11");
register int64_t reg_c asm("r12");
register int64_t reg_d asm("r13");
register int64_t reg_e asm("r14");

/* Function with complex addressing patterns */
void trigger_reloads(int iterations) {
    /* Local variables with different storage classes */
    int64_t stack_var1 = 0x12345678;
    int64_t stack_var2 = 0x87654321;
    volatile int64_t volatile_var = 0;
    struct ComplexStruct local_struct;
    
    /* Initialize local struct */
    for (int i = 0; i < 4; i++) {
        local_struct.b[i] = i * 100;
    }
    for (int i = 0; i < 8; i++) {
        local_struct.c[i] = i * 10;
    }
    
    /* Initialize explicit register variables */
    reg_a = (int64_t)&global_array[0];
    reg_b = (int64_t)&volatile_buffer[0];
    reg_c = (int64_t)&local_struct;
    reg_d = stack_var1;
    reg_e = stack_var2;
    
    /* Main loop with complex addressing patterns */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
        /* Complex inline asm with memory operand requiring address reload */
        asm volatile (
            /* Operation using multiple registers and memory */
            "add %[mem1], %[reg1], %[reg2]\n\t"
            "sub %[reg3], %[reg4], %[mem2]\n\t"
            /* Force address computation for memory operand */
            : [reg1] "+r" (reg_a), [reg2] "+r" (reg_b), [reg3] "=&r" (reg_c)
            : [mem1] "m" (global_array[i % 16].b[0]), 
              [mem2] "m" (*(int64_t*)((char*)&local_struct + (i % 32))),
              [reg4] "r" (reg_d)
            : "memory", "cc"
        );
        
        /* Pattern 2: RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OPERAND_ADDRESS */
        /* Multiple memory operands with different addressing requirements */
        int64_t temp1, temp2;
        asm volatile (
            /* Complex addressing with offset computation */
            "mov %[out1], [%[base1], %[idx1], lsl #3]\n\t"
            "mov %[out2], [%[base2], %[idx2], lsl #2]\n\t"
            : [out1] "=r" (temp1), [out2] "=r" (temp2)
            : [base1] "r" (reg_a), [idx1] "r" (i),
              [base2] "r" (reg_b), [idx2] "r" (i * 2),
              "m" (global_array[0]), "m" (volatile_buffer[0])
            : "memory"
        );
        
        /* Pattern 3: RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        /* Store operations with complex address computation */
        int64_t computed_addr = (int64_t)&volatile_buffer[i % 256];
        int64_t computed_offset = i * sizeof(int64_t);
        
        asm volatile (
            /* Store with computed address */
            "str %[val], [%[addr], %[offset]]\n\t"
            /* Another store with different addressing */
            "str %[val2], [%[base], %[idx], lsl #3]\n\t"
            : 
            : [val] "r" (reg_a), [addr] "r" (reg_b), [offset] "r" (computed_offset),
              [val2] "r" (reg_c), [base] "r" (reg_d), [idx] "r" (i)
            : "memory"
        );
        
        /* Pattern 4: RELOAD_FOR_OPADDR_ADDR */
        /* Mixed operand types forcing operand address reloads */
        int64_t immediate_val = 0xDEADBEEF;
        int64_t* volatile_ptr = (int64_t*)&volatile_buffer[0];
        
        asm volatile (
            /* Operation mixing immediate, register, and memory */
            "add %[out], %[imm], %[reg]\n\t"
            "ldr %[out2], [%[memptr], %[offset]]\n\t"
            : [out] "=&r" (temp1), [out2] "=r" (temp2)
            : [imm] "i" (0x1234), [reg] "r" (reg_e),
              [memptr] "r" (volatile_ptr), [offset] "r" (i * 8),
              "m" (*volatile_ptr)
            : "memory", "cc"
        );
        
        /* Pattern 5: Multiple overlapping clobbers forcing reloads */
        /* Use all explicit registers in different combinations */
        asm volatile (
            "mov %0, %1\n\t"
            "add %2, %3, %4\n\t"
            : "+r" (reg_a), "+r" (reg_b)
            : "r" (reg_c), "r" (reg_d), "r" (reg_e)
            : "cc"
        );
        
        /* Complex array access with multiple index calculations */
        /* This creates pressure for various reload types */
        int idx1 = (i * 3) % 16;
        int idx2 = (i * 5) % 8;
        int idx3 = (i * 7) % 4;
        
        /* Access different parts of structures with computed indices */
        volatile_var = global_array[idx1].b[idx3];
        volatile_var += local_struct.c[idx2];
        
        /* Force address computation for struct member access */
        int64_t* volatile ptr1 = &global_array[idx1].b[idx3];
        int16_t* volatile ptr2 = &local_struct.c[idx2];
        
        asm volatile (
            "ldr %0, [%1]\n\t"
            "ldrh %2, [%3]\n\t"
            : "=r" (temp1), "=r" (temp2)
            : "1" (ptr1), "3" (ptr2)
            : "memory"
        );
        
        /* Update register variables to create live range conflicts */
        reg_a += temp1;
        reg_b += temp2;
        reg_c = (reg_c << 3) | (i & 0x7);
        reg_d = reg_d * 1103515245 + 12345;
        reg_e = reg_e ^ (reg_a + reg_b);
    }
    
    /* Final barrier to prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Secondary function with different addressing patterns */
void nested_addressing(int depth, int64_t* base_ptr) {
    if (depth <= 0) return;
    
    /* Complex pointer arithmetic */
    volatile int64_t* volatile ptr_array[4];
    ptr_array[0] = (volatile int64_t*)((char*)base_ptr + depth * 8);
    ptr_array[1] = (volatile int64_t*)((char*)base_ptr + depth * 16);
    ptr_array[2] = (volatile int64_t*)((char*)base_ptr + depth * 24);
    ptr_array[3] = (volatile int64_t*)((char*)base_ptr + depth * 32);
    
    /* Mixed addressing modes in inline asm */
    for (int i = 0; i < 4; i++) {
        int64_t offset = i * sizeof(int64_t);
        
        asm volatile (
            /* Multiple memory accesses with different addressing */
            "ldr x0, [%[ptr], %[off]]\n\t"
            "str x0, [%[base], %[idx], lsl #3]\n\t"
            : 
            : [ptr] "r" (ptr_array[i]), [off] "r" (offset),
              [base] "r" (reg_a), [idx] "r" (i)
            : "x0", "memory"
        );
    }
    
    /* Recursive call with modified pointer */
    nested_addressing(depth - 1, base_ptr + 1);
}

/* Main function to set up and trigger reloads */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 16; i++) {
        global_array[i].a = i * 1000;
        for (int j = 0; j < 4; j++) {
            global_array[i].b[j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        volatile_buffer[i] = i;
    }
    
    /* Trigger primary reload patterns */
    trigger_reloads(iterations);
    
    /* Trigger nested addressing patterns */
    nested_addressing(8, (int64_t*)&global_array[0]);
    
    /* Final use of register variables to keep them live */
    asm volatile (
        "add %0, %1, %2\n\t"
        "add %3, %4, %5\n\t"
        : "+r" (reg_a), "+r" (reg_b)
        : "r" (reg_c), "r" (reg_d), "r" (reg_e), "0" (reg_a)
        : "cc"
    );
    
    return (int)(reg_a + reg_b + reg_c + reg_d + reg_e);
}
