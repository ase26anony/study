/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    void *next;
} data_array[256];

volatile struct PointerChain {
    struct PointerChain **links;
    long long bigval;
    int small;
} chain_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void update_pointer(int ***ppp) {
    **ppp = (int*)((uintptr_t)**ppp + 1);
}

static void chain_helper(struct PointerChain ****pcppp) {
    ***pcppp = (struct PointerChain*)((uintptr_t)***pcppp + 8);
}

/* Function with complex addressing patterns */
static void stress_address_calculations(int iter) {
    /* Bind specific variables to registers */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct PointerChain *p2 asm ("r13") = &chain_array[0];
    register int *addr_temp asm ("r14");
    
    /* Complex offset calculations */
    int offset1 = iter * 3 + 7;
    int offset2 = iter * 5 + 11;
    long offset3 = iter * 7 + 13;
    
    /* Block 1: Multiple addressing modes */
    {
        /* RELOAD_FOR_INPUT_ADDRESS patterns */
        volatile double *dp = &p1[offset1].values[offset2 % 3];
        volatile char *cp = &p1[offset2].tags[offset1 % 8];
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movq %[dp_ptr], %%r15\n\t"
            "movsd (%%r15), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (*dp)
            : [dp_ptr] "r" (dp)
            : "r15", "xmm0", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (iter & 1) goto compute_addr;
    }
    
    /* Block 2: Different addressing pattern */
    {
        compute_addr:
        /* RELOAD_FOR_OUTPUT_ADDRESS patterns */
        volatile void **next_ptr = &p1[offset3 % 64].next;
        
        /* Assembly with conflicting constraints */
        asm volatile (
            "mov %[next], %%rbx\n\t"
            "movq $0x12345678, (%%rbx)\n\t"
            : 
            : [next] "r" (next_ptr)
            : "rbx", "memory"
        );
        
        /* Nested pointer usage */
        struct PointerChain ***temp = &chain_array[offset1 % 32].links;
        chain_helper(&temp);
    }
    
    /* Block 3: More complex patterns */
    {
        /* RELOAD_FOR_INPADDR_ADDRESS patterns */
        int **pp = (int**)&p2[offset2 % 16].links;
        update_pointer(&pp);
        
        /* Mixed addressing in single expression */
        addr_temp = (int*)&p1[offset1].values[1] + offset2;
        
        /* Assembly with multiple memory operands */
        asm volatile (
            "mov %[addr], %%rdi\n\t"
            "movl $42, (%%rdi)\n\t"
            "movl (%%rdi), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[val]\n\t"
            : [val] "=m" (p2[offset3 % 8].small)
            : [addr] "r" (addr_temp)
            : "rdi", "rax", "memory"
        );
    }
    
    /* Block 4: Output address patterns */
    {
        /* RELOAD_FOR_OUTADDR_ADDRESS patterns */
        volatile long long *bigptr = &p2[offset1 % 64].bigval;
        
        /* Complex address computation in constraint */
        asm volatile (
            "mov %[base], %%rsi\n\t"
            "mov %[offset], %%rcx\n\t"
            "leaq (%%rsi,%%rcx,8), %%rdx\n\t"
            "movq $0x987654321, (%%rdx)\n\t"
            : 
            : [base] "r" (p2), [offset] "r" (offset3 % 32)
            : "rsi", "rcx", "rdx", "memory"
        );
        
        /* Use goto to jump back */
        if (iter & 2) goto compute_addr;
    }
}

/* Function with operand address reloads */
static void stress_operand_addresses(void) {
    register int *op1 asm ("r12") = (int*)&data_array[32].counter;
    register int *op2 asm ("r13") = (int*)&data_array[64].counter;
    register int *op3 asm ("r14") = (int*)&data_array[96].counter;
    
    /* RELOAD_FOR_OPERAND_ADDRESS patterns */
    asm volatile (
        "mov %[a], %%r15\n\t"
        "mov %[b], %%rbx\n\t"
        "mov %[c], %%rcx\n\t"
        "movl (%%r15), %%eax\n\t"
        "addl (%%rbx), %%eax\n\t"
        "subl (%%rcx), %%eax\n\t"
        "movl %%eax, (%%r15)\n\t"
        : 
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3)
        : "r15", "rbx", "rcx", "rax", "memory"
    );
    
    /* RELOAD_FOR_OPADDR_ADDR patterns */
    int **addr_of_op1 = &op1;
    asm volatile (
        "mov %[ptr], %%rdi\n\t"
        "movq (%%rdi), %%r8\n\t"
        "movl $99, (%%r8)\n\t"
        : 
        : [ptr] "r" (addr_of_op1)
        : "rdi", "r8", "memory"
    );
}

/* Function with other address reloads */
static void stress_other_addresses(int idx) {
    register volatile char *base asm ("r12") = (char*)data_array;
    volatile int *dest asm ("r13") = &chain_array[0].small;
    
    /* Complex addressing chain */
    char *addr1 = base + idx * sizeof(struct MixedData) + offsetof(struct MixedData, tags);
    char *addr2 = addr1 + 4;
    int *final = (int*)(addr2 - 2);
    
    /* RELOAD_FOR_OTHER_ADDRESS patterns */
    asm volatile (
        "mov %[src], %%r14\n\t"
        "mov %[dst], %%r15\n\t"
        "movzwl (%%r14), %%eax\n\t"
        "movl %%eax, (%%r15)\n\t"
        : 
        : [src] "r" (final), [dst] "r" (dest)
        : "r14", "r15", "rax", "memory"
    );
}

/* Main function that creates the stress patterns */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].tags[0] = 'A' + (i % 26);
    }
    
    /* Multiple iterations with different parameters */
    for (int i = 0; i < 8; i++) {
        stress_address_calculations(i);
    }
    
    stress_operand_addresses();
    
    for (int i = 0; i < 4; i++) {
        stress_other_addresses(i * 16);
    }
    
    /* Final complex block mixing everything */
    {
        register volatile struct MixedData *mp asm ("r12") = &data_array[128];
        register int *ip asm ("r13") = (int*)&chain_array[32];
        
        /* Mixed constraints in single asm */
        asm volatile (
            "mov %[base], %%r14\n\t"
            "movl 4(%%r14), %%eax\n\t"
            "imull $3, %%eax, %%eax\n\t"
            "mov %[dest], %%r15\n\t"
            "movl %%eax, (%%r15)\n\t"
            "addq $16, %%r14\n\t"
            "movq %%r14, %[base]\n\t"
            : [base] "+r" (mp)
            : [dest] "r" (ip)
            : "r14", "r15", "rax", "memory"
        );
        
        /* Address of register variable */
        int **pp = &ip;
        update_pointer(&pp);
    }
    
    return 0;
}
