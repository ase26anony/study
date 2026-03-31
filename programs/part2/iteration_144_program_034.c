/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures to prevent optimization */
struct MixedData {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
};

struct NestedPtrs {
    volatile int** pp;
    volatile struct MixedData* arr;
};

/* Global volatile arrays to force memory accesses */
static volatile struct MixedData big_array[1000];
static volatile struct NestedPtrs ptr_array[100];

/* Helper function taking pointer-to-pointer */
void use_double_ptr(volatile int*** ppp) {
    volatile int*** local = ppp;
    (void)local; /* Prevent unused warning */
}

/* Another helper for address computations */
volatile int* compute_offset(volatile struct MixedData* base, int idx) {
    return &base[idx].a;
}

/* Main stress function */
void stress_reloads(void) {
    /* Bind specific variables to registers */
    register volatile struct MixedData* p1 asm ("r12") = &big_array[0];
    register volatile struct NestedPtrs* p2 asm ("r13") = &ptr_array[0];
    register int idx asm ("r14") = 100;
    register int offset asm ("r15") = 50;
    
    volatile int result = 0;
    volatile int** temp_pp;
    
    /* Label for goto jumps */
    compute_addr_1:
    
    /* Complex address computation forcing RELOAD_FOR_INPUT_ADDRESS */
    volatile int* addr1 = &p1[(idx * 3 + offset) / 2].a;
    
    /* Inline assembly with memory operand and clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*addr1)
        : "m" (*addr1)
        : "eax", "r12", "r13", "r14", "r15", "memory"
    );
    
    /* Jump to create control flow complexity */
    goto compute_addr_2;
    
    /* Dead code block to separate control flow */
    {
        volatile int dummy = 0;
        dummy++;
    }
    
    compute_addr_2:
    
    /* Different address computation using same registers */
    volatile double* addr2 = &p2[(offset * 2 - idx) % 50].arr->b;
    
    /* Another asm with conflicting constraints */
    register double* r_addr asm ("r12") = (double*)addr2;
    asm volatile (
        "movsd (%1), %%xmm0\n\t"
        "addsd %%xmm0, %%xmm0\n\t"
        "movsd %%xmm0, (%0)\n\t"
        : 
        : "r" (r_addr), "r" (r_addr)
        : "xmm0", "r12", "r13", "memory"
    );
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    volatile int** out_addr = &p2[offset].pp;
    
    /* Nested function call with address-taken argument */
    temp_pp = (volatile int**)out_addr;
    use_double_ptr((volatile int***)&temp_pp);
    
    /* Complex expression for RELOAD_FOR_INPADDR_ADDRESS */
    volatile int* inpdaddr = compute_offset(
        &p1[idx + offset], 
        (offset * 3) % 20
    );
    
    /* Inline asm with multiple memory operands */
    asm volatile (
        "movl (%1), %%ebx\n\t"
        "imull $2, %%ebx\n\t"
        "movl %%ebx, (%0)\n\t"
        : 
        : "r" (inpdaddr), "r" (inpdaddr)
        : "ebx", "r12", "r13", "memory"
    );
    
    /* Loop to create multiple reload opportunities */
    for (int i = 0; i < 3; i++) {
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        volatile char* char_addr = &p1[i * 100].c[(idx + i) % 7];
        
        asm volatile (
            "movb (%1), %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, (%0)\n\t"
            : 
            : "r" (char_addr), "r" (char_addr)
            : "al", "r12", "r13", "memory"
        );
        
        /* Address computation for RELOAD_FOR_OPADDR_ADDR */
        volatile int** opaddr = (volatile int**)&p2[i].pp;
        
        /* Use goto within loop for complex control flow */
        if (i == 1) {
            goto special_case;
        }
        
        continue;
        
        special_case:
        /* Different address mode in special case */
        volatile int* special_addr = &p1[offset + i * 50].a;
        
        asm volatile (
            "lock addl $1, %0\n\t"
            : "+m" (*special_addr)
            : 
            : "r12", "r13", "memory"
        );
    }
    
    /* Force RELOAD_FOR_OUTADDR_ADDRESS */
    volatile int*** outdaddr = (volatile int***)&p2[offset].pp;
    
    /* Complex chain of address computations */
    volatile int* final_addr = (volatile int*)*outdaddr;
    if (final_addr) {
        *final_addr = idx + offset;
    }
    
    /* Final asm with many clobbers */
    asm volatile (
        "movl %0, %%ecx\n\t"
        "movl %1, %%edx\n\t"
        "addl %%edx, %%ecx\n\t"
        "movl %%ecx, %2\n\t"
        : 
        : "m" (idx), "m" (offset), "m" (result)
        : "ecx", "edx", "r12", "r13", "r14", "r15", "memory"
    );
}

/* Additional stress patterns */
void more_stress(void) {
    /* Bind to different registers */
    register volatile char* cp asm ("r10") = (volatile char*)big_array;
    register volatile int* ip asm ("r11") = &big_array[0].a;
    
    /* Unaligned accesses forcing address reloads */
    for (int i = 0; i < 10; i++) {
        volatile char* unaligned = cp + i * 13 + 3;
        
        asm volatile (
            "movb (%1), %%al\n\t"
            "xorb $0xFF, %%al\n\t"
            "movb %%al, (%0)\n\t"
            : 
            : "r" (unaligned), "r" (unaligned)
            : "al", "r10", "r11", "memory"
        );
        
        /* Switch between different address modes */
        if (i & 1) {
            volatile int* aligned = (volatile int*)(((uintptr_t)ip + 63) & ~63);
            *aligned = i;
        }
    }
    
    /* Pointer-to-pointer chains */
    volatile int* ptr1 = ip;
    volatile int** ptr2 = &ptr1;
    volatile int*** ptr3 = &ptr2;
    
    use_double_ptr((volatile int***)ptr3);
    
    /* Mixed addressing in single expression */
    volatile int val = *(volatile int*)((volatile char*)ptr3 + 4);
    (void)val;
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1000; i++) {
        big_array[i].a = i;
        big_array[i].b = i * 1.5;
        for (int j = 0; j < 7; j++) {
            big_array[i].c[j] = (char)(i + j);
        }
    }
    
    for (int i = 0; i < 100; i++) {
        ptr_array[i].pp = (volatile int**)&big_array[i].d;
        ptr_array[i].arr = &big_array[i * 10];
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    more_stress();
    
    /* Additional stress in main */
    register volatile struct MixedData* mp asm ("rbx") = &big_array[500];
    
    /* Complex address with multiple components */
    volatile int* complex_addr = &mp[
        ((int)mp->a * 2 + (int)mp->b) % 100
    ].a;
    
    asm volatile (
        "movl (%1), %%eax\n\t"
        "negl %%eax\n\t"
        "movl %%eax, (%0)\n\t"
        : 
        : "r" (complex_addr), "r" (complex_addr)
        : "eax", "rbx", "memory"
    );
    
    return 0;
}
