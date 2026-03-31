/* reload_stress.c - Stress GCC's reload pass for coverage testing */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
struct MixedData {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
};

struct NestedPtr {
    volatile struct MixedData** ptr_to_ptr;
    volatile long long big;
};

/* Global volatile arrays to force memory accesses */
static volatile struct MixedData global_array[256];
static volatile struct NestedPtr global_nested[128];

/* Helper functions that take pointer-to-pointer arguments */
static void modify_pptr(volatile int*** ppp) {
    volatile int dummy = ***ppp;
    (void)dummy;
}

static void compute_address(volatile void** addr_store, volatile void* base, int offset) {
    *addr_store = (volatile char*)base + offset * sizeof(struct MixedData);
}

/* Main stress function with complex addressing patterns */
static void stress_reload(void) {
    /* Bind specific variables to registers to create conflicts */
    register volatile struct MixedData* p1 asm ("r12") = &global_array[0];
    register volatile struct NestedPtr* p2 asm ("r13") = &global_nested[0];
    register int index asm ("r14") = 64;
    
    volatile int local_var = 42;
    volatile int* local_ptr = &local_var;
    volatile int** local_pptr = &local_ptr;
    
    /* Complex addressing mode 1: RELOAD_FOR_INPUT_ADDRESS */
    {
        volatile int* addr1;
        /* Compute address with register-bound pointer and non-constant offset */
        compute_address((volatile void**)&addr1, p1, index * 2 + 3);
        
        /* Inline assembly with memory operand and clobbered address register */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, (%%eax)\n\t"
            : /* no outputs */
            : "m" (*addr1)
            : "eax", "r12", "memory"
        );
        
        /* Jump to create control flow complexity */
        goto label1;
        
        /* Unreachable code to create more basic blocks */
        {
            volatile int dummy __attribute__((unused)) = 0;
        }
    }
    
label1:
    /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
    {
        volatile int result;
        volatile int* out_addr;
        
        /* Complex address computation using multiple registers */
        out_addr = (volatile int*)((char*)p1 + index * sizeof(struct MixedData) + 8);
        
        /* Inline assembly with output memory operand */
        asm volatile (
            "movl $99, %0\n\t"
            : "=m" (*out_addr)
            :
            : "r12", "r13"
        );
        
        /* Use goto to jump over code */
        if (local_var > 0) {
            goto label2;
        }
    }
    
    /* This block will be skipped */
    {
        volatile int unused = 0;
        (void)unused;
    }
    
label2:
    /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS patterns */
    {
        volatile int** inout_addr;
        volatile int temp;
        
        /* Complex expression for address-of-address */
        inout_addr = (volatile int**)((char*)&global_array[32].d + index);
        
        /* Inline assembly with both input and output addressing */
        asm volatile (
            "movl (%1), %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=r" (temp)
            : "r" (inout_addr)
            : "ebx", "r12", "r13", "r14"
        );
        
        /* Pass address of address to function */
        modify_pptr((volatile int***)&local_pptr);
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS pattern with nested addressing */
    {
        volatile struct MixedData*** super_ptr;
        volatile int offset1 = 16;
        volatile int offset2 = 8;
        
        /* Very complex addressing chain */
        super_ptr = (volatile struct MixedData***)
                   ((char*)&global_nested[0].ptr_to_ptr + offset1 * 2);
        
        /* Multiple memory operands in assembly */
        asm volatile (
            "movq (%1), %%r15\n\t"
            "movq (%2), %%r15\n\t"
            "addl $1, (%%r15)\n\t"
            : /* no outputs */
            : "r" (super_ptr), "m" (*p2), "r" (&offset2)
            : "r15", "r12", "r13", "r14", "memory"
        );
    }
    
    /* Loop with varying addressing modes to create multiple reload contexts */
    for (volatile int i = 0; i < 3; i++) {
        volatile int* loop_addr;
        
        /* Different addressing mode each iteration */
        switch (i) {
            case 0:
                loop_addr = (volatile int*)((char*)p1 + i * 16);
                break;
            case 1:
                loop_addr = &global_array[i * 8].a;
                break;
            case 2:
                loop_addr = (volatile int*)((char*)&global_nested[0] + 24);
                break;
            default:
                loop_addr = &local_var;
        }
        
        /* Assembly that clobbers address registers */
        asm volatile (
            "movl %1, %%ecx\n\t"
            "movl (%%ecx), %%edx\n\t"
            : /* no outputs */
            : "r" (loop_addr)
            : "ecx", "edx", "r12", "memory"
        );
        
        /* Function call with address-taken argument */
        compute_address((volatile void**)&loop_addr, p2, i * 4);
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS pattern */
    {
        register volatile char* byte_ptr asm ("r15");
        volatile int offset_array[4] = {1, 3, 7, 15};
        
        byte_ptr = (volatile char*)p1 + offset_array[2] * 4;
        
        /* Multiple constraints on the same operand */
        volatile int val;
        asm volatile (
            "movsbl (%1), %0\n\t"
            : "=r" (val)
            : "r" (byte_ptr), "m" (*byte_ptr)
            : "r15"
        );
        
        /* Use goto to create cross-block register pressure */
        if (val > 0) {
            goto final_label;
        }
    }
    
    /* Another block that won't be executed */
    {
        volatile int unused2 = 0;
        (void)unused2;
    }
    
final_label:
    /* Final complex addressing with all registers tied up */
    {
        volatile double* dbl_ptr;
        volatile int idx1 = 10, idx2 = 20;
        
        /* Extremely complex address calculation */
        dbl_ptr = (volatile double*)(
            (char*)&global_array[0] + 
            idx1 * sizeof(struct MixedData) + 
            idx2 * 2 + 
            offsetof(struct MixedData, b)
        );
        
        /* Assembly with many clobbered registers */
        asm volatile (
            "movsd (%1), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (*dbl_ptr)
            : "r" (dbl_ptr)
            : "xmm0", "r12", "r13", "r14", "r15", "memory"
        );
    }
}

/* Additional stressor functions */
static void stress_reload_chain(void) {
    volatile int chain[8];
    volatile int* ptrs[4];
    
    /* Create chain of address computations */
    for (int i = 0; i < 4; i++) {
        ptrs[i] = &chain[i * 2];
        
        /* Inline assembly that uses chain of addresses */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movl (%%eax), %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "=m" (*ptrs[i])
            : "r" (i == 0 ? &chain[0] : ptrs[i-1])
            : "eax", "ebx", "memory"
        );
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        global_array[i].a = i;
        global_array[i].b = i * 1.5;
    }
    
    for (int i = 0; i < 128; i++) {
        global_nested[i].ptr_to_ptr = (volatile struct MixedData**)&global_array[0];
        global_nested[i].big = i * 1000LL;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_reload_chain();
    
    /* More complex patterns in main */
    {
        register volatile int* alt_p asm ("rbx");
        volatile int stack_array[32];
        
        alt_p = &stack_array[16];
        
        /* Mixed constraints on memory operands */
        volatile int input = 100;
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull $2, %%eax\n\t"
            "movl %%eax, (%0)\n\t"
            : 
            : "r" (alt_p), "rm" (input)
            : "eax", "rbx", "memory"
        );
        
        /* Address of address computation */
        volatile int** addr_of_addr = &alt_p;
        modify_pptr((volatile int***)&addr_of_addr);
    }
    
    return 0;
}
