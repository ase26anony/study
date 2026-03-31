/* reload_stress.c - Stress GCC's reload pass with complex addressing modes */
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
volatile Container containers[4];
volatile int32_t global_buffer[256];
volatile double global_doubles[128];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pointer(int32_t*** ppp) {
    volatile int32_t dummy = ***ppp;
    (void)dummy;
}

void compute_address(void** addr, int offset) {
    volatile char* p = (volatile char*)*addr;
    p += offset * 37;
    *addr = (void*)p;
}

/* Main stress function with complex addressing patterns */
void stress_reload(void) {
    /* Bind specific registers for address computation */
    register volatile MixedType* p1 asm ("r12") = &containers[0].arr[0];
    register volatile int32_t* p2 asm ("r13") = &global_buffer[0];
    register volatile double* p3 asm ("r14") = &global_doubles[0];
    register void* temp_addr asm ("r15");
    
    volatile int idx = 0;
    volatile int offset = 0;
    
    /* Complex control flow with goto */
    goto start_block;
    
recompute_addresses:
    /* Force recomputation with different offsets */
    idx = (idx + 1) & 0x3F;
    offset = (offset * 13 + 7) & 0xFF;
    
    /* Complex pointer arithmetic with multiple registers */
    p1 = &containers[idx % 4].arr[(idx * 17) % 100];
    p2 = &global_buffer[(offset * 3) % 256];
    p3 = &global_doubles[(idx * 5) % 128];
    
    goto after_asm;
    
start_block:
    for (int i = 0; i < 3; i++) {
        /* Complex addressing with multiple constraints */
        volatile int32_t* addr1 = &p1->a;
        volatile double* addr2 = &p1->b;
        volatile char* addr3 = &p1->c[3];
        
        /* Inline assembly with multiple memory operands and clobbers */
        asm volatile (
            "movl %[val1], %%eax\n\t"
            "addl %%eax, %[mem1]\n\t"
            "movq %[val2], %%xmm0\n\t"
            "addsd %%xmm0, %[mem2]\n\t"
            "movb %[val3], %%cl\n\t"
            "addb %%cl, %[mem3]\n\t"
            : [mem1] "+m" (*addr1),
              [mem2] "+m" (*addr2),
              [mem3] "+m" (*addr3)
            : [val1] "r" (i),
              [val2] "r" ((long long)(i * 2)),
              [val3] "r" ((int)(i & 0xFF))
            : "eax", "ecx", "xmm0", "memory", "r12", "r13", "r14"
        );
        
        /* Nested function calls with address-taken arguments */
        volatile int32_t** pp = (volatile int32_t**)&p2;
        modify_pointer((int32_t***)&pp);
        
        /* More complex addressing */
        temp_addr = (void*)&p1->d;
        compute_address(&temp_addr, i);
        
        /* Non-contiguous memory accesses */
        volatile int64_t val = *(volatile int64_t*)temp_addr;
        containers[(i + 1) % 4].arr[(i * 19) % 100].d = val + i;
        
        if (i & 1) {
            goto recompute_addresses;
        }
        
after_asm:
        /* Different addressing mode for same data */
        volatile MixedType* p4 = p1 + (i * 2);
        volatile int32_t* p5 = p2 - (i * 4);
        
        /* Another inline asm with conflicting constraints */
        asm volatile (
            "leal (%[base], %[index], 4), %%ebx\n\t"
            "movl (%%ebx), %%edx\n\t"
            "imull %%edx, %[out]\n\t"
            : [out] "+r" (idx)
            : [base] "r" (p5),
              [index] "r" (i)
            : "ebx", "edx", "memory", "r12", "r13"
        );
        
        /* Address computation for output */
        volatile int32_t* out_addr = &global_buffer[(idx * 7) % 256];
        
        /* Output address reload scenario */
        asm volatile (
            "movl %[in], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (*out_addr)
            : [in] "r" (idx)
            : "eax", "memory"
        );
        
        /* Jump to create control flow complexity */
        if (i == 1) {
            goto extra_computation;
        }
        
        continue;
        
extra_computation:
        /* Additional address computation block */
        volatile double* dp = p3 + (idx % 16);
        volatile int32_t* ip = (volatile int32_t*)dp;
        
        /* Mixed-type access forcing alignment handling */
        asm volatile (
            "movq %[src], %%xmm1\n\t"
            "cvttsd2si %%xmm1, %%eax\n\t"
            "movl %%eax, %[dst]\n\t"
            : [dst] "=m" (*ip)
            : [src] "m" (*dp)
            : "eax", "xmm1", "memory"
        );
    }
    
    /* Final complex addressing chain */
    volatile void* chain[4];
    chain[0] = (void*)p1;
    chain[1] = (void*)p2;
    chain[2] = (void*)p3;
    chain[3] = temp_addr;
    
    for (int j = 0; j < 4; j++) {
        volatile char* cp = (volatile char*)chain[j];
        cp += j * 16;
        
        /* Operand address reload scenario */
        asm volatile (
            "movb $0x42, (%[addr])\n\t"
            : 
            : [addr] "r" (cp)
            : "memory"
        );
    }
}

/* Secondary stress function with different patterns */
void more_stress(void) {
    register volatile int32_t* r1 asm ("r10") = &global_buffer[64];
    register volatile double* r2 asm ("r11") = &global_doubles[32];
    
    volatile int32_t* local_ptrs[8];
    
    for (int i = 0; i < 8; i++) {
        /* Complex index calculation */
        int idx = (i * 29 + 11) % 256;
        
        /* Address of address computation */
        local_ptrs[i] = r1 + idx;
        
        /* Inline asm with input address reload */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl %%eax, %%ecx\n\t"
            : 
            : [addr] "r" (local_ptrs[i]),
              "c" (i)
            : "eax", "memory", "r10"
        );
        
        /* Output address with different base */
        volatile double* out_dbl = r2 + (i % 16);
        
        /* Output address reload */
        asm volatile (
            "cvtsi2sd %%ecx, %%xmm2\n\t"
            "movsd %%xmm2, %[out]\n\t"
            : [out] "=m" (*out_dbl)
            : 
            : "xmm2", "memory", "r11"
        );
        
        /* Jump to create basic block boundaries */
        if (i & 2) {
            goto skip;
        }
        
        /* More address computation */
        volatile int32_t** pp = &local_ptrs[i];
        modify_pointer((int32_t***)&pp);
        
        skip:
        /* Empty target for goto */
        ;
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = i;
    }
    
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 0.5;
    }
    
    /* Call stress functions multiple times */
    for (int iter = 0; iter < 2; iter++) {
        stress_reload();
        more_stress();
    }
    
    return 0;
}
