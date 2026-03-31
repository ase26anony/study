/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int32_t a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType *ptr;
    volatile int offset;
} AddrHolder;

/* Global volatile arrays to force memory accesses */
volatile MixedType global_array[256];
volatile AddrHolder addr_holders[16];

/* Helper function taking pointer-to-pointer */
void modify_pointer(volatile MixedType ***ppp) {
    volatile MixedType **temp = *ppp;
    if (temp && *temp) {
        /* Force memory access through multiple indirections */
        (*temp)->a = ((*temp)->a & 0xFF) | 0x100;
    }
}

/* Another helper with complex addressing */
void compute_address(volatile MixedType *base, int idx1, int idx2, int idx3) {
    /* Complex address computation */
    volatile MixedType *p = base + idx1;
    p = p + (idx2 * 3) / 2;
    p = (volatile MixedType *)((char *)p + idx3 * 13);
    
    /* Use in inline asm with constraints */
    register volatile MixedType *r12_ptr asm ("r12") = p;
    register int r13_idx asm ("r13") = idx1 + idx2 + idx3;
    
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl %%eax, %%eax\n\t"
        "movl %%eax, (%[ptr])\n\t"
        : 
        : [ptr] "r" (r12_ptr), "r" (r13_idx)
        : "eax", "memory", "r12", "r13"
    );
}

/* Function with multiple address reload scenarios */
void stress_reloads(void) {
    /* Bind specific variables to registers */
    register volatile MixedType *p1 asm ("r12") = &global_array[0];
    register volatile MixedType *p2 asm ("r13") = &global_array[128];
    register int offset asm ("r14") = 64;
    
    volatile MixedType *local_ptrs[4];
    volatile MixedType **pp = &local_ptrs[0];
    
    /* Label for goto jumps */
    compute_addr:
    
    /* Complex address computation 1 - may need RELOAD_FOR_INPUT_ADDRESS */
    volatile MixedType *addr1 = p1 + (offset * 3) / 2;
    addr1 = (volatile MixedType *)((char *)addr1 + (offset & 0xF) * 7);
    
    /* Inline asm with memory operand and clobbered address register */
    asm volatile (
        "movq (%[base]), %%r15\n\t"
        "addq $8, %%r15\n\t"
        "movq %%r15, (%[base])\n\t"
        : 
        : [base] "r" (&addr1), "m" (*addr1)
        : "r15", "memory"
    );
    
    /* Use computed address */
    local_ptrs[0] = addr1;
    
    /* Jump to create control flow complexity */
    goto next_block;
    
    /* Unreachable code that still affects compilation */
    p1 = p1 + 1;
    
    next_block:
    
    /* Different address computation for same register variable */
    p1 = p2 + offset;
    p1 = (volatile MixedType *)((char *)p1 - (offset * 5));
    
    /* Inline asm that clobbers the register we're using for addressing */
    asm volatile (
        "xor %%r12, %%r12\n\t"
        "xor %%r13, %%r13\n\t"
        : 
        : 
        : "r12", "r13"
    );
    
    /* Now p1 and p2 are clobbered, need reloads */
    
    /* Complex addressing with multiple constraints */
    register volatile char *char_ptr asm ("r12") = (char *)&global_array[0];
    register int idx asm ("r13") = offset * 3 + 7;
    
    /* May trigger RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "movb (%[ptr], %[idx]), %%al\n\t"
        "addb $1, %%al\n\t"
        "movb %%al, (%[ptr], %[idx])\n\t"
        : 
        : [ptr] "r" (char_ptr), [idx] "r" (idx)
        : "al", "memory", "r12", "r13"
    );
    
    /* Call function with address-taken argument - may need RELOAD_FOR_INPADDR_ADDRESS */
    modify_pointer(&pp);
    
    /* More complex addressing with structure member access */
    volatile MixedType *p3 = &global_array[offset];
    p3 = (volatile MixedType *)((char *)p3 + p3->a);
    
    /* Inline asm with output memory operand - may need RELOAD_FOR_OUTPUT_ADDRESS */
    int32_t temp;
    asm volatile (
        "leal (%[base], %[idx], 4), %[out]\n\t"
        : [out] "=r" (temp)
        : [base] "r" (p3->a), [idx] "r" (offset)
        : "cc"
    );
    
    /* Store result through complex address */
    volatile int32_t *out_addr = &((volatile MixedType *)((char *)p1 + temp))->a;
    
    /* May trigger RELOAD_FOR_OUTADDR_ADDRESS */
    asm volatile (
        "movl %[val], (%[addr])\n\t"
        : 
        : [val] "r" (temp), [addr] "r" (out_addr)
        : "memory"
    );
    
    /* Loop with scattered accesses */
    for (int i = 0; i < 3; i++) {
        /* Non-contiguous access pattern */
        volatile MixedType *elem = &global_array[(i * 37) & 0xFF];
        
        /* Mix data types in access */
        elem->b = elem->a * 2.0;
        elem->c[i % 7] = elem->d & 0xFF;
        
        /* Complex address for next iteration */
        if (i < 2) {
            p1 = (volatile MixedType *)((char *)elem + elem->a * 8);
            goto compute_addr;  /* Jump back with changed register */
        }
    }
    
    /* Final inline asm with multiple memory operands */
    volatile int32_t values[4];
    asm volatile (
        "movl (%[src1]), %%eax\n\t"
        "addl (%[src2]), %%eax\n\t"
        "movl %%eax, (%[dst1])\n\t"
        "movl (%[src3]), %%ebx\n\t"
        "subl (%[src4]), %%ebx\n\t"
        "movl %%ebx, (%[dst2])\n\t"
        : 
        : [src1] "r" (&global_array[0].a),
          [src2] "r" (&global_array[1].a),
          [dst1] "r" &values[0],
          [src3] "r" (&global_array[2].a),
          [src4] "r" (&global_array[3].a),
          [dst2] "r" &values[1]
        : "eax", "ebx", "memory", "r12", "r13", "r14"
    );
}

/* Additional stress function */
void more_address_computations(void) {
    /* Use register variables with explicit registers */
    register volatile MixedType *r10_ptr asm ("r10") = &global_array[32];
    register volatile MixedType *r11_ptr asm ("r11") = &global_array[96];
    
    /* Chain of address computations */
    for (int i = 0; i < 4; i++) {
        /* May trigger RELOAD_FOR_OPADDR_ADDR */
        volatile MixedType **addr_of_ptr = &r10_ptr;
        
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "addq $16, %%rax\n\t"
            "movq %%rax, (%[addr])\n\t"
            : 
            : [addr] "r" (addr_of_ptr)
            : "rax", "memory", "r10"
        );
        
        /* Switch between different base pointers */
        if (i & 1) {
            r10_ptr = r11_ptr + i * 8;
        } else {
            r11_ptr = r10_ptr - i * 12;
        }
        
        /* Access through computed pointer */
        volatile char *char_access = (char *)r10_ptr + r11_ptr->a;
        
        /* Inline asm with "m" constraint on computed address */
        asm volatile (
            "incb (%[mem])\n\t"
            : 
            : [mem] "m" (*char_access)
            : "memory", "r10", "r11"
        );
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
        global_array[i].d = i * 1000LL;
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    more_address_computations();
    
    /* Additional inline complexity in main */
    register volatile int *reg_int asm ("r12") = &global_array[0].a;
    register int idx asm ("r13") = 42;
    
    /* Complex addressing in main */
    for (int j = 0; j < 2; j++) {
        /* May trigger RELOAD_FOR_OTHER_ADDRESS */
        volatile int *complex_addr = reg_int + (idx * (j + 1) * 7) / 3;
        
        asm volatile (
            "movl (%[base], %[index], 4), %%eax\n\t"
            "negl %%eax\n\t"
            "movl %%eax, (%[base], %[index], 4)\n\t"
            : 
            : [base] "r" (complex_addr), [index] "r" (j)
            : "eax", "memory", "r12", "r13"
        );
        
        /* Function call with complex argument address */
        compute_address(&global_array[0], j, idx, j * 2);
    }
    
    return 0;
}
