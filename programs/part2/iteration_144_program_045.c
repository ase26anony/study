/* reload_stress.c - Stress GCC's reload pass for uncovered switch cases */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile int* volatile ptr_array[50];
} Container;

/* Global volatile arrays to force memory accesses */
volatile Container containers[10];
volatile int global_buffer[1000];

/* Helper functions that take pointer-to-pointer arguments */
void update_pointer(int** pp, int* new_val) {
    *pp = new_val;
}

void complex_address_helper(volatile int**** pppp) {
    /* Force address computation */
    volatile int dummy = ****pppp;
    (void)dummy;
}

/* Function with complex addressing patterns */
void stress_reloads(void) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12") = &containers[0].arr[0];
    register volatile int* p2 asm ("r13") = &global_buffer[0];
    register volatile int* p3 asm ("r14") = &global_buffer[500];
    
    volatile int local_array[100];
    volatile int* local_ptrs[20];
    
    /* Complex addressing with multiple constraints */
    int offset1 = 10;
    int offset2 = 20;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    for (int i = 0; i < 5; i++) {
        /* Complex array indexing with register variables */
        volatile MixedType* addr1 = p1 + offset1 + i * 3;
        volatile int* addr2 = p2 + offset2 - i * 2;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %[val1], (%[mem1])\n\t"
            "movl %[val2], (%[mem2])\n\t"
            : 
            : [mem1] "r" (addr1), [val1] "r" (i),
              [mem2] "r" (addr2), [val2] "r" (i * 2)
            : "memory", "r12", "r13"
        );
        
        /* Use goto to create complex control flow */
        if (i & 1) {
            goto compute_other_address;
        }
        
        continue_label:
        /* Nested pointer indirection */
        int** pp = &local_ptrs[i];
        update_pointer(pp, (int*)addr1);
        
        /* Another inline asm with different constraints */
        asm volatile (
            "addl $1, %0\n\t"
            "movl %0, %1\n\t"
            : "+r" (offset1), "=m" (local_array[i])
            : 
            : "cc"
        );
        
        continue;
        
        compute_other_address:
        /* Different addressing mode using same registers */
        volatile int* addr3 = p3 + i * 7;
        
        asm volatile (
            "movq %[src], %%r15\n\t"
            "movl (%%r15), %[dst]\n\t"
            : [dst] "=r" (offset2)
            : [src] "r" (addr3)
            : "r15", "memory"
        );
        
        goto continue_label;
    }
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    {
        register volatile Container* c_ptr asm ("r12") = &containers[3];
        volatile int*** ppp;
        
        /* Complex pointer chain */
        ppp = (volatile int***)&c_ptr->ptr_array[10];
        
        /* Inline asm with output memory operand */
        int result;
        asm volatile (
            "movl $0x1234, %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            "leaq %[input], %%rbx\n\t"
            : [output] "=m" (*(volatile int*)(c_ptr->arr[5].c + 2))
            : [input] "r" (ppp)
            : "rax", "rbx", "memory", "r12"
        );
        
        /* Force operand address reloads */
        complex_address_helper((volatile int****)&ppp);
    }
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    {
        volatile int* addr_array[10];
        
        for (int i = 0; i < 10; i++) {
            /* Complex address computation */
            addr_array[i] = &local_array[i * 7 + 3] + (i & 3);
        }
        
        /* Inline asm with multiple memory constraints */
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "addl (%[addr2]), %%eax\n\t"
            "movl %%eax, (%[addr3])\n\t"
            : 
            : [addr1] "r" (addr_array[0]),
              [addr2] "r" (addr_array[5]),
              [addr3] "r" (addr_array[9])
            : "rax", "memory", "r12", "r13", "r14"
        );
        
        /* Jump to force register reallocation */
        goto final_block;
        
        mid_block:
        /* Different use of same registers */
        asm volatile (
            "movq %0, %%r12\n\t"
            "movq %1, %%r13\n\t"
            : 
            : "r" (&containers[5]),
              "r" (&global_buffer[100])
            : "r12", "r13"
        );
        return;
        
        final_block:
        /* Force other address reloads */
        volatile int**** complex_ptr = (volatile int****)&containers[2].ptr_array[15];
        
        asm volatile (
            "movq %[ptr], %%r15\n\t"
            : 
            : [ptr] "r" (complex_ptr)
            : "r15"
        );
        
        goto mid_block;
    }
}

/* Additional stress function with different patterns */
void more_reload_stress(void) {
    volatile double darray[200];
    volatile int* iptr = (volatile int*)&darray[0];
    
    /* Mix float and int accesses */
    for (int i = 0; i < 10; i++) {
        register volatile double* dp asm ("r12") = &darray[i * 3];
        register volatile int* ip asm ("r13") = iptr + i * 6;
        
        /* Inline asm with mixed constraints */
        double dval;
        asm volatile (
            "movsd (%[src]), %%xmm0\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "movl %%eax, (%[dst])\n\t"
            : "=t" (dval)
            : [src] "r" (dp), [dst] "r" (ip)
            : "xmm0", "eax", "memory", "r12", "r13"
        );
        
        /* Address taken and passed to function */
        update_pointer((int**)&ip, (int*)dp);
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1000; i++) {
        global_buffer[i] = i;
    }
    
    /* Call stress functions multiple times */
    for (int i = 0; i < 3; i++) {
        stress_reloads();
        more_reload_stress();
    }
    
    return 0;
}
