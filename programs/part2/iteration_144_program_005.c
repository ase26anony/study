/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures with mixed types to prevent optimization */
typedef struct {
    volatile int32_t a;
    volatile double b;
    volatile char c[7];
    volatile int64_t d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile int32_t* volatile ptrs[50];
} Container;

/* Global volatile arrays to force memory accesses */
volatile Container containers[10];
volatile int64_t global_buffer[256];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pointer(int32_t*** pp) {
    /* Force pointer indirection */
    if (pp && *pp) {
        ***pp += 1;
    }
}

void compute_address(void** addr1, void** addr2) {
    /* Complex address computation helper */
    volatile static int counter = 0;
    if (addr1 && addr2) {
        *addr1 = (void*)((uintptr_t)*addr2 + (counter++ * 37));
    }
}

/* Main stress function */
void stress_reloads(void) {
    /* Bind specific pointers to explicit registers */
    register MixedType* p1 asm ("r12");
    register int64_t* p2 asm ("r13");
    register void* p3 asm ("r14");
    register int32_t* p4 asm ("r15");
    
    /* Initialize register-bound pointers */
    p1 = (MixedType*)&containers[0].arr[0];
    p2 = &global_buffer[0];
    
    /* Complex addressing mode 1: Array indexing with multiple registers */
    int offset1 = 3;
    int offset2 = 7;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    for (int i = 0; i < 3; i++) {
        /* Compute address using register variable with complex offset */
        MixedType* addr1 = p1 + offset1 * i + offset2;
        
        /* Inline assembly with memory operand and clobbered registers */
        asm volatile (
            "movq %[mem], %%rax\n\t"
            "addl $1, (%%rax)\n\t"
            : 
            : [mem] "m" (addr1->a)
            : "rax", "r12", "memory"
        );
        
        /* Jump to create complex control flow */
        if (i == 1) goto recompute_addr;
        
        continue_addr:
        /* Nested pointer indirection */
        int32_t** pp = (int32_t**)&containers[i].ptrs[0];
        modify_pointer(&pp);
    }
    
    goto done;
    
recompute_addr:
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    {
        /* Reuse same register for different base */
        p2 = &global_buffer[128];
        
        /* Complex offset computation */
        int64_t complex_offset = (int64_t)((uintptr_t)p2 * 3) / 17;
        
        /* Inline assembly with multiple conflicting constraints */
        int64_t result;
        asm volatile (
            "movq %[base], %%rbx\n\t"
            "leaq (%[base], %[offset], 4), %%rcx\n\t"
            "movq (%%rcx), %[res]\n\t"
            : [res] "=r" (result)
            : [base] "r" (p2), [offset] "r" (complex_offset)
            : "rbx", "rcx", "r13", "memory"
        );
        
        /* Use result in address computation */
        p3 = (void*)((uintptr_t)p2 + result);
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        compute_address(&p3, (void**)&p2);
    }
    
    goto continue_addr;
    
done:
    /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
    {
        /* Complex pointer chain */
        volatile char* char_ptr = (volatile char*)&containers[5];
        p4 = (int32_t*)(char_ptr + 37);
        
        /* Multiple memory accesses with different types */
        for (int j = 0; j < 2; j++) {
            /* Scattered, non-sequential access pattern */
            int32_t val1 = containers[j].arr[j*13 + 7].a;
            double val2 = containers[j+2].arr[j*17 + 11].b;
            
            /* Inline assembly with explicit register destruction */
            asm volatile (
                "movl %[val1], %%eax\n\t"
                "cvtsi2sd %%eax, %%xmm0\n\t"
                "addsd %[val2], %%xmm0\n\t"
                "movsd %%xmm0, %[storage]\n\t"
                : [storage] "=m" (global_buffer[j])
                : [val1] "rm" (val1), [val2] "xm" (val2)
                : "rax", "xmm0", "r12", "r13", "r14", "r15", "memory"
            );
            
            /* Address-taken argument with complex expression */
            int32_t* volatile temp_ptr = p4 + j * 3;
            modify_pointer((int32_t***)&temp_ptr);
        }
    }
    
    /* Final complex addressing with goto */
    p1 = (MixedType*)&containers[9].arr[99];
    goto final_computation;
    
final_computation:
    {
        /* Force all reload types in one block */
        MixedType* volatile addr_array[4];
        
        for (int k = 0; k < 4; k++) {
            /* Each iteration uses different addressing mode */
            switch (k) {
                case 0:
                    /* Direct register addressing */
                    addr_array[k] = p1 + k;
                    break;
                case 1:
                    /* Indirect with offset */
                    addr_array[k] = (MixedType*)((char*)p1 + k * sizeof(MixedType) * 2);
                    break;
                case 2:
                    /* Complex computation */
                    addr_array[k] = &containers[k/2].arr[k*5 % 50];
                    break;
                case 3:
                    /* Pointer chain */
                    addr_array[k] = (MixedType*)((int64_t*)addr_array[1] + 3);
                    break;
            }
            
            /* Inline assembly that clobbers address registers */
            if (k & 1) {
                asm volatile (
                    "movq %[addr], %%rsi\n\t"
                    "lock addl $1, (%%rsi)\n\t"
                    :
                    : [addr] "m" (addr_array[k]->a)
                    : "rsi", "memory", "r12", "r13"
                );
            }
        }
        
        /* One more pointer-to-pointer call */
        int32_t* final_ptr = (int32_t*)addr_array[3];
        int32_t** final_pp = &final_ptr;
        modify_pointer(&final_pp);
    }
}

/* Additional stressor functions */
void stress_address_chains(void) {
    volatile static int chain_buffer[100];
    register int* r1 asm ("r10");
    register int* r2 asm ("r11");
    
    r1 = &chain_buffer[0];
    r2 = &chain_buffer[50];
    
    /* Create address computation chain */
    for (int i = 0; i < 5; i++) {
        int* temp = r1 + i * 7;
        
        /* Force reloads by using same register in different ways */
        asm volatile (
            "movl (%[src]), %%eax\n\t"
            "movl %%eax, (%[dst])\n\t"
            : 
            : [src] "r" (temp), [dst] "r" (r2 + i * 3)
            : "rax", "r10", "r11", "memory"
        );
        
        /* Jump to disrupt register allocation */
        if (i == 2) goto chain_break;
        continue_chain:
        /* Empty */;
    }
    return;
    
chain_break:
    /* Different addressing mode */
    r1 = (int*)((char*)r2 + 16);
    goto continue_chain;
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            containers[i].arr[j].a = i * 100 + j;
            containers[i].arr[j].b = (double)(i + j) / 2.0;
        }
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    stress_address_chains();
    
    /* Additional inline stress in main */
    {
        register volatile double* dptr asm ("rbx");
        dptr = (volatile double*)&containers[3].arr[33].b;
        
        /* Complex floating point addressing */
        for (int i = 0; i < 2; i++) {
            double* aligned_ptr = (double*)(((uintptr_t)dptr + 31) & ~31);
            
            asm volatile (
                "movsd (%[ptr]), %%xmm1\n\t"
                "addsd %%xmm1, %%xmm1\n\t"
                "movsd %%xmm1, %[out]\n\t"
                : [out] "=m" (*aligned_ptr)
                : [ptr] "r" (dptr + i)
                : "xmm1", "rbx", "memory"
            );
        }
    }
    
    return 0;
}
