/* reload_stress.c - Designed to stress GCC's reload pass */
#include <stdint.h>

/* Volatile structures to prevent optimization */
typedef struct {
    volatile int a;
    volatile double b;
    volatile char c[7];
    volatile int* d;
} MixedType;

typedef struct {
    volatile MixedType arr[100];
    volatile long long extra[50];
} BigStruct;

/* Global volatile arrays */
static volatile BigStruct global_data[10];
static volatile int* volatile ptr_array[100];

/* Helper functions that take complex pointer arguments */
static void use_pointer_to_pointer(volatile int*** ppp) {
    volatile int** pp = *ppp;
    if (pp) {
        volatile int* p = *pp;
        if (p) {
            *p += 1;
        }
    }
}

static void address_chain(volatile MixedType**** mpppp) {
    volatile MixedType*** mppp = *mpppp;
    if (mppp) {
        volatile MixedType** mpp = *mppp;
        if (mpp) {
            volatile MixedType* mp = *mpp;
            if (mp) {
                mp->a = mp->a + 1;
            }
        }
    }
}

/* Main stress function */
static void stress_reload(void) {
    /* Explicit register binding */
    register volatile MixedType* base_ptr asm("r12");
    register volatile int* index_ptr asm("r13");
    register volatile char* char_ptr asm("r14");
    
    /* Initialize with complex addresses */
    base_ptr = &global_data[2].arr[10];
    index_ptr = &global_data[3].arr[20].a;
    char_ptr = &global_data[1].arr[5].c[3];
    
    volatile int local_var = 42;
    volatile double local_double = 3.14159;
    volatile int* local_ptr = &local_var;
    
    /* Complex addressing mode 1 */
    {
        volatile int* addr1 = &base_ptr[local_var % 10].a;
        volatile double* addr2 = &base_ptr[(local_var + 7) % 10].b;
        
        /* Inline assembly with memory operands and clobbers */
        asm volatile (
            "addl $1, %[mem1]\n\t"
            "fldl %[mem2]\n\t"
            : [mem1] "+m" (*addr1)
            : [mem2] "m" (*addr2)
            : "memory", "st", "r12", "r13", "r14"
        );
        
        /* Jump to force register pressure */
        goto label1;
    }
    
    /* Different addressing mode after goto */
    {
        label1:
        /* Recompute addresses using same registers */
        volatile char* new_char_ptr = &char_ptr[local_var * 3 - 50];
        volatile int* new_int_ptr = &index_ptr[(int)local_double];
        
        /* Another inline assembly with conflicting constraints */
        asm volatile (
            "movl %[in], %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (*new_int_ptr)
            : [in] "m" (*new_char_ptr), "m" (*new_int_ptr)
            : "eax", "memory", "r12", "r13"
        );
        
        /* Nested function call with address-taken argument */
        {
            volatile int** pp = &local_ptr;
            use_pointer_to_pointer((volatile int***)&pp);
        }
    }
    
    /* More complex addressing with pointer arithmetic */
    {
        register volatile MixedType** mpp asm("r15");
        mpp = (volatile MixedType**)&ptr_array[30];
        
        /* Chain of address computations */
        volatile MixedType* mp1 = base_ptr + (index_ptr - (volatile int*)base_ptr) / 100;
        volatile MixedType* mp2 = (volatile MixedType*)((char*)base_ptr + 
                              ((char*)index_ptr - (char*)base_ptr) / 2);
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "movq %[src], %%r12\n\t"
            "movq %%r12, %[dst]\n\t"
            "leaq 16(%[base],%[index],4), %%r13\n\t"
            : [dst] "=m" (mp1->a)
            : [src] "m" (mp2->a), 
              [base] "r" (base_ptr), 
              [index] "r" ((long)(index_ptr - (volatile int*)base_ptr))
            : "r12", "r13", "memory"
        );
        
        /* Complex control flow with computed goto */
        {
            static void* labels[] = { &&label2, &&label3, &&label4 };
            int idx = local_var % 3;
            goto *labels[idx];
        }
    }
    
label2:
    {
        /* Different use of address registers */
        volatile long long* llptr = (volatile long long*)char_ptr;
        llptr += (index_ptr - (volatile int*)base_ptr) / 8;
        
        asm volatile (
            "movq %0, %%rax\n\t"
            "addq $1, %%rax\n\t"
            "movq %%rax, %1\n\t"
            : "=m" (*llptr)
            : "m" (*llptr)
            : "rax", "memory", "r14"
        );
        goto label5;
    }
    
label3:
    {
        /* RELOAD_FOR_OPERAND_ADDRESS pattern */
        volatile MixedType**** mpppp = (volatile MixedType****)&ptr_array[50];
        address_chain(mpppp);
        
        /* Mixed constraints on same operand */
        volatile int temp;
        asm volatile (
            "movl %[addr], %[temp]\n\t"
            "addl %%ecx, %[temp]\n\t"
            : [temp] "=r,m" (temp)
            : [addr] "m,r" (base_ptr->a)
            : "ecx", "memory"
        );
        goto label5;
    }
    
label4:
    {
        /* Output address reload pattern */
        volatile int out_val;
        asm volatile (
            "leal (%[base],%[index],2), %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (out_val)
            : [base] "r" (base_ptr->a), 
              [index] "r" (index_ptr[2])
            : "eax", "memory"
        );
        /* Fall through */
    }
    
label5:
    /* Final complex addressing with all registers used */
    {
        volatile MixedType* final_ptr = (volatile MixedType*)(
            (uintptr_t)base_ptr + 
            (uintptr_t)index_ptr * 2 - 
            (uintptr_t)char_ptr
        );
        
        /* Multiple memory operands with different constraints */
        asm volatile (
            "movl %1, %%ebx\n\t"
            "imull %2, %%ebx\n\t"
            "addl %%ebx, %0\n\t"
            "movl %3, %%ecx\n\t"
            "subl %%ecx, %0\n\t"
            : "+m" (final_ptr->a)
            : "m" (index_ptr[3]), 
              "m" (final_ptr->a), 
              "r" (local_var)
            : "ebx", "ecx", "memory", "r12", "r13", "r14"
        );
    }
}

/* Additional stress patterns */
static void more_stress(void) {
    volatile int buffer[100];
    
    for (int i = 0; i < 10; i++) {
        register volatile int* p1 asm("r10");
        register volatile int* p2 asm("r11");
        
        p1 = &buffer[i * 7 % 100];
        p2 = &buffer[(i * 13 + 5) % 100];
        
        /* Force input address reloads */
        asm volatile (
            "movl (%[addr1],%[idx1],4), %%eax\n\t"
            "addl %%eax, (%[addr2],%[idx2],4)\n\t"
            : 
            : [addr1] "r" (p1), 
              [idx1] "r" (i),
              [addr2] "r" (p2),
              [idx2] "r" (i + 1)
            : "eax", "memory", "r10", "r11"
        );
        
        /* Inpaddr/outaddr address patterns */
        {
            volatile int** pp = &p1;
            volatile int* volatile* vpp = &p2;
            
            asm volatile (
                "movq %[pp], %%r15\n\t"
                "movq (%%r15), %%rax\n\t"
                "movq %%rax, %[vpp]\n\t"
                : [vpp] "=m" (*vpp)
                : [pp] "m" (pp)
                : "rax", "r15", "memory"
            );
        }
    }
}

int main(void) {
    /* Initialize some data */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 100; j++) {
            global_data[i].arr[j].a = i * 100 + j;
            global_data[i].arr[j].b = (double)(i + j) / 2.0;
            ptr_array[i * 10 + j] = (volatile int*)&global_data[i].arr[j].a;
        }
    }
    
    /* Call stress functions multiple times */
    stress_reload();
    more_stress();
    
    /* Loop with varying patterns */
    for (int iter = 0; iter < 3; iter++) {
        stress_reload();
        
        /* Alternate pattern */
        register volatile MixedType* alt_ptr asm("r12");
        alt_ptr = &global_data[iter].arr[0];
        
        for (int i = 0; i < 5; i++) {
            volatile int* elem = &alt_ptr[i * 17 % 50].a;
            
            asm volatile (
                "lock xaddl %%eax, %[mem]\n\t"
                : [mem] "+m" (*elem)
                : "a" (iter + 1)
                : "memory", "r12"
            );
        }
    }
    
    return 0;
}
