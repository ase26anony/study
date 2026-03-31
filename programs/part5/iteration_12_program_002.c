/* reload1_trigger.c
 * Designed to trigger complex reload scenarios in GCC's reload pass
 * Specifically targets RELOAD_FOR_OTHER_ADDRESS and related reload types
 */

#include <stdint.h>
#include <stdlib.h>

/* Complex data structure to force interesting addressing */
struct MultiDim {
    int data[4][8][16];
    volatile int* volatile ptrs[8];
    long offsets[16];
};

/* Explicit register variables - force specific register allocation */
register int reg_a asm("r10");
register int reg_b asm("r11"); 
register int reg_c asm("r12");
register int reg_d asm("r13");
register int reg_e asm("r14");
register int reg_f asm("r15");

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile struct MultiDim* volatile global_struct = 0;

/* Function with complex addressing patterns */
void complex_addressing_loop(struct MultiDim* md, int iterations) {
    /* Local explicit register variables */
    register int idx1 asm("r8") = 0;
    register int idx2 asm("r9") = 1;
    register int* addr_reg asm("rbx");
    
    /* Force initial values into our fixed registers */
    asm volatile("" : "+r"(reg_a), "+r"(reg_b), "+r"(reg_c));
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Memory operand with complex address computation
         * Forces RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        int temp1, temp2, temp3;
        
        /* Complex address calculation using multiple registers */
        asm volatile(
            "lea (%[base], %[idx1], 4), %[addr]\n\t"
            "mov (%[addr], %[idx2], 8), %[out1]\n\t"
            "imul %[reg_a], %[out1]\n\t"
            "add %[reg_b], %[out1]"
            : [out1] "=r"(temp1), [addr] "=&r"(addr_reg)
            : [base] "r"(&md->data[0][0][0]),
              [idx1] "r"(idx1),
              [idx2] "r"(idx2),
              [reg_a] "r"(reg_a),
              [reg_b] "r"(reg_b)
            : "memory"
        );
        
        /* Pattern 2: Mixed operand types with register pressure
         * Forces RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile(
            "mov %[imm], %%eax\n\t"
            "add (%%rbx), %%eax\n\t"
            "mov %%eax, %[out]"
            : [out] "=rm"(temp2)
            : [imm] "i"(0x1234),
              "m"(*(volatile int*)addr_reg)
            : "eax", "memory"
        );
        
        /* Pattern 3: Output address reload with clobbered registers
         * Forces RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        volatile int* output_ptr;
        asm volatile(
            "mov %[in], %%rsi\n\t"
            "lea 16(%%rsi), %[out]\n\t"
            "movl $42, (%[out])"
            : [out] "=r"(output_ptr)
            : [in] "r"(&md->offsets[0]),
              "m"(md->offsets[0])
            : "rsi", "memory"
        );
        
        /* Pattern 4: Operand address reload for other address
         * Specifically targets RELOAD_FOR_OTHER_ADDRESS */
        int other_temp;
        register int* other_addr asm("rdi");
        
        /* Complex addressing with multiple levels of indirection */
        asm volatile(
            "mov %[base], %[addr]\n\t"
            "add %[offset], %[addr]\n\t"
            "mov (%[addr]), %[addr]\n\t"  /* Load address from memory */
            "mov (%[addr], %[idx], 4), %[out]"
            : [out] "=r"(other_temp), [addr] "=&r"(other_addr)
            : [base] "r"(&md->ptrs[0]),
              [offset] "r"(idx1 * sizeof(void*)),
              [idx] "r"(idx2)
            : "memory"
        );
        
        /* Pattern 5: Opaddr address reload
         * Forces RELOAD_FOR_OPADDR_ADDR */
        asm volatile(
            "mov %[struct], %%rax\n\t"
            "add $128, %%rax\n\t"
            "mov (%%rax, %[scale], 4), %[out]"
            : [out] "=r"(temp3)
            : [struct] "r"(md),
              [scale] "r"(reg_c)
            : "rax", "memory"
        );
        
        /* Update indices and registers to create varying patterns */
        idx1 = (idx1 + reg_a) & 7;
        idx2 = (idx2 + reg_b) & 15;
        reg_c = temp1 + temp2;
        reg_d = other_temp;
        
        /* Access volatile memory through computed address */
        *output_ptr = reg_c;
        
        /* Force spill/reload of fixed registers */
        asm volatile("" : : "r"(reg_e), "r"(reg_f));
    }
}

/* Another function with different patterns */
void nested_addressing(struct MultiDim* md) {
    /* Force register pressure */
    register int a asm("r10") = 1;
    register int b asm("r11") = 2;
    register int c asm("r12") = 3;
    register int d asm("r13") = 4;
    
    /* Complex expression that needs multiple reloads */
    for (int i = 0; i < 100; i++) {
        int result;
        
        /* Memory constraint with complex addressing */
        asm volatile(
            "imul %[b], %[a]\n\t"
            "add %[c], %[a]\n\t"
            "mov (%[mem], %[a], 4), %[out]"
            : [out] "=r"(result), [a] "+r"(a)
            : [b] "r"(b),
              [c] "r"(c),
              [mem] "r"(&md->data[0][0][0])
            : "memory"
        );
        
        /* Use the result in another asm with different constraints */
        asm volatile(
            "add %[in], %%eax\n\t"
            "mov %%eax, (%[ptr])"
            : 
            : [in] "r"(result),
              [ptr] "r"(&md->offsets[i & 15]),
              "m"(md->offsets[i & 15])
            : "eax", "memory"
        );
        
        /* Rotate registers to force different allocation decisions */
        asm volatile(
            "xchg %[a], %[b]\n\t"
            "xchg %[b], %[c]"
            : [a] "+r"(a), [b] "+r"(b), [c] "+r"(c)
            :
        );
        
        d = result + d;
    }
}

/* Main function sets up data and calls complex functions */
int main() {
    /* Allocate and initialize complex structure */
    struct MultiDim* md = (struct MultiDim*)malloc(sizeof(struct MultiDim));
    
    /* Initialize data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                md->data[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    for (int i = 0; i < 8; i++) {
        md->ptrs[i] = (volatile int*)&md->data[i % 4][0][0];
    }
    
    for (int i = 0; i < 16; i++) {
        md->offsets[i] = i * 8;
    }
    
    /* Initialize global register variables */
    reg_a = 1;
    reg_b = 2;
    reg_c = 3;
    reg_d = 4;
    reg_e = 5;
    reg_f = 6;
    
    global_struct = md;
    
    /* Call functions with different patterns */
    complex_addressing_loop(md, 50);
    nested_addressing(md);
    
    /* Final complex asm to ensure all reload types are considered */
    int final_result;
    asm volatile(
        "mov %[struct], %%rbx\n\t"
        "mov 64(%%rbx), %%rax\n\t"      /* Load address from ptrs */
        "mov (%%rax, %[idx], 4), %%rcx\n\t"
        "add %%rcx, %[out]\n\t"
        "mov %[out], (%[global])"
        : [out] "=r"(final_result)
        : [struct] "r"(md),
          [idx] "r"(reg_a),
          [global] "r"(&global_counter)
        : "rax", "rbx", "rcx", "memory"
    );
    
    free(md);
    
    /* Return something to prevent optimization */
    return final_result + reg_a + reg_b + reg_c;
}
