/* reload_stress.c - Stress test for GCC reload pass */
#include <stdint.h>

/* Volatile structures to prevent optimization */
volatile struct MixedData {
    int counter;
    double values[3];
    char tags[8];
    void *next;
} data_array[100];

volatile struct PointerChain {
    int **pptr;
    long *lptr;
    volatile char *cptr;
} chain_array[50];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pointer(int ***ppp) {
    volatile static int dummy = 42;
    *ppp = (int **)&dummy;
}

static void compute_address(volatile void **addr_ptr, int offset) {
    *addr_ptr = (volatile void *)((char *)*addr_ptr + offset);
}

/* Main stress function */
static void stress_reload(void) {
    /* Bind specific registers for address computation */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile struct PointerChain *p2 asm ("r13") = &chain_array[0];
    register int *index_ptr asm ("r14");
    register volatile char *char_ptr asm ("r15");
    
    int i, j;
    volatile int temp;
    volatile double dtemp;
    int *local_ptr = &temp;
    int **pptr = &local_ptr;
    
    /* Complex addressing mode 1: array indexing with register base */
    for (i = 0; i < 5; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS pattern */
        temp = p1[i * 7 + 3].counter + i;
        
        /* Inline assembly with memory operand and clobber */
        asm volatile (
            "movl %[mem], %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[mem]"
            : [mem] "=m" (p1[i * 7 + 3].counter)
            : 
            : "eax", "r12", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (i & 1) goto compute_offset;
        
        back_from_goto:
        /* Use the same register for different address computation */
        char_ptr = (volatile char *)&p1[i].tags[0];
        
        /* Another asm with conflicting constraints */
        asm volatile (
            "movb (%[addr]), %%al\n\t"
            "orb $0x20, %%al\n\t"
            "movb %%al, (%[addr])"
            : 
            : [addr] "r" (char_ptr)
            : "eax", "r15", "memory"
        );
        
        continue;
        
    compute_offset:
        /* RELOAD_FOR_INPADDR_ADDRESS pattern */
        compute_address((volatile void **)&p1, i * sizeof(struct MixedData));
        goto back_from_goto;
    }
    
    /* Nested pointer chains for RELOAD_FOR_OUTPUT_ADDRESS */
    for (j = 0; j < 3; j++) {
        int **temp_pptr;
        
        /* Take address of register variable's address */
        modify_pointer(&pptr);
        
        /* Complex expression for address */
        index_ptr = (int *)((char *)p2 + j * 16 + 8);
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movq (%[src]), %%rax\n\t"
            "movq %%rax, (%[dst])\n\t"
            "addq $8, %%rax\n\t"
            "movq %%rax, (%[src])"
            : 
            : [src] "r" (index_ptr), [dst] "r" (&p2[j].lptr)
            : "rax", "r14", "r13", "memory"
        );
        
        /* RELOAD_FOR_OPERAND_ADDRESS pattern */
        temp_pptr = &local_ptr;
        *temp_pptr = (int *)&data_array[j * 10].counter;
        
        /* Another asm with explicit clobbers */
        asm volatile (
            "movl (%[addr]), %%ebx\n\t"
            "imull $3, %%ebx\n\t"
            "movl %%ebx, (%[addr])"
            : 
            : [addr] "r" (*temp_pptr)
            : "ebx", "memory"
        );
    }
    
    /* Mixed-type access with volatile */
    p1 = &data_array[20];
    for (i = 0; i < 4; i++) {
        /* Force address reload by using same register differently */
        dtemp = p1->values[i % 3];
        
        /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
        asm volatile (
            "movsd %[in], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=m" (p1->values[(i + 1) % 3])
            : [in] "m" (dtemp)
            : "xmm0", "r12", "memory"
        );
        
        /* Switch between different base pointers */
        if (i == 2) {
            p1 = &data_array[30];
            goto skip_recalc;
        }
        
        recalc:
        p1 = (volatile struct MixedData *)((char *)p1 + sizeof(double));
        
        skip_recalc:
        /* Access with different offset */
        temp = *(int *)((char *)p1 + 4);
        
        if (i == 2) goto recalc;
    }
    
    /* Final complex pattern combining multiple techniques */
    {
        register int offset asm ("ebx") = 64;
        volatile int *dynamic_addr;
        
        /* RELOAD_FOR_OTHER_ADDRESS pattern */
        dynamic_addr = (volatile int *)((char *)&data_array[0] + offset);
        
        asm volatile (
            "lock addl $1, %[mem]"
            : [mem] "+m" (*dynamic_addr)
            : 
            : "cc", "ebx", "memory"
        );
        
        /* Chain of address computations */
        int ***ppptr = &pptr;
        modify_pointer(ppptr);
        
        /* Force output address reload */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "movl $99, (%%r12)"
            : 
            : [ptr] "r" (*pptr)
            : "r12", "memory"
        );
    }
}

/* Secondary stress function with different patterns */
static void more_reload_stress(void) {
    volatile int matrix[10][10];
    register int (*row_ptr)[10] asm ("r12") = &matrix[0];
    register int i asm ("r13") = 0;
    register int j asm ("r14") = 0;
    
    /* Nested loops with complex array addressing */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            /* RELOAD_FOR_OPADDR_ADDR pattern */
            int *elem_ptr = &row_ptr[i][j];
            
            asm volatile (
                "movl (%[base], %[index], 4), %%eax\n\t"
                "addl %%eax, %%eax\n\t"
                "movl %%eax, (%[base], %[index], 4)"
                : 
                : [base] "r" (row_ptr[i]), [index] "r" ((long)j)
                : "eax", "r12", "r13", "r14", "memory"
            );
            
            /* Jump to disrupt register allocation */
            if (j == 5 && i == 5) goto special_case;
            
            continue;
            
        special_case:
            /* Change base register usage */
            row_ptr = &matrix[5];
            asm volatile ("" : : : "r12", "memory");
        }
    }
    
    /* Pointer-to-pointer chain */
    {
        int x = 42;
        int *p = &x;
        int **pp = &p;
        int ***ppp = &pp;
        
        /* Force address reload through multiple indirections */
        asm volatile (
            "movq %[ppp], %%rax\n\t"
            "movq (%%rax), %%rbx\n\t"
            "movq (%%rbx), %%rcx\n\t"
            "addl $1, (%%rcx)"
            : 
            : [ppp] "r" (ppp)
            : "rax", "rbx", "rcx", "memory"
        );
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        data_array[i].counter = i;
        data_array[i].values[0] = i * 1.5;
        data_array[i].tags[0] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 50; i++) {
        chain_array[i].lptr = (long *)&data_array[i].counter;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    more_reload_stress();
    
    /* Additional patterns in main */
    {
        register volatile int *reg_ptr asm ("r12") = &data_array[0].counter;
        volatile int result;
        
        /* Complex address computation in loop */
        for (int k = 0; k < 3; k++) {
            /* Force different reload types */
            asm volatile (
                "movl (%[base], %[idx], 4), %%eax\n\t"
                "movl %%eax, %[res]"
                : [res] "=r" (result)
                : [base] "r" (reg_ptr), [idx] "r" (k * 10)
                : "eax", "r12", "memory"
            );
            
            /* Use result in address computation */
            int *new_addr = (int *)((char *)reg_ptr + result);
            
            asm volatile (
                "movl $0xDEADBEEF, (%[addr])"
                : 
                : [addr] "r" (new_addr)
                : "memory"
            );
        }
    }
    
    return 0;
}
