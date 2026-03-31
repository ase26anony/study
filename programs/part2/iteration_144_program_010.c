/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int counter;
    volatile double value;
    volatile char tag;
    volatile int64_t big;
    volatile float small;
};

/* Large volatile arrays */
static volatile struct MixedData data_array[1024];
static volatile int addr_array[2048];
static volatile double double_array[512];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(int **pp) {
    volatile int temp = **pp;
    **pp = temp + 1;
}

static void compute_address(void **addr_ptr) {
    volatile uintptr_t val = (uintptr_t)*addr_ptr;
    *addr_ptr = (void*)(val + 8);
}

/* Main stress function with complex addressing patterns */
static void stress_reload(void) {
    /* Register-bound pointers - force specific registers */
    register volatile struct MixedData *p1 asm ("r12") = &data_array[0];
    register volatile int *p2 asm ("r13") = &addr_array[0];
    register volatile double *p3 asm ("r14") = &double_array[0];
    
    /* Local variables that will have addresses taken */
    volatile int local1 = 42;
    volatile double local2 = 3.14159;
    volatile int local3 = 100;
    
    /* Complex addressing mode 1: array indexing with register base */
    int offset1 = 128;
    int offset2 = 256;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPUT */
    {
        /* Complex address computation */
        volatile struct MixedData *addr1 = p1 + offset1;
        volatile int *addr2 = (volatile int*)((char*)p2 + offset2 * sizeof(int));
        
        /* Inline assembly with memory operand constraints */
        asm volatile (
            "addl $1, %[mem1]\n\t"
            "fldl %[mem2]\n\t"
            : [mem1] "+m" (addr1->counter), [mem2] "+m" (addr2[16])
            : 
            : "memory", "r12", "r13", "r14", "cc"
        );
    }
    
    /* Jump to create control flow complexity */
    goto label1;
    
    /* Unreachable code that still affects compilation */
    {
        volatile int dummy = 0;
        dummy++;
    }
    
label1:
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    {
        /* More complex addressing with pointer arithmetic */
        register volatile double *p4 asm ("r15") = p3 + 64;
        
        /* Nested address computation */
        volatile double **ppd = (volatile double**)&p4;
        volatile int *indirect = (volatile int*)((uintptr_t)*ppd + 32);
        
        /* Inline assembly with conflicting constraints */
        asm volatile (
            "movq %[in], %%rax\n\t"
            "movq (%%rax), %%rbx\n\t"
            "addq $8, %%rbx\n\t"
            "movq %%rbx, %[out]\n\t"
            : [out] "=m" (indirect[4])
            : [in] "r" (&p4), "m" (p4)
            : "rax", "rbx", "memory", "r12", "r13", "r14", "r15", "cc"
        );
    }
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    {
        /* Take address of register variable */
        int *ptr_to_reg = (int*)&p2;
        
        /* Complex expression with address taken */
        volatile int complex_addr = *(int*)((char*)ptr_to_reg + local1);
        
        /* Inline assembly using the computed address */
        asm volatile (
            "movl %[addr], %%eax\n\t"
            "incl %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=m" (complex_addr)
            : [addr] "rm" (complex_addr)
            : "eax", "memory", "cc"
        );
    }
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS */
    {
        /* Chain of address computations */
        volatile int **pp1 = (volatile int**)&local1;
        volatile int **pp2 = (volatile int**)&local3;
        
        /* Function call with address-taken arguments */
        modify_pptr((int**)pp1);
        
        /* Inline assembly between address computations */
        asm volatile ("" : : : "r12", "r13");
        
        /* Another function call */
        compute_address((void**)pp2);
    }
    
    /* Loop with varying offsets to create multiple reload opportunities */
    for (int i = 0; i < 4; i++) {
        /* Different addressing modes in each iteration */
        volatile struct MixedData *elem = p1 + (i * 32);
        volatile int *int_elem = p2 + (i * 64);
        volatile double *double_elem = p3 + (i * 16);
        
        /* Mixed operations to force different reload types */
        switch (i % 3) {
            case 0:
                /* Force input address reloads */
                elem->counter = int_elem[i * 8];
                break;
            case 1:
                /* Force output address reloads */
                double_elem[4] = (double)elem->big;
                break;
            case 2:
                /* Force operand address reloads */
                asm volatile (
                    "movq %[src], %%rax\n\t"
                    "movq %%rax, %[dst]\n\t"
                    : [dst] "=m" (int_elem[12])
                    : [src] "m" (elem->value)
                    : "rax", "memory", "cc"
                );
                break;
        }
        
        /* Jump to create more control flow */
        if (i & 1) {
            goto inner_label;
        }
        
        continue;
        
    inner_label:
        /* Additional address computation in jumped-to block */
        volatile char *char_ptr = (volatile char*)elem + offset1;
        *char_ptr = (char)i;
    }
    
    /* Final complex addressing pattern */
    {
        /* Triple indirection */
        volatile void ***triple = (volatile void***)&p1;
        volatile void **double_ptr = *triple;
        volatile void *single_ptr = *double_ptr;
        
        /* Use in inline assembly */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "addq $16, %%r12\n\t"
            : 
            : [ptr] "r" (single_ptr)
            : "r12", "cc"
        );
    }
}

/* Additional stress functions for more coverage */
static void stress_more(void) {
    /* Register variables with different types */
    register volatile int *r1 asm ("r10") = &addr_array[512];
    register volatile double *r2 asm ("r11") = &double_array[128];
    
    /* Complex offset computation */
    int dynamic_offset = r1[0] & 0xFF;
    
    /* Force various reload types through inline assembly */
    asm volatile (
        /* Multiple memory operands with different constraints */
        "movl (%[base1],%[idx1],4), %%eax\n\t"
        "addl %%eax, (%[base2])\n\t"
        "fldl (%[base3])\n\t"
        "fstpl (%[base4])\n\t"
        : 
        : [base1] "r" (r1), [idx1] "r" (dynamic_offset),
          [base2] "r" (&addr_array[256]),
          [base3] "r" (r2),
          [base4] "r" (&double_array[64])
        : "eax", "memory", "r10", "r11", "st", "cc"
    );
    
    /* Address of register variable in memory operand */
    volatile int **pp_reg = (volatile int**)&r1;
    
    asm volatile (
        "movq %[pp], %%rax\n\t"
        "movq (%%rax), %%rbx\n\t"
        "movl (%%rbx), %%ecx\n\t"
        "movl %%ecx, %[out]\n\t"
        : [out] "=m" (addr_array[128])
        : [pp] "r" (pp_reg)
        : "rax", "rbx", "ecx", "memory", "cc"
    );
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1024; i++) {
        data_array[i].counter = i;
        data_array[i].value = i * 0.5;
        data_array[i].big = i * 1000LL;
    }
    
    for (int i = 0; i < 2048; i++) {
        addr_array[i] = i * 2;
    }
    
    for (int i = 0; i < 512; i++) {
        double_array[i] = i * 0.25;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_more();
    
    /* More inline assembly with complex addressing */
    {
        register volatile int *base asm ("r12") = &addr_array[0];
        volatile int index = 768;
        
        asm volatile (
            "leaq (%[b],%[i],4), %%rax\n\t"
            "movl (%%rax), %%ebx\n\t"
            "leaq 1024(%[b]), %%rcx\n\t"
            "movl %%ebx, (%%rcx)\n\t"
            : 
            : [b] "r" (base), [i] "r" (index)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
    }
    
    /* Function calls with complex address arguments */
    {
        volatile int x = 42;
        volatile int *px = &x;
        volatile int **ppx = &px;
        
        modify_pptr((int**)ppx);
        
        /* Chain of address computations */
        volatile int ***pppx = (volatile int***)&ppx;
        volatile void *addr_chain = (void*)((uintptr_t)*pppx + 16);
        
        compute_address(&addr_chain);
    }
    
    return 0;
}
