/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    int *ptr;
    long long big;
} data_array[256];

volatile struct NestedPtrs {
    int **pptr;
    volatile int *vptr;
    double matrix[2][2];
} ptr_array[128];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int ***ppp) {
    volatile static int dummy = 42;
    *ppp = (int **)&dummy;
}

static void compute_address(volatile void **addr, int offset) {
    *addr = (volatile void *)((char *)*addr + offset);
}

/* Function with complex addressing patterns */
static void stress_reloads(int iterations) {
    /* Bind specific registers for address computation */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct NestedPtrs *p2 asm ("r13") = &ptr_array[0];
    register int *index_ptr asm ("r14") = (int *)&iterations;
    
    volatile int local_vars[16];
    volatile double results[8];
    
    /* Label for goto jumps */
    compute_again:
    
    /* Complex address computation with multiple constraints */
    for (int i = 0; i < iterations; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS patterns */
        volatile int *addr1 = &p1[i * 3].counter;
        volatile double *addr2 = &p1[i * 2 + 1].values[(i % 3)];
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %[val1], %%eax\n\t"
            "addl %%eax, %[val2]\n\t"
            : [val2] "+m" (*addr1)
            : [val1] "m" (*addr2), "m" (local_vars[i % 16])
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS patterns */
        volatile char *tag_ptr = &p1[(i * 7) % 256].tags[(i * 3) % 8];
        
        /* Another asm with conflicting constraints */
        asm volatile (
            "movb %%al, %[tag]\n\t"
            "incb %[tag]\n\t"
            : [tag] "=m" (*tag_ptr)
            : "a" ((char)i)
            : "r12", "r13", "cc"
        );
        
        /* Nested pointer operations - RELOAD_FOR_INPADDR_ADDRESS */
        int **pptr = (int **)&p2[i % 128].pptr;
        modify_pptr(&pptr);
        
        /* Complex chain of address computations */
        volatile void *complex_addr = (volatile void *)&p1[i].values;
        compute_address(&complex_addr, i * sizeof(double));
        
        /* Use goto to break control flow */
        if (i == iterations / 2) {
            goto mid_loop;
        }
        
        continue;
        
        mid_loop:
        /* Different addressing mode after goto */
        volatile long long *big_ptr = &p1[(i * 5) % 256].big;
        
        asm volatile (
            "movq %[src], %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %[dst]\n\t"
            : [dst] "=m" (*big_ptr)
            : [src] "m" (*big_ptr)
            : "rax", "r12", "r13", "r14", "cc"
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS patterns */
        register int offset asm ("r15") = i * 8;
        volatile int *op_addr = (volatile int *)((char *)p1 + offset);
        
        asm volatile (
            "movl %[in], %%ebx\n\t"
            "movl %%ebx, %[out]\n\t"
            : [out] "=m" (*op_addr)
            : [in] "r" (offset), "m" (*index_ptr)
            : "ebx", "r15", "memory"
        );
    }
    
    /* Jump back to recompute with different base */
    if (iterations > 1) {
        p1 = &data_array[128];  /* Change base pointer */
        p2 = &ptr_array[64];
        goto compute_again;
    }
}

/* Another function with output address reloads */
static void stress_output_reloads(void) {
    register volatile double *out1 asm ("r12") = &data_array[0].values[0];
    register volatile double *out2 asm ("r13") = &data_array[128].values[0];
    
    /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    for (int i = 0; i < 4; i++) {
        volatile double *matrix_ptr = &ptr_array[i].matrix[i % 2][(i + 1) % 2];
        
        /* Assembly with output memory operand */
        asm volatile (
            "movsd %[src], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[dst]\n\t"
            : [dst] "=m" (*matrix_ptr)
            : [src] "m" (*out1)
            : "xmm0", "r12", "r13", "memory"
        );
        
        /* Chain of address computations */
        volatile void **addr_chain = (volatile void **)&ptr_array[i].vptr;
        *addr_chain = (volatile void *)out2;
        
        /* Another asm with multiple clobbers */
        asm volatile (
            "leaq %[base], %%rax\n\t"
            "movq %%rax, %[ptr]\n\t"
            : [ptr] "=m" (*addr_chain)
            : [base] "r" (out2)
            : "rax", "r12", "r13", "memory"
        );
    }
}

/* Function for RELOAD_FOR_OTHER_ADDRESS */
static void stress_other_address(void) {
    volatile static int global_buffer[64];
    register int *buf_ptr asm ("r12") = (int *)global_buffer;
    
    /* Complex indexing with multiple dimensions */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int index = (i * 8 + j) * sizeof(int);
            volatile int *elem = (volatile int *)((char *)buf_ptr + index);
            
            /* Inline asm with "other" address patterns */
            asm volatile (
                "movl %[idx], %%ecx\n\t"
                "movl %%ecx, %[mem]\n\t"
                : [mem] "=m" (*elem)
                : [idx] "r" (index)
                : "ecx", "r12", "memory"
            );
            
            /* Take address of address */
            int ***ppptr = (int ***)&ptr_array[i].pptr;
            modify_pptr(ppptr);
        }
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].values[1] = i * 2.5;
        data_array[i].values[2] = i * 3.5;
    }
    
    for (int i = 0; i < 128; i++) {
        ptr_array[i].pptr = (int **)&data_array[i].ptr;
        ptr_array[i].vptr = &data_array[i].counter;
    }
    
    /* Stress different reload patterns */
    stress_reloads(3);           /* Small iteration count to create patterns */
    stress_output_reloads();
    stress_other_address();
    
    /* Additional complex pattern in main */
    {
        register volatile char *byte_ptr asm ("r12") = (volatile char *)data_array;
        volatile int *int_ptr = (volatile int *)byte_ptr;
        
        /* Mixed addressing modes */
        for (int i = 0; i < 4; i++) {
            asm volatile (
                "movl %[in], %%eax\n\t"
                "movl %%eax, %[out]\n\t"
                : [out] "=m" (int_ptr[i * 16])
                : [in] "m" (int_ptr[i * 16 + 8])
                : "eax", "r12", "memory"
            );
        }
    }
    
    return 0;
}
