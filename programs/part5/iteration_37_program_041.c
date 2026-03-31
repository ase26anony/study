/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native */
/* Also try: gcc -O3 -frename-registers -fno-tree-vectorize -march=native */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 8;
    volatile unsigned int f4 : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* Complex memory structure */
struct level3 {
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *next;
    volatile int values[3];
};

struct level1 {
    volatile struct level2 *chain;
    volatile int count;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for SUBREG influence */
static inline void subreg_operation(volatile int *out, int in) {
    register int reg_var asm("eax") = in;
    register short reg_short asm("ax");
    
    /* Inline assembly suggesting subregister use */
    asm volatile (
        "movw %%ax, %1\n\t"
        "addw $1, %1\n\t"
        "movw %1, %%ax"
        : "=r" (reg_var), "+m" (reg_short)
        : "0" (reg_var)
        : "cc"
    );
    
    /* Force SUBREG through type conversion */
    *out = (int)reg_short;
}

int main(void) {
    struct bitfields bf = {0};
    union split_int split = {0};
    v4si vec = {1, 2, 3, 4};
    volatile int * volatile ptr_array[10];
    struct level1 complex_mem = {0};
    
    /* Initialize complex memory structure */
    struct level3 l3 = {{10, 20, 30, 40}};
    struct level2 l2 = {&l3, {100, 200, 300}};
    complex_mem.chain = &l2;
    
    /* Initialize pointer array */
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = (volatile int *)malloc(sizeof(int) * 5);
        for (int j = 0; j < 5; j++) {
            ptr_array[i][j] = i * 10 + j;
        }
    }
    
    /* Loop with multiple RTL-generating operations */
    for (int i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT through bitfield assignments */
        if (i & 1) {
            bf.f1 = (i & 0x7);           /* 3-bit field */
            bf.f3 = (i & 0xFF);          /* 8-bit field */
        } else {
            bf.f2 = (i & 0x1F);          /* 5-bit field */
            bf.f4 = (i & 0xFFFF);        /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART through union/pointer punning */
        if (i % 3 == 0) {
            split.parts.low = i & 0xFFFF;    /* Update low 16 bits */
        } else if (i % 3 == 1) {
            split.parts.high = (i >> 16) & 0xFFFF; /* Update high 16 bits */
        } else {
            /* Alternative: pointer cast for STRICT_LOW_PART */
            *((volatile short*)&split.full + (i & 1)) = i & 0xFFFF;
        }
        
        /* 3. SUBREG through register variables and vector operations */
        volatile int subreg_result;
        subreg_operation(&subreg_result, i);
        
        /* Vector operation that may generate SUBREG */
        int idx = i & 3;
        volatile int vec_element = vec[idx];  /* Element extraction */
        vec[idx] = vec_element + subreg_result;
        
        /* 4. Complex MEM addressing modes */
        if (complex_mem.chain && complex_mem.chain->next) {
            /* Multi-level pointer dereferencing */
            volatile int mem_val = complex_mem.chain->next->data[idx];
            
            /* Array indexing with non-constant index */
            volatile int *ptr = ptr_array[i % 10];
            ptr[(i + idx) % 5] = mem_val + i;
            
            /* Chain dereference in assignment */
            complex_mem.chain->values[idx % 3] = 
                complex_mem.chain->next->data[(idx + 1) % 4] + 
                ptr_array[(i + 1) % 10][idx % 5];
        }
        
        /* 5. Inline assembly with constraints to influence RTL */
        register int r1 asm("ebx") = i;
        register int r2 asm("ecx") = split.full;
        
        asm volatile (
            "addl %%ebx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            "movb %%cl, %1"
            : "=m" (global_counter), "=m" (*(volatile char*)&split.full)
            : "r" (r1), "r" (r2)
            : "eax", "ebx", "ecx", "memory", "cc"
        );
        
        /* Loop exit condition based on operations */
        if (global_counter > 1000) {
            break;
        }
    }
    
    /* Compute result to prevent dead code elimination */
    global_result = bf.f1 + bf.f2 + bf.f3 + bf.f4 + 
                   split.full + 
                   vec[0] + vec[1] + vec[2] + vec[3] + 
                   global_counter;
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free((void*)ptr_array[i]);
    }
    
    return global_result & 0xFF;
}
