/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int32_t a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType *ptr;
    volatile int offset;
} PtrWrapper;

/* Global volatile arrays to force memory accesses */
volatile MixedType global_array[256];
volatile PtrWrapper wrappers[16];

/* Helper functions that take pointer-to-pointer arguments */
void update_pointer(volatile MixedType ***ppp) {
    volatile MixedType **temp = *ppp;
    if (temp && *temp) {
        /* Force memory access */
        asm volatile("" : "+m" (**temp));
    }
}

void compute_address(volatile int64_t **addr_ptr, int offset) {
    if (*addr_ptr) {
        *addr_ptr = (volatile int64_t *)((char *)(*addr_ptr) + offset);
    }
}

/* Function with complex addressing patterns */
void stress_reloads(void) {
    /* Bind specific pointers to explicit registers */
    register volatile MixedType *p1 asm ("r12") = &global_array[0];
    register volatile MixedType *p2 asm ("r13") = &global_array[128];
    register volatile int *index asm ("r14") = &wrappers[0].offset;
    
    volatile int64_t accumulator = 0;
    volatile MixedType **ptr_to_ptr = &p1;
    
    /* Complex control flow with goto */
    goto start_block;
    
    /* Label for jumping back */
    loop_back:;
    
    /* Block 1: Complex array indexing with register-bound pointers */
    {
        int offset = (*index) * 3 + 7;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        volatile MixedType *temp = p1 + offset;
        accumulator += temp->d;
        
        /* Inline assembly with memory operand and clobbers */
        asm volatile (
            "movq (%[ptr]), %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, (%[ptr])"
            : [ptr] "+m" (temp->d)
            : 
            : "rax", "r12", "r13", "r14", "memory"
        );
        
        /* Nested function call with address-taken argument */
        update_pointer(&ptr_to_ptr);
    }
    
    goto next_block;
    
    start_block:;
    /* Initialize index */
    *index = 1;
    
    /* Block 2: Different addressing mode */
    {
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        volatile double *dp = &p2->b;
        
        /* Complex pointer arithmetic */
        char *char_ptr = (char *)p2;
        char_ptr += (*index) * sizeof(MixedType) + 8;
        
        /* Inline assembly with multiple constraints */
        volatile double result;
        asm volatile (
            "movsd (%[src]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[dst]"
            : [dst] "=m" (result)
            : [src] "r" (char_ptr)
            : "xmm0", "r12", "r13", "memory"
        );
        
        *dp = result;
    }
    
    next_block:;
    
    /* Block 3: Address computation chain */
    {
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        volatile int64_t *addr_array[4];
        addr_array[0] = &p1->d;
        addr_array[1] = &p2->d;
        
        /* Complex chain of address computations */
        volatile int64_t **current = &addr_array[0];
        compute_address(current, 16);
        
        /* Use computed address */
        if (*current) {
            accumulator += **current;
        }
        
        /* Another inline assembly with conflicting constraints */
        asm volatile (
            "leaq (%[base],%[index],8), %%r15\n\t"
            "movq (%%r15), %%rax"
            : 
            : [base] "r" (global_array), [index] "r" (*index)
            : "r15", "rax", "memory"
        );
    }
    
    /* Block 4: Operand address reloads */
    {
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        volatile MixedType *local_ptr = p1 + *index;
        
        /* Inline assembly that uses the address in multiple ways */
        int64_t val1, val2;
        asm volatile (
            "movq %[addr], %%rbx\n\t"
            "movq (%%rbx), %[out1]\n\t"
            "leaq 16(%%rbx), %%rcx\n\t"
            "movq (%%rcx), %[out2]"
            : [out1] "=r" (val1), [out2] "=r" (val2)
            : [addr] "m" (local_ptr)
            : "rbx", "rcx", "memory"
        );
        
        accumulator += val1 + val2;
    }
    
    /* Block 5: Other address reloads */
    {
        /* Force RELOAD_FOR_OTHER_ADDRESS */
        volatile char *byte_ptr = (volatile char *)p2;
        byte_ptr += (*index) * 3;
        
        /* Multiple memory accesses with different offsets */
        for (int i = 0; i < 3; i++) {
            accumulator += byte_ptr[i * 7];
        }
        
        /* Assembly that clobbers address registers */
        asm volatile (
            "movq %[ptr], %%r12\n\t"
            "addb $1, (%%r12)"
            : 
            : [ptr] "r" (byte_ptr)
            : "r12", "memory"
        );
    }
    
    /* Loop control with goto */
    static int counter = 0;
    if (counter++ < 2) {
        goto loop_back;
    }
}

/* Additional stress function with different patterns */
void more_stress(void) {
    register volatile int *r1 asm ("r10") = &global_array[0].a;
    register volatile double *r2 asm ("r11") = &global_array[64].b;
    
    /* Switch between different addressing modes */
    volatile int mode = 0;
    
    mode_switch:
    if (mode == 0) {
        /* Mode 0: Indexed addressing */
        volatile int idx = 3;
        volatile int *addr = r1 + idx * 2;
        
        asm volatile (
            "imull $7, (%[addr])"
            : [addr] "+m" (*addr)
            : 
            : "r10", "r11", "memory"
        );
        
        mode = 1;
        goto mode_switch;
    } else {
        /* Mode 1: Base + offset addressing */
        char *base = (char *)r2;
        base += 24;
        
        asm volatile (
            "movsd (%[base]), %%xmm1\n\t"
            "mulsd %%xmm1, %%xmm1"
            : 
            : [base] "r" (base)
            : "xmm1", "r10", "r11", "memory"
        );
    }
    
    /* Complex expression with address taken */
    volatile int **pp = &r1;
    update_pointer((volatile MixedType ***)&pp);
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
        global_array[i].d = i * 1000LL;
        for (int j = 0; j < 7; j++) {
            global_array[i].c[j] = i + j;
        }
    }
    
    for (int i = 0; i < 16; i++) {
        wrappers[i].ptr = &global_array[i * 16];
        wrappers[i].offset = i;
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    more_stress();
    
    /* Additional inline stress in main */
    {
        register volatile MixedType *mp asm ("r15") = &global_array[32];
        
        /* Complex addressing chain */
        volatile char *cp = (volatile char *)mp;
        cp += mp->a * 2 + 8;
        
        asm volatile (
            "movb $42, (%[ptr])"
            : 
            : [ptr] "r" (cp)
            : "r15", "memory"
        );
        
        /* Nested pointer access */
        volatile MixedType **mpp = &mp;
        **mpp = global_array[0];
    }
    
    return 0;
}
