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
} ComplexStruct;

/* Global volatile arrays to force memory accesses */
volatile ComplexStruct global_data[10];
volatile int global_ints[1000];

/* Helper functions that take pointer-to-pointer arguments */
void modify_pptr(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

void compute_address(int* base, int offset, int** result) {
    *result = base + offset;
}

/* Function with complex addressing patterns */
void stress_reloads(void) {
    /* Bind specific pointers to registers */
    register MixedType* p1 asm ("r12");
    register int* p2 asm ("r13");
    register int* p3 asm ("r14");
    register volatile int* p4 asm ("r15");
    
    /* Initialize register-bound pointers */
    p1 = (MixedType*)&global_data[0].arr[0];
    p2 = (int*)&global_ints[0];
    p3 = (int*)&global_data[2].arr[10];
    p4 = &global_ints[500];
    
    int i, j, k;
    
    /* Complex control flow with goto */
    goto start_block;
    
    /* Label for jumping back */
    recompute_addresses:
    /* Force recomputation of addresses with different bases */
    p1 = (MixedType*)((char*)p1 + 64);
    p2 = p2 + 8;
    goto after_asm;
    
    start_block:
    
    for (i = 0; i < 3; i++) {
        /* Complex array indexing with multiple registers */
        volatile int* addr1 = &p1[i * 7 + 2].a;
        volatile double* addr2 = &p1[i * 3 + 1].b;
        
        /* Inline assembly with conflicting constraints */
        asm volatile (
            "movl %[val1], %%eax\n\t"
            "addl %%eax, %[mem1]\n\t"
            "movq %[val2], %%xmm0\n\t"
            "addsd %%xmm0, %[mem2]\n\t"
            : [mem1] "+m" (*addr1), [mem2] "+m" (*addr2)
            : [val1] "r" (i * 100), [val2] "r" ((long long)(i * 50))
            : "eax", "xmm0", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Nested function call with address-taken arguments */
        int* temp_ptr = (int*)addr1;
        modify_pptr(&temp_ptr);
        
        /* Jump to force register pressure */
        if (i == 1) {
            goto recompute_addresses;
        }
        
        after_asm:
        
        /* More complex addressing with pointer arithmetic */
        for (j = 0; j < 2; j++) {
            /* Use register variables in address calculations */
            int offset = (i * 13 + j * 7) & 0x3F;
            volatile char* char_ptr = &p1[offset].c[3];
            
            /* Another inline asm with memory operand */
            asm volatile (
                "movb $42, %[dest]\n\t"
                : [dest] "=m" (*char_ptr)
                :
                : "r12", "r13", "memory"
            );
            
            /* Chain of address computations */
            int** pptr;
            compute_address((int*)p2, offset * 2, &pptr);
            
            /* Use the computed pointer */
            if (pptr) {
                asm volatile (
                    "incl %0\n\t"
                    : "+m" (**pptr)
                    :
                    : "r12", "r13", "r14", "memory"
                );
            }
        }
        
        /* Switch between different base pointers */
        if (i & 1) {
            /* Use p3 for addressing */
            volatile int64_t* addr3 = &((MixedType*)p3)[i].d;
            *addr3 += i;
        } else {
            /* Use p4 for addressing with different offset calculation */
            int idx = (i * 17) % 100;
            p4[idx] = p2[i * 3];
        }
    }
    
    /* Final complex addressing pattern */
    MixedType* volatile_ptr_array[5];
    for (k = 0; k < 5; k++) {
        /* Non-sequential, scattered accesses */
        int scatter_idx = (k * 37) % 100;
        volatile_ptr_array[k] = (MixedType*)&global_data[k % 3].arr[scatter_idx];
        
        /* Inline asm with multiple memory operands */
        asm volatile (
            "movl %[src], %%ebx\n\t"
            "movl %%ebx, %[dst1]\n\t"
            "movl %%ebx, %[dst2]\n\t"
            : [dst1] "=m" (volatile_ptr_array[k]->a),
              [dst2] "=m" (global_ints[k * 20])
            : [src] "r" (k * 1000)
            : "ebx", "r12", "r13", "r14", "r15", "memory"
        );
    }
}

/* Additional stress function for output address reloads */
void stress_output_addresses(void) {
    register int* out1 asm ("r12");
    register double* out2 asm ("r13");
    
    volatile double results[50];
    volatile int outputs[50];
    
    out1 = (int*)&outputs[0];
    out2 = (double*)&results[0];
    
    for (int i = 0; i < 10; i++) {
        /* Complex output addressing */
        int* dest1 = out1 + i * 3;
        double* dest2 = out2 + i * 2;
        
        /* Inline asm with output memory constraints */
        asm volatile (
            "movl %[in1], %[out1]\n\t"
            "cvtsi2sd %[in2], %%xmm0\n\t"
            "movsd %%xmm0, %[out2]\n\t"
            : [out1] "=m" (*dest1), [out2] "=m" (*dest2)
            : [in1] "r" (i * 100), [in2] "r" (i * 50)
            : "xmm0", "r12", "r13", "memory"
        );
        
        /* Address computation that might need RELOAD_FOR_OUTADDR_ADDRESS */
        int** addr_of_output;
        compute_address(out1, i * 4, &addr_of_output);
        
        /* Use the address in another operation */
        if (addr_of_output) {
            asm volatile (
                "addl $1, %0\n\t"
                : "+m" (**addr_of_output)
                :
                : "r12", "memory"
            );
        }
    }
}

/* Main function that sets up and calls stress functions */
int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 1000; i++) {
        global_ints[i] = i;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i * 100 + j;
            global_data[i].arr[j].b = i * 100.0 + j;
            global_data[i].arr[j].d = i * 1000LL + j;
        }
    }
    
    /* Call stress functions multiple times */
    stress_reloads();
    stress_output_addresses();
    
    /* More complex pattern in main */
    {
        register volatile int* r1 asm ("r12");
        register volatile int* r2 asm ("r13");
        
        r1 = &global_ints[100];
        r2 = &global_ints[300];
        
        /* Jump-based control flow */
        goto compute;
        
        reload_point:
        /* Force address reload by clobbering registers in asm */
        asm volatile ("" : : : "r12", "r13", "r14", "r15");
        r1 = r2 + 10;
        goto done;
        
        compute:
        /* Complex address computation */
        for (int i = 0; i < 5; i++) {
            int idx = (i * 29) % 50;
            volatile int* addr = r1 + idx * 2;
            
            /* Inline asm that uses and clobbers address registers */
            asm volatile (
                "movl (%[addr]), %%eax\n\t"
                "imull $3, %%eax\n\t"
                "movl %%eax, (%[addr])\n\t"
                : 
                : [addr] "r" (addr)
                : "eax", "r12", "r13", "memory"
            );
        }
        goto reload_point;
        
        done:
        /* Final access */
        *r1 = 42;
    }
    
    return 0;
}
