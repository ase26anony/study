/* reload_stress.c - Designed to trigger GCC reload pass edge cases */
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
void modify_pptr(int** pp) {
    if (pp && *pp) {
        **pp += 1;
    }
}

void complex_address_helper(volatile int*** ppp, int offset) {
    if (ppp && *ppp && **ppp) {
        ***ppp += offset;
    }
}

/* Function with complex addressing patterns */
void stress_reloads(int iter) {
    /* Bind specific pointers to registers */
    register volatile MixedType* p1 asm ("r12");
    register volatile int* p2 asm ("r13");
    register volatile char* p3 asm ("r14");
    register int* p4 asm ("r15");
    
    /* Initialize register-bound pointers */
    p1 = &containers[iter & 3].arr[0];
    p2 = &global_buffer[0];
    p3 = (volatile char*)&containers[0];
    p4 = (int*)&global_buffer[128];
    
    /* Complex addressing computation block A */
    {
        volatile int* addr1 = &p1->a + (iter * 3);
        volatile double* addr2 = &p1->b + (iter & 1);
        volatile char* addr3 = p3 + (iter * 17);
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+m" (*addr1)
            : "m" (*addr2), "m" (*addr3)
            : "eax", "r12", "r13", "r14", "memory"
        );
        
        /* Jump to force control flow complexity */
        goto compute_block_b;
    }
    
    /* This label creates a basic block boundary */
    compute_block_b:
    
    /* Recompute addresses using same registers for different purposes */
    p1 = &containers[(iter + 1) & 3].arr[10];
    p2 = &global_buffer[64];
    
    /* Complex offset computation */
    int offset = (iter * 7 + 3) & 31;
    volatile int* complex_addr = &p1->a + (offset * 2);
    volatile int* another_addr = p2 + (offset ^ 0xF);
    
    /* Inline assembly with conflicting constraints */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "subl %%ebx, %0\n\t"
        "leal (%0, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %2\n\t"
        : "+r" (*complex_addr), "+m" (*another_addr)
        : 
        : "ebx", "ecx", "r12", "r13", "memory"
    );
    
    /* Nested pointer operations */
    {
        int local_var = iter * 3;
        int* local_ptr = &local_var;
        int** pptr = &local_ptr;
        
        /* Pass address of address to function */
        modify_pptr(pptr);
        
        /* Use result in another computation */
        volatile int* volatile* vpptr = (volatile int* volatile*)&p2;
        
        /* Triple pointer indirection */
        volatile int*** triple_ptr = (volatile int***)&vpptr;
        complex_address_helper((int***)triple_ptr, offset);
    }
    
    /* Block with output address reloads */
    {
        register volatile int64_t* p5 asm ("r12"); /* Reuse r12 */
        p5 = &p1->d;
        
        int64_t temp;
        /* Inline assembly with output memory operand */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq $0x1234, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=m" (*p5)
            : "m" (*p5)
            : "rax", "r12", "memory"
        );
        
        /* Jump back to create loop in control flow */
        if (iter & 1) {
            goto final_block;
        }
    }
    
    /* Another block with different addressing mode */
    {
        volatile MixedType* base = &containers[2].arr[20];
        volatile int* indices = &global_buffer[0];
        
        /* Non-contiguous, scattered accesses */
        for (int i = 0; i < 5; i++) {
            volatile int* elem = &base[indices[i] & 15].a;
            *elem += i;
            
            /* Inline asm that clobbers address registers */
            asm volatile (
                "movl %0, %%edx\n\t"
                "rorl $8, %%edx\n\t"
                "movl %%edx, %0\n\t"
                : "+m" (*elem)
                : 
                : "edx", "r12", "r13", "r14", "memory"
            );
        }
    }
    
    final_block:
    
    /* Final complex address computation mixing everything */
    {
        volatile char* base1 = (volatile char*)&containers[0];
        volatile int* base2 = &global_buffer[0];
        int idx1 = (iter * 13) % 100;
        int idx2 = (iter * 17) % 256;
        
        /* Multiple address computations in same expression */
        volatile int* final_addr = (volatile int*)(base1 + idx1 * sizeof(MixedType) + 4);
        volatile int* final_addr2 = base2 + idx2;
        
        /* Inline assembly with multiple memory operands and address clobbers */
        asm volatile (
            "movl %1, %%esi\n\t"
            "addl %%esi, %0\n\t"
            "movl %0, %%edi\n\t"
            "movl %%edi, %2\n\t"
            : "+m" (*final_addr), "+m" (*final_addr2)
            : 
            : "esi", "edi", "r12", "r13", "r14", "r15", "memory"
        );
    }
}

/* Main function that creates multiple reload scenarios */
int main() {
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_buffer[i] = i * 3;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 100; j++) {
            containers[i].arr[j].a = i + j;
            containers[i].arr[j].b = (double)(i * j) / 3.0;
            containers[i].arr[j].d = (int64_t)(i * 1000 + j);
        }
    }
    
    /* Create multiple reload scenarios with different parameters */
    for (int iter = 0; iter < 8; iter++) {
        /* Vary the iteration to create different addressing patterns */
        stress_reloads(iter);
        
        /* Additional inline complexity in main */
        {
            register volatile Container* cp asm ("r12");
            cp = &containers[iter & 3];
            
            volatile int* volatile* ptrptr = (volatile int* volatile*)&cp->ptr_array[0];
            
            /* Complex pointer chain */
            for (int i = 0; i < 5; i++) {
                ptrptr[i] = &global_buffer[i * 10 + iter];
                
                /* Inline asm that uses the pointer chain */
                asm volatile (
                    "movq %0, %%r8\n\t"
                    "movl (%%r8), %%r9d\n\t"
                    "addl $1, %%r9d\n\t"
                    "movl %%r9d, (%%r8)\n\t"
                    : 
                    : "r" (ptrptr[i])
                    : "r8", "r9", "r12", "memory"
                );
            }
        }
        
        /* Jump to create additional control flow complexity */
        if (iter & 1) {
            goto skip_block;
        }
        
        /* Block with output address computation */
        {
            volatile int output_buffer[16];
            register volatile int* op asm ("r13");
            op = &output_buffer[0];
            
            for (int i = 0; i < 8; i++) {
                /* Address computation that may need output address reload */
                volatile int* out_addr = op + (i * 2) + (iter & 3);
                
                asm volatile (
                    "movl %1, %%r10d\n\t"
                    "movl %%r10d, %0\n\t"
                    : "=m" (*out_addr)
                    : "r" (i * 100 + iter)
                    : "r10", "r13", "memory"
                );
            }
        }
        
        skip_block:
        
        /* More address computations */
        {
            volatile MixedType* mp = &containers[iter & 1].arr[50];
            volatile int* ip = &global_buffer[iter * 8];
            
            /* Mixed type address computation */
            volatile char* char_ptr = (volatile char*)mp + offsetof(MixedType, c);
            char_ptr += iter * 3;
            
            asm volatile (
                "movb $0xAA, %0\n\t"
                : "=m" (*char_ptr)
                : 
                : "r12", "r13", "memory"
            );
        }
    }
    
    return 0;
}
