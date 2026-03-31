/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    void *next;
} data_array[100];

volatile struct AddrChain {
    int **ptr_to_ptr;
    long offsets[4];
    volatile struct AddrChain *link;
} addr_chain[50];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int ***ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(volatile void **addr, int offset) {
    *addr = (volatile void *)((char *)*addr + offset);
}

/* Main stress function */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].tags[0] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        addr_chain[i].ptr_to_ptr = (int **)&data_array[i].counter;
        addr_chain[i].offsets[0] = i * 16;
    }
    
    /* Force specific registers for address computation */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct AddrChain *p2 asm ("r13") = &addr_chain[0];
    register int *index_reg asm ("r14") = (int *)&data_array[10].counter;
    
    /* Complex addressing mode computations */
    volatile int result = 0;
    
    /* Block 1: Multiple address reload types */
    {
        /* RELOAD_FOR_INPUT_ADDRESS pattern */
        volatile double *addr1 = &p1[(uintptr_t)index_reg % 50].values[1];
        volatile char *addr2 = &p1[(uintptr_t)index_reg % 50 + 3].tags[2];
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movq (%[a1]), %%rax\n\t"
            "addq (%[a2]), %%rax\n\t"
            "movq %%rax, %[res]"
            : [res] "=m" (result)
            : [a1] "r" (addr1), [a2] "r" (addr2)
            : "rax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to force register reallocation */
        goto block2;
    }
    
block1_return:
    /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
    {
        volatile int **out_addr = (volatile int **)&p2[5].ptr_to_ptr;
        
        /* Complex address computation with multiple registers */
        register long offset asm ("r15") = (long)(p1[10].values[0] * 2.0);
        volatile int *final_addr = (volatile int *)((char *)out_addr + offset);
        
        asm volatile (
            "movl $0x12345678, (%[addr])"
            : 
            : [addr] "r" (final_addr)
            : "memory", "r15"
        );
    }
    
    /* Nested function calls with address-taken arguments */
    {
        int local_var = 42;
        int *ptr_to_local = &local_var;
        int **pptr = &ptr_to_local;
        
        /* This chain may require RELOAD_FOR_INPADDR_ADDRESS */
        modify_pptr(&pptr);
        
        /* Use result in another complex address */
        volatile void *complex_addr = (void *)((char *)p1 + (local_var * 8));
        compute_address(&complex_addr, result);
    }
    
    return 0;

block2:
    /* RELOAD_FOR_OPERAND_ADDRESS pattern */
    {
        /* Force address computation with register constraints */
        register volatile struct MixedData *p3 asm ("r12") = &data_array[20];
        
        /* Different use of same register for different base */
        volatile double *addr3 = &p3->values[(uintptr_t)p2 % 3];
        
        asm volatile (
            "movsd (%[addr]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=m" (data_array[0].values[0])
            : [addr] "r" (addr3)
            : "xmm0", "r12", "memory"
        );
        
        /* Jump back */
        goto block1_return;
    }
}

/* Additional stress function for more patterns */
void extra_stress(void) {
    /* RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_OPADDR_ADDR patterns */
    volatile int buffer[256];
    
    /* Complex loop with scattered accesses */
    for (register int i asm ("r12") = 0; i < 100; i++) {
        /* Non-contiguous access pattern */
        volatile int *elem = &buffer[(i * 17) % 256];
        
        /* Inline asm with both memory and register constraints */
        register int temp asm ("r13");
        asm volatile (
            "movl (%[mem]), %[reg]\n\t"
            "imull $0x3, %[reg], %[reg]\n\t"
            "movl %[reg], (%[mem])"
            : [reg] "=r" (temp), [mem] "+m" (*elem)
            : 
            : "r12", "r13"
        );
        
        /* Address computation that may need reloading */
        volatile int **addr_of_addr = (volatile int **)&buffer[(i * 13) % 256];
        *addr_of_addr = (volatile int *)elem;
        
        /* Small conditional jump to complicate control flow */
        if (i & 1) {
            goto odd_case;
        } else {
            goto even_case;
        }
        
    odd_case:
        /* Different address computation using same registers */
        volatile double *dbl_ptr = (volatile double *)&buffer[(i * 23) % 256];
        asm volatile ("" : : "r" (dbl_ptr) : "r12");
        continue;
        
    even_case:
        /* Yet another address pattern */
        volatile char *char_ptr = (volatile char *)&buffer[(i * 7) % 256];
        asm volatile ("" : : "r" (char_ptr) : "r13");
        continue;
    }
    
    /* Multiple memory operands with conflicting constraints */
    {
        volatile int x = 1, y = 2, z = 3;
        
        asm volatile (
            "movl %[x], %%eax\n\t"
            "addl %[y], %%eax\n\t"
            "subl %[z], %%eax\n\t"
            "movl %%eax, %[x]"
            : [x] "+m" (x), [y] "+m" (y)
            : [z] "r" (z)
            : "eax", "memory"
        );
    }
}
