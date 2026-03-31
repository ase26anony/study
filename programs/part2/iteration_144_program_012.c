/* reload_stress.c - Stress GCC's reload pass with complex addressing modes */
#include <stdint.h>

/* Volatile structures to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType arr[32];
    volatile long long big[16];
} BigStruct;

/* Global volatile arrays */
static volatile BigStruct global_data[8];
static volatile int* volatile global_ptrs[64];

/* Helper functions that take pointer-to-pointer */
static void modify_pptr(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

static void compute_address(void** addr, int offset) {
    *addr = (void*)((uintptr_t)*addr + offset);
}

/* Function with complex addressing patterns */
static void stress_reloads(int iter) {
    /* Bind specific variables to registers */
    register volatile MixedType* p1 asm ("r12") = &global_data[0].arr[0];
    register volatile int* p2 asm ("r13") = (volatile int*)&global_data[1];
    register volatile char* p3 asm ("r14") = (volatile char*)&global_data[2];
    
    volatile int local_array[128];
    volatile double local_doubles[64];
    
    /* Complex addressing computation block */
    {
        /* Force RELOAD_FOR_INPUT_ADDRESS */
        int idx1 = iter * 3 + 1;
        int idx2 = iter * 7 + 2;
        
        /* Multiple addressing modes in same expression */
        volatile int* addr1 = &p1[idx1].a + idx2;
        volatile double* addr2 = (volatile double*)((char*)&p1[idx2].b + iter);
        
        /* Inline asm with memory operands and clobbers */
        asm volatile (
            "movl %[val1], (%[mem1])\n\t"
            "movsd %[val2], (%[mem2])\n\t"
            : 
            : [mem1] "r" (addr1), [val1] "r" (iter),
              [mem2] "r" (addr2), [val2] "x" ((double)iter)
            : "memory", "r12", "r13", "r14"
        );
    }
    
    /* Jump to create control flow complexity */
    goto label1;
    
    /* Unreachable code that still affects analysis */
    {
        volatile int unused = 0;
        unused = unused + 1;
    }
    
label1:
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    {
        register volatile long long* p4 asm ("r12") = &global_data[3].big[0];
        
        /* Complex offset computation */
        int offset = (iter << 2) | (iter & 3);
        volatile long long* out_addr = p4 + offset * 2;
        
        /* Inline asm with output memory operand */
        long long result;
        asm volatile (
            "movq (%[in]), %%rax\n\t"
            "imulq $37, %%rax, %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            : [out] "=m" (*out_addr)
            : [in] "r" (&global_data[4].big[iter & 7])
            : "rax", "memory", "r12"
        );
    }
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    {
        volatile int** ptr_to_ptr = &global_ptrs[iter & 63];
        volatile int* volatile* volatile pp = ptr_to_ptr;
        
        /* Nested addressing */
        int*** ppp = (int***)&pp;
        if (*ppp) {
            /* Complex chain of address computations */
            int* final = **(int***)ppp + iter;
            
            /* Multiple asm blocks with clobbers */
            asm volatile ("" : : "r" (final) : "r12", "r13");
            
            /* Call function with address-taken argument */
            modify_pptr((int**)ppp);
        }
    }
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS */
    {
        /* Multiple register-bound pointers used in same expression */
        register int* r1 asm ("r12") = (int*)&local_array[0];
        register int* r2 asm ("r13") = (int*)&local_doubles[0];
        
        /* Extremely complex addressing expression */
        volatile int* complex_addr = (int*)(
            (uintptr_t)r1 + 
            ((uintptr_t)r2 >> 3) + 
            iter * sizeof(MixedType)
        );
        
        /* Inline asm with multiple constraints on same operand */
        int dummy;
        asm volatile (
            "leal (%[base], %[index], 4), %[dummy]\n\t"
            "movl %[dummy], (%[addr])\n\t"
            : [dummy] "=&r" (dummy)
            : [base] "r" (r1), [index] "r" (iter), 
              [addr] "r" (complex_addr)
            : "memory", "r12", "r13"
        );
    }
    
    /* Force RELOAD_FOR_OPADDR_ADDR */
    {
        /* Use goto to create disjoint register usage */
        if (iter & 1) {
            goto alternate_path;
        }
        
        /* Block A: Uses r12 for one purpose */
        register volatile int* reg_a asm ("r12") = &local_array[16];
        *reg_a = iter;
        
        asm volatile ("" : : : "r12");  /* Clobber r12 */
        
        goto merge_point;
        
    alternate_path:
        /* Block B: Uses r12 for different purpose */
        register volatile double* reg_b asm ("r12") = &local_doubles[16];
        *reg_b = (double)iter;
        
        asm volatile ("" : : : "r12");  /* Clobber r12 */
        
    merge_point:
        /* Force reload of address for r12-based access */
        volatile int* recovered = (volatile int*)((uintptr_t)&local_array[0] + iter * 4);
        
        /* Inline asm that needs both address and value in registers */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, (%[addr])\n\t"
            : 
            : [addr] "r" (recovered)
            : "eax", "memory", "r12"
        );
    }
    
    /* Force RELOAD_FOR_OTHER_ADDRESS */
    {
        /* Mixed-type, non-contiguous accesses */
        for (int i = 0; i < 4; i++) {
            /* Different addressing mode each iteration */
            volatile char* base;
            switch (i) {
                case 0:
                    base = (volatile char*)&global_data[0];
                    break;
                case 1:
                    base = (volatile char*)&global_data[1].arr[iter & 31];
                    break;
                case 2:
                    base = p3 + (iter * 13) & 255;
                    break;
                case 3:
                    base = (volatile char*)p2;
                    break;
            }
            
            /* Access with misaligned offset */
            volatile int* misaligned = (volatile int*)(base + 1);
            
            /* Assembly with explicit address register usage */
            asm volatile (
                "movl (%[ptr]), %%ebx\n\t"
                "roll $3, %%ebx\n\t"
                "movl %%ebx, (%[ptr])\n\t"
                : 
                : [ptr] "r" (misaligned)
                : "ebx", "memory", "r12", "r13", "r14"
            );
        }
    }
}

/* Secondary stress function */
static void more_complex_addressing(int seed) {
    volatile MixedType stack_var[4];
    
    /* Bind to registers and use in address computations */
    register volatile MixedType* rbase asm ("r12") = &stack_var[0];
    register int rindex asm ("r13") = seed;
    
    /* Complex expression forcing address reloads */
    for (int i = 0; i < 3; i++) {
        /* Different addressing mode each iteration */
        volatile int* addr;
        
        if (i == 0) {
            /* Array indexing with register base and index */
            addr = &rbase[rindex & 3].a;
        } else if (i == 1) {
            /* Pointer arithmetic with cast */
            addr = (volatile int*)((char*)rbase + (rindex * sizeof(MixedType)) + 8);
        } else {
            /* Nested structure member access */
            addr = (volatile int*)(&rbase->d);
        }
        
        /* Function call with address computation */
        void* vaddr = (void*)addr;
        compute_address(&vaddr, i * 4);
        
        /* Inline asm using the computed address */
        int temp;
        asm volatile (
            "movl (%[addr]), %[temp]\n\t"
            "notl %[temp]\n\t"
            "movl %[temp], (%[addr])\n\t"
            : [temp] "=r" (temp)
            : [addr] "r" (vaddr)
            : "memory", "r12", "r13"
        );
        
        /* Clobber address registers between uses */
        asm volatile ("" : : : "r12", "r13");
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 8; i++) {
        global_data[i].arr[0].a = i;
        global_data[i].arr[0].b = (double)i;
    }
    
    /* Call stress functions multiple times with different parameters */
    for (int i = 0; i < 8; i++) {
        stress_reloads(i);
        more_complex_addressing(i * 17 + 1);
        
        /* Additional inline complex addressing */
        {
            register volatile int* r15 asm ("r15") = (volatile int*)&global_data[i];
            
            /* Force address reload with volatile and asm */
            volatile int** ptrptr = (volatile int**)&global_ptrs[i * 8];
            *ptrptr = (volatile int*)r15;
            
            asm volatile (
                "movq %[ptr], %%rsi\n\t"
                "movq (%%rsi), %%rdi\n\t"
                "addq $8, %%rdi\n\t"
                "movq %%rdi, (%%rsi)\n\t"
                : 
                : [ptr] "r" (ptrptr)
                : "rsi", "rdi", "memory", "r15"
            );
        }
    }
    
    return 0;
}
