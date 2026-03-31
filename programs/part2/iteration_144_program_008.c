/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>
#include <stdlib.h>

/* Volatile mixed-type structures to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    void *next;
} data_array[256];

volatile struct PointerChain {
    struct PointerChain **links[4];
    long indices[2];
    float weights[4];
} chain_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void update_pointer(void **pp) {
    *pp = (void*)((uintptr_t)*pp + 1);
}

static void compute_address(int ***ppp, int offset) {
    **ppp = ***ppp + offset;
}

/* Function with complex addressing patterns */
static void stress_address_computation(int iter) {
    /* Bind specific pointers to registers */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct PointerChain *p2 asm ("r13") = &chain_array[0];
    register int *addr_reg asm ("r14") = NULL;
    
    /* Complex addressing with multiple constraints */
    int offset1 = iter * 3 + 1;
    int offset2 = iter * 7 - 2;
    
    /* Block 1: RELOAD_FOR_INPUT_ADDRESS patterns */
    {
        volatile int *addr1 = &p1[offset1].counter;
        volatile double *addr2 = &p1[offset2].values[1];
        
        /* Inline asm with memory operands and clobbers */
        asm volatile (
            "addl $1, %[mem1]\n\t"
            "fldl %[mem2]\n\t"
            : [mem1] "+m" (*addr1)
            : [mem2] "m" (*addr2)
            : "memory", "r12", "r13", "st"
        );
    }
    
    /* Jump to create control flow complexity */
    goto compute_more;
    
recompute:
    /* Block 2: Different addressing mode after jump */
    {
        /* Force recomputation of addresses */
        p1 = &data_array[iter % 128];
        volatile char *tag_ptr = &p1[offset2].tags[3];
        
        /* Another asm with conflicting constraints */
        int temp;
        asm volatile (
            "movb %[in], %%al\n\t"
            "movb %%al, %[out]\n\t"
            : [out] "=m" (*tag_ptr)
            : [in] "m" (*tag_ptr)
            : "al", "r12"
        );
    }
    
    /* Nested pointer indirection for RELOAD_FOR_INPADDR_ADDRESS */
    {
        void *ptr = (void*)&p1[iter].next;
        update_pointer(&ptr);
        
        /* Use the updated pointer */
        volatile struct MixedData *p3 = (volatile struct MixedData*)ptr;
        p3->counter = iter;
    }
    
    return;
    
compute_more:
    /* Block 3: RELOAD_FOR_OUTPUT_ADDRESS patterns */
    {
        /* Complex array indexing with multiple dimensions */
        volatile struct PointerChain ***link_ptr = 
            &p2[offset1 % 64].links[2];
        
        /* Inline asm with output memory operand */
        long index_val;
        asm volatile (
            "movq %[idx], %%rax\n\t"
            "incq %%rax\n\t"
            "movq %%rax, %[store]\n\t"
            : [store] "=m" (p2[offset2 % 32].indices[1])
            : [idx] "m" (p2[offset1 % 32].indices[0])
            : "rax", "r13", "memory"
        );
        
        /* Triple pointer indirection */
        int ***triple_ptr = (int***)&link_ptr;
        compute_address(triple_ptr, offset1);
    }
    
    /* Jump back to create loop-like flow */
    goto recompute;
}

/* Another stress function focusing on operand addresses */
static void stress_operand_addresses(void) {
    register int *base1 asm ("r12") = (int*)&data_array[0].counter;
    register int *base2 asm ("r13") = (int*)&chain_array[0].indices[0];
    
    /* Multiple memory accesses with address computations */
    for (int i = 0; i < 4; i++) {
        /* Varying offsets create different addressing modes */
        int *addr1 = base1 + i * sizeof(struct MixedData) / sizeof(int);
        int *addr2 = base2 + i * 7;
        
        /* Inline asm using both addresses */
        asm volatile (
            "movl %[src], %%eax\n\t"
            "addl $42, %%eax\n\t"
            "movl %%eax, %[dst]\n\t"
            : [dst] "=m" (*addr2)
            : [src] "m" (*addr1)
            : "eax", "r12", "r13", "memory"
        );
        
        /* Non-contiguous access pattern */
        volatile float *weight_ptr = 
            &chain_array[i * 3 % 128].weights[i % 4];
        
        /* Another asm with different constraints */
        asm volatile (
            "pxor %%xmm0, %%xmm0\n\t"
            "cvtsi2ssl %[val], %%xmm0\n\t"
            "movss %%xmm0, %[out]\n\t"
            : [out] "=m" (*weight_ptr)
            : [val] "r" (i)
            : "xmm0", "memory"
        );
    }
    
    /* Complex control flow with goto */
    if (base1 != base2) {
        goto handle_different;
    }
    
    return;
    
handle_different:
    {
        /* Address computation that may need RELOAD_FOR_OPADDR_ADDR */
        int **ptr_to_ptr = &base1;
        update_pointer((void**)ptr_to_ptr);
        
        /* Use the modified pointer */
        volatile int value = **ptr_to_ptr;
        (void)value; /* Prevent unused warning */
    }
}

/* Function with mixed data type accesses */
static void stress_mixed_types(int seed) {
    /* Bind to specific registers */
    register volatile char *char_ptr asm ("r12") = 
        (volatile char*)&data_array[0];
    register volatile double *double_ptr asm ("r13") = 
        (volatile double*)&chain_array[0];
    
    /* Scattered, non-sequential accesses */
    for (int i = 0; i < 8; i++) {
        /* Different stride for different types */
        volatile char *c = char_ptr + i * 13;
        volatile double *d = double_ptr + i * 5;
        
        /* Inline asm with type-specific operations */
        asm volatile (
            "movb $65, %[chr]\n\t"
            "fldl %[dbl]\n\t"
            "fadd %%st(0), %%st(0)\n\t"
            "fstpl %[dbl]\n\t"
            : [chr] "=m" (*c), [dbl] "+m" (*d)
            : 
            : "st", "memory", "r12", "r13"
        );
    }
    
    /* Pointer chain for address reloads */
    {
        volatile void **ptr_array[4];
        ptr_array[0] = (volatile void**)&char_ptr;
        ptr_array[1] = (volatile void**)&double_ptr;
        ptr_array[2] = (volatile void**)&data_array[seed % 16].next;
        
        /* Multiple levels of indirection */
        for (int j = 0; j < 3; j++) {
            volatile void *current = *ptr_array[j];
            update_pointer((void**)&current);
            
            /* Use in memory operation */
            if (j == 1) {
                asm volatile (
                    "movq %[ptr], %%r14\n\t"
                    "orq $1, (%%r14)\n\t"
                    : 
                    : [ptr] "r" (current)
                    : "r14", "memory"
                );
            }
        }
    }
}

/* Main function that orchestrates the stress tests */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].tags[0] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 128; i++) {
        chain_array[i].indices[0] = i;
        chain_array[i].weights[0] = i * 0.5f;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int iter = 0; iter < 8; iter++) {
        stress_address_computation(iter);
        stress_operand_addresses();
        stress_mixed_types(iter);
        
        /* Additional inline complexity in main */
        {
            register volatile int *p asm ("r12") = 
                &data_array[iter].counter;
            register int offset asm ("r13") = iter * 16;
            
            /* Complex address calculation */
            volatile int *addr = p + offset / sizeof(int);
            
            /* Asm with both input and output address reloads */
            asm volatile (
                "movl %[in], %%eax\n\t"
                "leal 100(%%eax), %%ebx\n\t"
                "movl %%ebx, %[out]\n\t"
                : [out] "=m" (*addr)
                : [in] "m" (*addr)
                : "eax", "ebx", "r12", "r13", "memory"
            );
            
            /* Jump to create basic block boundaries */
            if (iter & 1) {
                goto odd_iteration;
            }
            
            continue;
            
        odd_iteration:
            /* Different address mode for odd iterations */
            volatile double *dbl_addr = 
                &data_array[iter].values[iter % 3];
            asm volatile (
                "fldl %[val]\n\t"
                "fsqrt\n\t"
                "fstpl %[val]\n\t"
                : [val] "+m" (*dbl_addr)
                :
                : "st", "memory"
            );
        }
    }
    
    return 0;
}
