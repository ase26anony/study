/* reload_stress.c - Designed to trigger GCC reload pass uncovered lines */
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

/* Global volatile arrays to force complex addressing */
volatile Container containers[10];
volatile int global_buffer[1000];

/* Helper function taking pointer-to-pointer */
void modify_pointer(int*** pp) {
    **pp += 1;
}

/* Another helper with complex addressing */
void* compute_offset(void* base, int idx1, int idx2) {
    return (char*)base + idx1 * 37 + idx2 * 11;
}

/* Main stress function */
void stress_reload(void) {
    /* Bind specific registers for address computation */
    register MixedType* p1 asm ("r12") = (MixedType*)&containers[0];
    register int* p2 asm ("r13") = (int*)global_buffer;
    register void* p3 asm ("r14") = 0;
    register int* p4 asm ("r15") = 0;
    
    volatile int local_var = 42;
    volatile int local_array[50];
    int* volatile ptr_to_local = &local_array[0];
    
    /* Complex control flow with goto */
    int i = 0;
    
start_loop:
    if (i >= 5) goto end_loop;
    
    /* RELOAD_FOR_INPUT_ADDRESS pattern */
    {
        /* Complex address computation using register-bound pointer */
        int offset1 = i * 17 + 3;
        int offset2 = i * 23 + 7;
        
        /* Force address reload by using in inline asm */
        asm volatile (
            "movl %[val], (%[addr]) \n\t"
            : 
            : [addr] "r" (&p1->arr[offset1].a + offset2),
              [val] "r" (i)
            : "memory"
        );
    }
    
    /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
    {
        register int* temp asm ("r12") = p2 + i * 13;
        
        /* Clobber the register used for addressing */
        asm volatile (
            "movl %%eax, %[out] \n\t"
            : [out] "=m" (*temp)
            : 
            : "eax", "r12", "memory"
        );
        
        /* Jump to force register pressure */
        if (i & 1) goto odd_case;
        goto even_case;
        
    odd_case:
        /* Different address computation using same register */
        p3 = compute_offset(p1, i, i * 2);
        goto after_case;
        
    even_case:
        p3 = compute_offset(p2, i * 3, i);
        goto after_case;
        
    after_case:
        /* Use p3 in another asm */
        asm volatile (
            "addl $1, (%[ptr])"
            : 
            : [ptr] "r" (p3)
            : "memory", "r14"
        );
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS pattern */
    {
        int** pp = &ptr_to_local;
        
        /* Nested addressing */
        modify_pointer(&pp);
        
        /* Inline asm with multiple memory constraints */
        asm volatile (
            "movl (%[in]), %%eax \n\t"
            "movl %%eax, (%[out]) \n\t"
            : 
            : [in] "r" (&local_array[i * 7]),
              [out] "r" (&global_buffer[i * 19])
            : "eax", "memory"
        );
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS pattern */
    {
        /* Complex expression that needs temporary address register */
        volatile int* addr1 = &p1->arr[i].a + containers[i % 3].arr[0].a;
        volatile int* addr2 = &p2[i * 31] + local_var;
        
        /* Inline asm using both addresses */
        asm volatile (
            "movl (%[src]), %%ebx \n\t"
            "addl %%ebx, (%[dst]) \n\t"
            : 
            : [src] "r" (addr1),
              [dst] "r" (addr2)
            : "ebx", "memory", "r12", "r13"
        );
    }
    
    /* RELOAD_FOR_OPADDR_ADDR pattern */
    {
        /* Pointer to pointer with offset */
        int** nested_ptr = (int**)&containers[2].ptr_array[i];
        *nested_ptr = &local_array[i * 3];
        
        /* Force reload by clobbering address registers */
        asm volatile (
            "movl $0x12345678, %%r12d \n\t"
            "movl $0x87654321, %%r13d \n\t"
            : 
            : 
            : "r12", "r13", "r14", "r15"
        );
        
        /* Use the pointer after clobber */
        **nested_ptr += i;
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS pattern */
    {
        /* Mixed addressing modes */
        register double* dp asm ("r12") = (double*)&p1->arr[i].b;
        
        /* Volatile access with complex index */
        volatile int idx = containers[0].arr[i].a;
        dp += idx;
        
        /* Inline asm with explicit constraints */
        asm volatile (
            "movsd (%[src]), %%xmm0 \n\t"
            "addsd %%xmm0, %%xmm0 \n\t"
            "movsd %%xmm0, (%[dst]) \n\t"
            : 
            : [src] "r" (dp),
              [dst] "r" (&containers[1].arr[i].b)
            : "xmm0", "memory", "r12"
        );
    }
    
    /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
    {
        /* Address of output operand */
        int* output_addr = &global_buffer[i * 47];
        
        /* Complex addressing in input */
        int* input_addr = &p1->arr[i * 2].a + containers[3].arr[1].a;
        
        asm volatile (
            "movl (%[in]), %%ecx \n\t"
            "leal (%%ecx, %%ecx, 2), %%edx \n\t"
            "movl %%edx, %[out] \n\t"
            : [out] "=m" (*output_addr)
            : [in] "r" (input_addr)
            : "ecx", "edx", "memory"
        );
    }
    
    i++;
    goto start_loop;
    
end_loop:
    
    /* Final complex addressing pattern */
    {
        /* Multiple register-bound variables in one expression */
        p4 = (int*)p3 + (int)(p1->arr[0].a) + (int)(p2[0]);
        
        /* Force one more reload */
        asm volatile (
            "testl %[val], %[val]"
            : 
            : [val] "r" (*p4)
            : "cc", "r15"
        );
    }
}

/* Additional stressor functions */
void stress_reload2(void) {
    volatile int buffer[100];
    register int* r1 asm ("r10") = &buffer[0];
    register int* r2 asm ("r11") = &buffer[50];
    
    for (int j = 0; j < 10; j++) {
        /* Alternating register usage */
        if (j & 1) {
            asm volatile (
                "movl (%[a]), %%eax \n\t"
                "movl %%eax, (%[b]) \n\t"
                : 
                : [a] "r" (r1 + j * 3),
                  [b] "r" (r2 + j * 5)
                : "eax", "memory", "r10", "r11"
            );
        } else {
            /* Swap roles */
            asm volatile (
                "movl (%[b]), %%ebx \n\t"
                "movl %%ebx, (%[a]) \n\t"
                : 
                : [a] "r" (r2 + j * 2),
                  [b] "r" (r1 + j * 7)
                : "ebx", "memory", "r10", "r11"
            );
        }
        
        /* Pointer chasing */
        int** pp = (int**)&buffer[j * 4];
        *pp = &buffer[j * 6];
        modify_pointer(&pp);
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        containers[i].arr[0].a = i * 100;
        containers[i].arr[0].b = i * 1.5;
    }
    
    for (int i = 0; i < 1000; i++) {
        global_buffer[i] = i;
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    stress_reload2();
    
    /* More complex patterns in main */
    {
        register volatile Container* cp asm ("r12") = &containers[5];
        register int* ip asm ("r13") = &global_buffer[100];
        
        /* Unrolled loop with different addressing */
        for (int k = 0; k < 3; k++) {
            /* Compute address with multiple components */
            volatile int* addr = &cp->arr[k * 7].a + 
                                (ip[k * 13] & 0xFF) + 
                                (k * 23);
            
            /* Use in asm with clobber */
            asm volatile (
                "incl %[mem]"
                : [mem] "+m" (*addr)
                : 
                : "r12", "r13"
            );
            
            /* goto to break basic block */
            if (k == 1) goto special_handler;
            
            continue;
            
        special_handler:
            /* Different addressing mode */
            ip = &global_buffer[200];
            asm volatile (
                "movl $999, (%[ptr])"
                : 
                : [ptr] "r" (ip)
                : "memory", "r13"
            );
        }
    }
    
    return 0;
}
