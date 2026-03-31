/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile long x;
    volatile short y[5];
    volatile MixedType* z;
} Container;

/* Global volatile arrays to force memory accesses */
volatile MixedType global_array[100];
volatile Container container_array[50];

/* Helper function taking pointer-to-pointer */
void modify_pointer(int** pp) {
    static int dummy = 42;
    *pp = &dummy;
}

/* Another helper with complex signature */
void address_chain(volatile void*** ppp, int offset) {
    static void* dummy_chain[3];
    **ppp = &dummy_chain[offset % 3];
}

/* Function to stress reloads with inline assembly */
void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12") = &global_array[0];
    register volatile Container* p2 asm ("r13") = &container_array[0];
    register int index asm ("r14") = iter;
    
    volatile int local_var = 0;
    volatile double local_dbl = 3.14;
    int* ptr_to_ptr = &local_var;
    
    /* Complex addressing computation block 1 */
    {
        volatile int* addr1 = &p1[index * 3 + 1].a;
        volatile double* addr2 = &p1[(index + 7) % 20].b;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (*addr1)
            : "r" (index), "m" (*addr2)
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to force control flow complexity */
        if (iter & 1) goto compute_other_address;
    }
    
    /* Different addressing mode */
    compute_other_address:
    {
        /* Use the same register-bound variable for different computation */
        volatile char* char_ptr = &p1[index % 10].c[(index * 2) % 7];
        
        /* Another asm with conflicting constraints */
        asm volatile (
            "movb $0x41, (%0)\n\t"
            "incb %1\n\t"
            : 
            : "r" (char_ptr), "m" (*char_ptr)
            : "r12", "memory"
        );
    }
    
    /* Nested pointer indirection */
    {
        int** pptr = &ptr_to_ptr;
        modify_pointer(pptr);
        
        /* Use the modified pointer in address computation */
        volatile void* vptr = (volatile void*)*pptr;
        volatile void** vpptr = &vptr;
        volatile void*** vppptr = &vpptr;
        
        address_chain(vppptr, index);
        
        /* Access through the chain */
        asm volatile (
            "movl $99, %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            : 
            : "r" (*vpptr)
            : "eax", "memory"
        );
    }
    
    /* Switch to using p2 with different addressing pattern */
    {
        volatile long* long_ptr = &p2[(index * 5) % 30].x;
        volatile short* short_ptr = &p2[(index + 3) % 20].y[(index + 1) % 5];
        
        /* Asm with multiple memory outputs */
        asm volatile (
            "mov %1, %%ax\n\t"
            "addw %%ax, %0\n\t"
            "movl $100, %2\n\t"
            : "+m" (*short_ptr), "+m" (*long_ptr)
            : "r" (index)
            : "ax", "r12", "r13", "memory"
        );
    }
    
    /* Complex loop with address recomputation */
    for (int i = 0; i < 3; i++) {
        /* Rebind register for new computation */
        register int offset asm ("r15") = i * 7 + index;
        
        volatile MixedType* elem = &global_array[offset % 50];
        volatile int** d_ptr = (volatile int**)&elem->d;
        
        /* Jump within loop */
        if (i == 1) goto loop_middle;
        
        loop_start:
        /* Inline asm that clobbers address registers */
        asm volatile (
            "movl %1, (%0)\n\t"
            : 
            : "r" (d_ptr), "r" (&local_var)
            : "r12", "r13", "r14", "r15", "memory"
        );
        
        if (i == 0) goto loop_end;
        
        loop_middle:
        /* Different addressing mode in middle of loop */
        volatile double* b_ptr = &elem->b;
        asm volatile (
            "movsd %1, %0\n\t"
            : "=m" (*b_ptr)
            : "m" (local_dbl)
            : "memory"
        );
        
        loop_end:
        /* Empty to complete the control flow */
        ;
    }
    
    /* Final complex expression forcing multiple reload types */
    {
        volatile int* final_addr = 
            &global_array[(index * 11 + 17) % 80].a;
        
        /* Asm with operand address constraints */
        asm volatile (
            "leal (%1, %2, 4), %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=m" (*final_addr)
            : "r" (final_addr), "r" (index)
            : "ecx", "r12", "memory"
        );
    }
}

/* Additional stress function for output address reloads */
void stress_output_addresses(void) {
    volatile int results[10];
    register volatile int* out_ptr asm ("r12") = &results[0];
    
    for (int i = 0; i < 5; i++) {
        volatile int* current = out_ptr + i * 2;
        
        /* Asm with output memory operand */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*current)
            : "r" (i)
            : "eax", "r12", "memory"
        );
        
        /* Jump to force address recomputation */
        if (i == 2) goto special_case;
        
        continue;
        
        special_case:
        /* Different output addressing */
        asm volatile (
            "movl $0xDEADBEEF, %0\n\t"
            : "=m" (results[9])
            :
            : "memory"
        );
    }
}

/* Main function that orchestrates the stress test */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 100; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
    }
    
    for (int i = 0; i < 50; i++) {
        container_array[i].x = i * 100;
        container_array[i].z = &global_array[i % 20];
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int iter = 0; iter < 8; iter++) {
        stress_reloads(iter);
        
        if (iter % 3 == 0) {
            stress_output_addresses();
        }
        
        /* Small switch to vary control flow */
        switch (iter % 4) {
            case 0:
                /* Additional address computation */
                {
                    register volatile MixedType* p asm ("r12") = 
                        &global_array[iter * 7 % 50];
                    volatile int* addr = &p->a;
                    asm volatile ("" : : "r" (addr) : "r12");
                }
                break;
            case 1:
                /* Pointer-to-pointer chain */
                {
                    volatile int val = iter;
                    int* p1 = &val;
                    int** p2 = &p1;
                    int*** p3 = &p2;
                    modify_pointer(*p3);
                }
                break;
            default:
                /* Mixed addressing */
                {
                    volatile Container* c = &container_array[iter % 30];
                    volatile short* s = &c->y[(iter + 2) % 5];
                    asm volatile ("incw %0" : "+m" (*s) : : "memory");
                }
        }
    }
    
    return 0;
}
