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
volatile Container containers[4];
volatile int global_buffer[256];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pptr(int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

void compute_address(void** addr, int offset) {
    volatile int dummy = *(int*)((char*)*addr + offset);
    (void)dummy;
}

/* Function with complex addressing patterns */
void stress_reloads(int iter) {
    /* Bind specific pointers to explicit registers */
    register MixedType* p1 asm ("r12");
    register int* p2 asm ("r13");
    register char* p3 asm ("r14");
    register void* p4 asm ("r15");
    
    /* Initialize register-bound pointers */
    p1 = (MixedType*)&containers[iter % 4].arr[0];
    p2 = (int*)&global_buffer[0];
    p3 = (char*)&containers[(iter + 1) % 4];
    p4 = (void*)&containers[(iter + 2) % 4];
    
    /* Complex pointer arithmetic with non-constant offsets */
    int offset1 = iter * 37;
    int offset2 = iter * 53;
    int offset3 = iter * 71;
    
    /* Block 1: Multiple address computations */
    {
        volatile MixedType* addr1 = &p1[offset1 % 50];
        volatile int* addr2 = &p2[offset2 % 200];
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, (%2)\n\t"
            : /* no outputs */
            : "r" (addr1), "r" (addr2)
            : "eax", "memory", "r12", "r13"
        );
        
        /* Use goto to create complex control flow */
        if (iter & 1) goto recompute_addrs;
    }
    
    /* Block 2: Different addressing mode */
    {
        volatile char* byte_ptr = &p3[offset3 % 1000];
        
        /* Another asm with different constraints */
        int temp;
        asm volatile (
            "movsbl (%1), %0\n\t"
            : "=r" (temp)
            : "m" (*byte_ptr)
            : "r14"
        );
        
        /* Nested function call with address-taken argument */
        int* local_ptr = (int*)byte_ptr;
        int** pptr = &local_ptr;
        int*** ppptr = &pptr;
        modify_pptr(ppptr);
    }
    
recompute_addrs:
    /* Reuse same registers for different purposes */
    {
        /* Force RELOAD_FOR_INPUT_ADDRESS type */
        volatile int64_t* addr3 = (int64_t*)&p1[offset2 % 30].d;
        
        /* Complex expression for address */
        void* complex_addr = (void*)((char*)addr3 + offset1 * 3 - offset2);
        
        /* Inline asm with multiple memory constraints */
        uint64_t result;
        asm volatile (
            "movq (%[addr]), %[res]\n\t"
            "rorq $8, %[res]\n\t"
            : [res] "=r" (result)
            : [addr] "m" (*(volatile uint64_t*)complex_addr)
            : "r12", "r15"
        );
        
        /* Pass computed address to helper */
        compute_address(&complex_addr, offset3);
    }
    
    /* Block 3: Output address reloads */
    {
        volatile int output_data;
        int* out_addr = &output_data;
        
        /* Inline asm with output memory operand */
        asm volatile (
            "movl %1, (%0)\n\t"
            : 
            : "r" (out_addr), "r" (iter)
            : "memory", "r13"
        );
        
        /* Chain of address computations */
        void* addr_chain = (void*)out_addr;
        for (int i = 0; i < 3; i++) {
            addr_chain = (void*)((char*)addr_chain + (i * 16));
            volatile int dummy = *(int*)addr_chain;
            (void)dummy;
        }
    }
    
    /* Block 4: Mixed addressing modes in loop */
    {
        volatile int* base_addrs[4];
        base_addrs[0] = (int*)p1;
        base_addrs[1] = p2;
        base_addrs[2] = (int*)p3;
        base_addrs[3] = (int*)p4;
        
        for (int i = 0; i < 4; i++) {
            /* Non-contiguous access pattern */
            int idx = (i * 17 + iter) % 50;
            volatile int* elem = &base_addrs[i][idx];
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "lock addl $1, (%0)\n\t"
                : 
                : "r" (elem)
                : "memory", "r12", "r13", "r14", "r15"
            );
            
            /* Jump to create control flow complexity */
            if (i == 2) goto skip_last;
        }
        
        skip_last:
        /* Empty target for goto */
        ;
    }
}

/* Main function that creates multiple reload scenarios */
int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = i * 3;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 100; j++) {
            containers[i].arr[j].a = i + j;
            containers[i].arr[j].b = (double)(i * j) / 3.0;
            containers[i].arr[j].d = (int64_t)i * j * 1000;
        }
    }
    
    /* Call stress function multiple times with different parameters */
    for (int i = 0; i < 8; i++) {
        stress_reloads(i);
        
        /* Additional inline complexity in main */
        register int* reg_ptr asm ("r12");
        reg_ptr = &global_buffer[i * 16];
        
        /* Complex address computation inline */
        volatile int* volatile_ptr = (volatile int*)(
            (char*)reg_ptr + i * 8 - 32
        );
        
        /* Assembly with operand address constraints */
        int input = i * 100;
        asm volatile (
            "leal (%1, %2, 4), %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            : 
            : "r" (volatile_ptr), "r" (input), "r" (i)
            : "eax", "memory", "r12"
        );
        
        /* Pointer-to-pointer manipulation */
        int local_var = i * 7;
        int* ptr1 = &local_var;
        int** ptr2 = &ptr1;
        modify_pptr(&ptr2);
    }
    
    /* Final complex block with gotos */
    {
        register void* jump_ptr asm ("r13");
        jump_ptr = &containers[0];
        
    block_a:
        compute_address(&jump_ptr, 64);
        goto block_c;
        
    block_b:
        {
            volatile int temp;
            asm volatile (
                "movl (%1), %0\n\t"
                : "=r" (temp)
                : "r" (jump_ptr)
                : "r13"
            );
        }
        goto block_d;
        
    block_c:
        jump_ptr = (void*)((char*)jump_ptr + 128);
        goto block_b;
        
    block_d:
        /* Final access */
        volatile int final_dummy = *(int*)jump_ptr;
        (void)final_dummy;
    }
    
    return 0;
}
