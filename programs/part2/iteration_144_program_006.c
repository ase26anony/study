/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>
#include <stdlib.h>

/* Volatile mixed-type structures to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char flags[8];
    void* pointers[2];
} data_array[256];

volatile struct NestedPtrs {
    struct MixedData* md;
    int** matrix;
    volatile long* counters;
} ptr_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int*** ppp) {
    asm volatile("" : : "r"(ppp) : "memory");
}

static void compute_address(void** addr, volatile void* base, long offset) {
    *addr = (void*)((uintptr_t)base + offset);
}

/* Function with complex addressing patterns */
static void stress_address_calculations(int iter) {
    /* Bind specific pointers to explicit registers */
    register volatile struct MixedData* p1 asm ("r12") = &data_array[0];
    register volatile struct NestedPtrs* p2 asm ("r13") = &ptr_array[0];
    register int* index_ptr asm ("r14") = &iter;
    
    volatile int temp_results[16];
    void* computed_addrs[8];
    
    /* Complex control flow with goto */
    if (iter & 1) goto compute_block1;
    else goto compute_block2;
    
compute_block1:
    {
        /* Complex array indexing with multiple registers */
        long offset1 = (*index_ptr * 3 + 7) & 0xFF;
        long offset2 = (*index_ptr * 5 + 11) & 0x7F;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "addl %%eax, (%[addr2])\n\t"
            : 
            : [addr1] "m" (p1[offset1].counter),
              [addr2] "m" (p2[offset2].counters[0])
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to create control flow complexity */
        goto after_asm1;
    }
    
compute_block2:
    {
        /* Different addressing pattern */
        long offset = (*index_ptr * 7) & 0xFF;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "leaq (%[base],%[idx],8), %%r15\n\t"
            "movq %%r15, %[out]\n\t"
            : [out] "=m" (computed_addrs[0])
            : [base] "r" (p1), [idx] "r" (offset)
            : "r15", "r12", "memory"
        );
        goto after_asm1;
    }
    
after_asm1:
    /* Nested function call with address-taken arguments */
    {
        int** local_pptr = &index_ptr;
        modify_pptr(&local_pptr);
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        asm volatile (
            "movq %[inpaddr], %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "movl (%%rcx), %%edx\n\t"
            : 
            : [inpaddr] "m" (local_pptr)
            : "rbx", "rcx", "rdx", "memory"
        );
    }
    
    /* More complex addressing with pointer arithmetic */
    {
        register char* byte_ptr asm ("r15") = (char*)p1;
        byte_ptr += (*index_ptr * sizeof(struct MixedData)) & 0x3FF;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "movb $1, (%[ptr])\n\t"
            "addb $2, 8(%[ptr])\n\t"
            : 
            : [ptr] "r" (byte_ptr)
            : "r15", "memory"
        );
        
        /* Use in another context to force different reload type */
        temp_results[0] = *(int*)(byte_ptr + 4);
    }
    
    /* Loop with small iteration count */
    for (int i = 0; i < 3; i++) {
        /* Each iteration uses different addressing mode */
        void* addr;
        compute_address(&addr, p2, i * 16);
        
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        asm volatile (
            "movq %[addr], %%rsi\n\t"
            "movq $0x1234, (%%rsi)\n\t"
            : 
            : [addr] "m" (addr)
            : "rsi", "memory"
        );
        
        /* Mix data types in addressing */
        if (i & 1) {
            double* dbl_ptr = (double*)((char*)p1 + i * 24);
            *dbl_ptr += 1.0;
        }
    }
    
    /* Final complex pattern for RELOAD_FOR_OPADDR_ADDR */
    {
        volatile long* counters[4];
        for (int i = 0; i < 4; i++) {
            counters[i] = &p2[i].counters[0];
        }
        
        asm volatile (
            "movq %[base], %%rdi\n\t"
            "movq 8(%%rdi), %%r8\n\t"
            "movq 16(%%rdi), %%r9\n\t"
            "addq %%r8, %%r9\n\t"
            "movq %%r9, 24(%%rdi)\n\t"
            : 
            : [base] "m" (counters)
            : "rdi", "r8", "r9", "memory"
        );
    }
}

/* Another function with different patterns */
static void more_stress(int seed) {
    register volatile char* base1 asm ("r10") = (char*)&data_array[0];
    register volatile int* base2 asm ("r11") = &data_array[0].counter;
    
    /* Complex offset calculation */
    int offset = (seed * 13 + 17) % 256;
    
    /* Multiple memory operands with conflicting constraints */
    asm volatile (
        "imull $3, %[off], %%eax\n\t"
        "movl (%[b1],%%eax,1), %%ebx\n\t"
        "addl %%ebx, (%[b2],%[off],4)\n\t"
        : 
        : [b1] "r" (base1), [b2] "r" (base2), [off] "r" (offset)
        : "eax", "ebx", "r10", "r11", "memory"
    );
    
    /* Jump to create disjoint register usage */
    goto skip_section;
    
    {
        /* Unreachable but forces control flow analysis */
        volatile int dummy = 0;
        asm volatile ("" : : "r" (&dummy) : "memory");
    }
    
skip_section:
    /* Use same registers for different purpose */
    {
        int* alias1 = (int*)base1;
        int* alias2 = base2 + offset;
        
        /* Force output address reloads */
        asm volatile (
            "movl %[val], (%[out])\n\t"
            : 
            : [out] "m" (alias2), [val] "r" (*alias1)
            : "r10", "r11", "memory"
        );
    }
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        for (int j = 0; j < 3; j++) {
            data_array[i].values[j] = i * 0.5 + j;
        }
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i].md = (struct MixedData*)&data_array[i % 256];
        ptr_array[i].counters = (volatile long*)&data_array[i].counter;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 4; i++) {
        stress_address_calculations(i);
        more_stress(i * 7);
        
        /* Additional inline complexity */
        {
            register volatile struct MixedData* p asm ("r12") = &data_array[i * 16];
            volatile double* dp = &p->values[i % 3];
            
            /* Mixed constraints on same operand */
            asm volatile (
                "movsd (%[in]), %%xmm0\n\t"
                "addsd %%xmm0, %%xmm0\n\t"
                "movsd %%xmm0, %[out]\n\t"
                : [out] "=m" (*dp)
                : [in] "m" (*dp), "m" (p)
                : "xmm0", "r12", "memory"
            );
        }
    }
    
    return 0;
}
