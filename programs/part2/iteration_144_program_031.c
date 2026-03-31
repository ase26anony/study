/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct mixed_data {
    volatile int counter;
    volatile double values[3];
    volatile char tags[8];
    volatile int *ptr;
};

/* Large volatile arrays */
static volatile struct mixed_data data_array[1024];
static volatile int global_index = 0;

/* Helper functions that take pointer-to-pointer arguments */
void update_pointer(int **pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

void compute_address(void **addr_ptr, volatile void *base, int offset) {
    *addr_ptr = (void *)((char *)base + offset);
}

/* Function with complex addressing patterns */
void stress_reloads(void) {
    /* Bind specific registers for address computation */
    register volatile struct mixed_data *p1 asm ("r12") = &data_array[0];
    register volatile struct mixed_data *p2 asm ("r13") = &data_array[512];
    register int index asm ("r14") = global_index;
    
    volatile int local_var = 42;
    volatile double local_dbl = 3.14159;
    int *ptr_to_local = (int *)&local_var;
    
    /* Complex addressing computation block 1 */
    {
        volatile struct mixed_data *temp;
        int offset = (index * 3 + 7) & 0xFF;
        
        /* Force address computation with register constraints */
        asm volatile (
            "lea (%[base], %[idx], 4), %[temp]\n\t"
            : [temp] "=r" (temp)
            : [base] "r" (p1), [idx] "r" (offset)
            : /* No clobbers - yet */
        );
        
        /* Access with computed address */
        temp->counter += 1;
        
        /* Inline asm with memory operand and clobbered address register */
        asm volatile (
            "movl $99, (%[mem])\n\t"
            "addl $1, %%r12d\n\t"  /* Clobber r12 */
            : 
            : [mem] "m" (temp->counter)
            : "r12", "memory"
        );
    }
    
    /* Jump to different addressing pattern */
    goto second_block;
    
first_block_return:
    /* Nested pointer usage */
    {
        int **pp = &ptr_to_local;
        update_pointer(pp);
        
        /* Complex array indexing with multiple registers */
        int idx1 = (global_index * 2) % 1024;
        int idx2 = (global_index * 3) % 1024;
        
        volatile struct mixed_data *elem1 = &data_array[idx1];
        volatile struct mixed_data *elem2 = &data_array[idx2];
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movl (%[src]), %%eax\n\t"
            "addl %%eax, (%[dst])\n\t"
            : 
            : [src] "m" (elem1->counter), [dst] "m" (elem2->counter)
            : "rax", "memory"
        );
    }
    return;
    
second_block:
    /* Different addressing mode - output address reloads */
    {
        volatile int *out_ptr;
        int complex_offset = (index << 2) | 0x3F;
        
        /* Compute address for output */
        asm volatile (
            "lea (%[base], %[off], 2), %[out]\n\t"
            : [out] "=r" (out_ptr)
            : [base] "r" (p2), [off] "r" (complex_offset)
            : /* No clobbers */
        );
        
        /* Store with computed address */
        *out_ptr = 1234;
        
        /* Inline asm that clobbers address registers */
        asm volatile (
            "movq $0, %%r13\n\t"  /* Clobber r13 */
            "movq $0, %%r14\n\t"  /* Clobber r14 */
            : 
            : 
            : "r13", "r14", "memory"
        );
        
        /* Recompute address with clobbered registers */
        out_ptr = &p2->values[1];
        *out_ptr = local_dbl;
    }
    
    goto first_block_return;
}

/* Function targeting operand address reloads */
void stress_operand_address(void) {
    register volatile char *buf_ptr asm ("r15") = data_array[0].tags;
    volatile int results[4];
    void *computed_addr;
    
    /* Complex offset calculation */
    for (int i = 0; i < 4; i++) {
        int offset = (i * 7 + 3) * sizeof(double);
        
        /* Function call with address-taken argument */
        compute_address(&computed_addr, buf_ptr, offset);
        
        /* Use computed address in inline asm */
        asm volatile (
            "movq (%[addr]), %%rax\n\t"
            "movq %%rax, %[res]\n\t"
            : [res] "=m" (results[i])
            : [addr] "r" (computed_addr)
            : "rax", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (i == 2) {
            goto special_case;
        }
        
        continue;
        
    special_case:
        /* Different addressing pattern in the middle of loop */
        asm volatile (
            "incq %%r15\n\t"  /* Clobber address register */
            : 
            : 
            : "r15"
        );
        
        /* Force recomputation with clobbered register */
        buf_ptr = data_array[1].tags + i;
    }
    
    /* Final asm with multiple constraints on same operand */
    {
        volatile int x = 100;
        int y;
        
        asm volatile (
            "movl %[input], %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            : [output] "=m,r" (y)  /* Multiple constraints */
            : [input] "m,r" (x)    /* Multiple constraints */
            : "rax", "memory"
        );
    }
}

/* Main function that orchestrates the stress test */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1024; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.0;
        data_array[i].values[1] = i * 2.0;
        data_array[i].values[2] = i * 3.0;
        for (int j = 0; j < 8; j++) {
            data_array[i].tags[j] = 'A' + (i + j) % 26;
        }
    }
    
    /* Multiple calls with different patterns */
    for (int iter = 0; iter < 10; iter++) {
        global_index = iter;
        
        /* Call stress functions */
        stress_reloads();
        stress_operand_address();
        
        /* Inline complex addressing in main */
        {
            register volatile int *reg_ptr asm ("rbx");
            int idx = (iter * 13 + 5) % 1024;
            
            reg_ptr = &data_array[idx].counter;
            
            /* Complex addressing with inline asm */
            asm volatile (
                "movl (%[ptr]), %%eax\n\t"
                "imull $3, %%eax\n\t"
                "movl %%eax, (%[ptr])\n\t"
                : 
                : [ptr] "r" (reg_ptr)
                : "rax", "memory"
            );
            
            /* Clobber and reuse register */
            asm volatile (
                "xorq %%rbx, %%rbx\n\t"
                : 
                : 
                : "rbx"
            );
            
            /* Force reload by using register again */
            reg_ptr = &data_array[(iter + 1) % 1024].counter;
            *reg_ptr = iter;
        }
    }
    
    return 0;
}
