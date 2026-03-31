/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int *d;
};

struct NestedPtrs {
    volatile struct MixedData **arr;
    volatile long long counter;
};

/* Global volatile arrays to force memory accesses */
static volatile struct MixedData global_array[256];
static volatile struct NestedPtrs ptr_array[128];

/* Helper function taking pointer-to-pointer */
static void modify_through_pptr(volatile struct MixedData ***ppp, int idx) {
    volatile struct MixedData **temp = *ppp;
    if (temp && idx >= 0) {
        /* Force address computation */
        volatile struct MixedData *elem = temp[idx];
        if (elem) {
            elem->a = idx * 2;
        }
    }
}

/* Another helper with complex addressing */
static void compute_address_chain(volatile int ****quad_ptr, int i, int j, int k) {
    if (quad_ptr && *quad_ptr && **quad_ptr && ***quad_ptr) {
        volatile int *final = ***quad_ptr;
        /* Non-contiguous access pattern */
        final[i * 3 + j * 7 + k * 11] = i + j + k;
    }
}

/* Main stress function with inline assembly and complex control flow */
static void stress_reload_patterns(int iterations) {
    /* Bind specific variables to registers */
    register volatile struct MixedData *base_ptr asm ("r12") = &global_array[0];
    register volatile struct NestedPtrs *ptr_base asm ("r13") = &ptr_array[0];
    register int offset_reg asm ("r14") = 64;
    
    volatile int ***triple_ptr;
    volatile int deep_array[8][8][8];
    volatile int *alias_ptr;
    
    /* Initialize triple pointer chain */
    volatile int **double_ptr = (volatile int **)&deep_array[0][0];
    triple_ptr = (volatile int ***)&double_ptr;
    
    int i = 0;
    
    /* Complex loop with goto jumps */
    loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Complex address computation 1 - forces RELOAD_FOR_INPUT_ADDRESS */
    {
        volatile struct MixedData *elem1 = base_ptr + (offset_reg ^ i) * 3;
        volatile struct MixedData *elem2 = base_ptr + (offset_reg | i) * 5;
        
        /* Inline assembly with memory constraints and clobbers */
        asm volatile (
            "movl %[val1], (%[ptr1])\n\t"
            "movl %[val2], (%[ptr2])\n\t"
            : 
            : [ptr1] "r" (&elem1->a), 
              [val1] "r" (i * 2),
              [ptr2] "r" (&elem2->a),
              [val2] "r" (i * 3)
            : "memory", "r12", "r13", "r14"
        );
    }
    
    /* Jump to create control flow complexity */
    if (i & 1) goto odd_case;
    
    /* Even case - different address computation */
    {
        /* Forces RELOAD_FOR_OUTPUT_ADDRESS */
        volatile int *out_addr;
        register int idx asm ("r15") = i * 7 + 3;
        
        asm volatile (
            "leal (%[base], %[idx], 4), %[out]\n\t"
            : [out] "=r" (out_addr)
            : [base] "r" (base_ptr),
              [idx] "r" (idx)
            : "r15"
        );
        
        if (out_addr) {
            *out_addr = i * 100;
        }
        
        /* Call helper with address-taken argument */
        volatile struct MixedData **temp_ptr = &base_ptr;
        modify_through_pptr((volatile struct MixedData ***)&temp_ptr, i % 16);
    }
    
    goto increment;
    
    odd_case:
    /* Odd case - more complex addressing */
    {
        /* Forces RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        volatile struct NestedPtrs *current = ptr_base + (i % 8);
        volatile struct MixedData **indirect;
        
        /* Multiple levels of indirection */
        if (current->arr) {
            indirect = current->arr + (i % 4);
            
            /* Inline assembly with multiple memory operands */
            asm volatile (
                "movq (%[src]), %%rax\n\t"
                "movq %%rax, (%[dst])\n\t"
                "addl $1, (%[cnt])\n\t"
                : 
                : [src] "r" (indirect),
                  [dst] "r" (&current->arr),
                  [cnt] "r" (&current->counter)
                : "rax", "memory", "r12", "r13"
            );
        }
        
        /* Complex address chain computation */
        compute_address_chain((volatile int ****)&triple_ptr, 
                             i & 7, (i >> 3) & 7, (i >> 6) & 7);
    }
    
    increment:
    /* Re-bind register variables with new values to force reloads */
    {
        register int new_offset asm ("r14") = offset_reg + i * 13;
        
        /* Forces RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile (
            "imull $37, %[off], %[off]\n\t"
            : [off] "+r" (new_offset)
            :
            : "cc"
        );
        
        offset_reg = new_offset;
        
        /* Update base pointer with complex offset */
        base_ptr = &global_array[(i * 97) % 256];
    }
    
    i++;
    goto loop_start;
    
    loop_end:
    
    /* Final complex addressing pattern */
    {
        /* Forces RELOAD_FOR_OPADDR_ADDR and RELOAD_FOR_OTHER_ADDRESS */
        volatile int ****quad_ptr_chain = (volatile int ****)&triple_ptr;
        register volatile int *final_addr asm ("r12");
        
        asm volatile (
            "movq (%[chain]), %%r12\n\t"
            "testq %%r12, %%r12\n\t"
            "jz 1f\n\t"
            "movq (%%r12), %%r12\n\t"
            "testq %%r12, %%r12\n\t"
            "jz 1f\n\t"
            "movq (%%r12), %%r12\n\t"
            "1:\n\t"
            : "=r" (final_addr)
            : [chain] "r" (quad_ptr_chain)
            : "r12", "cc", "memory"
        );
        
        if (final_addr) {
            /* Scattered access pattern */
            for (int j = 0; j < 8; j++) {
                final_addr[j * 17] = j * 11;
            }
        }
    }
}

/* Additional stress patterns */
static void more_address_modes(void) {
    volatile double matrix[16][16];
    volatile int *int_view = (volatile int *)matrix;
    
    /* Register binding with explicit constraints */
    register volatile double *row_ptr asm ("r12") = &matrix[0][0];
    register int index asm ("r13") = 0;
    
    for (int i = 0; i < 8; i++) {
        /* Mixed-type access through different pointer types */
        double *dbl_ptr = (double *)(int_view + index * 4);
        
        /* Assembly with conflicting constraints */
        asm volatile (
            "movsd (%[src]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, (%[dst])\n\t"
            : 
            : [src] "r" (row_ptr),
              [dst] "r" (dbl_ptr)
            : "xmm0", "memory", "r12", "r13"
        );
        
        /* Update pointers in non-linear fashion */
        index = (index * 3 + 7) & 15;
        row_ptr = &matrix[index][index];
        
        /* Take address of register variable */
        volatile double **pptr = &row_ptr;
        if (pptr && *pptr) {
            (*pptr)[index] = 3.14159 * i;
        }
    }
}

int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
        for (int j = 0; j < 7; j++) {
            global_array[i].c[j] = 'A' + (i + j) % 26;
        }
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i].arr = (volatile struct MixedData **)&global_array[i % 64];
        ptr_array[i].counter = i * 1000LL;
    }
    
    /* Call stress functions multiple times with different parameters */
    stress_reload_patterns(4);  /* Small iteration count as requested */
    more_address_modes();
    
    /* Second call with different pattern */
    stress_reload_patterns(2);
    
    return 0;
}
