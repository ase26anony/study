/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 7;
    volatile unsigned int f4 : 9;
    volatile unsigned int f5 : 8;
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
    volatile struct level3 *l3;
    volatile int extra;
};

struct level1 {
    volatile struct level2 *l2_array[8];
    volatile int count;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for SUBREG influence */
static inline void subreg_operations(volatile int *output) {
    /* Register variable for SUBREG */
    register int reg_var asm("eax") = *output;
    register short reg_short asm("si") = (short)reg_var;
    
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movw %w1, %0\n\t"
        : "=r" (reg_short)
        : "r" (reg_var)
        : "eax", "si"
    );
    
    /* Vector operations that may generate SUBREG */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract element (may use SUBREG) */
    int elem = vec3[2];
    
    /* Combine results */
    *output = reg_var + reg_short + elem;
}

int main(void) {
    struct bitfields bf = {0};
    union split_int split = {0};
    
    /* Complex memory structure setup */
    struct level3 l3_inst = {{10, 20, 30, 40}};
    struct level2 l2_inst = {&l3_inst, 100};
    struct level1 l1_inst;
    
    /* Array of pointers for complex addressing */
    struct level2 *l2_array[8];
    for (int i = 0; i < 8; i++) {
        l2_array[i] = &l2_inst;
    }
    l1_inst.l2_array[0] = &l2_inst;
    l1_inst.count = 8;
    
    /* Multi-dimensional array for complex MEM addressing */
    volatile int md_array[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                md_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < 100; i++) {
        /* Conditional to create branching */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignments */
            bf.f1 = (i & 0x7);
            bf.f3 = ((i * 2) & 0x7F);
            bf.f5 = ((i + 5) & 0xFF);
            
            /* Additional bitfield combination */
            unsigned int temp = bf.f2 + bf.f4;
            bf.f2 = temp & 0x1F;
        } else if (i % 3 == 1) {
            /* STRICT_LOW_PART: Partial register updates */
            split.parts.low = (i * 3) & 0xFFFF;
            
            /* Another STRICT_LOW_PART pattern */
            volatile uint32_t *int_ptr = &split.full;
            volatile uint16_t *short_ptr = (volatile uint16_t *)int_ptr;
            short_ptr[1] = (i * 7) & 0xFFFF;  /* High part */
        } else {
            /* Mixed operations */
            split.parts.high = (i * 5) & 0xFFFF;
            bf.f4 = (i * 11) & 0x1FF;
        }
        
        /* SUBREG operations */
        volatile int subreg_temp = split.full + bf.f1;
        subreg_operations(&subreg_temp);
        
        /* Complex MEM addressing modes */
        int idx1 = (i * 7) % 8;
        int idx2 = (i * 11) % 8;
        int idx3 = (i * 13) % 8;
        
        /* Multi-level pointer dereferencing */
        volatile int mem_val1 = l1_inst.l2_array[0]->l3->data[idx1];
        
        /* Multi-dimensional array with complex indexing */
        volatile int mem_val2 = md_array[idx1][idx2][idx3];
        
        /* Chain of pointer accesses */
        volatile int mem_val3 = l2_array[idx2]->l3->data[idx3];
        
        /* Inline assembly with memory constraints */
        asm volatile (
            "addl %1, %0\n\t"
            "movl %0, %2\n\t"
            : "+r" (subreg_temp)
            : "m" (mem_val1), "m" (md_array[idx1][idx2][0])
            : "memory"
        );
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                md_array[idx1][idx2][idx3] = subreg_temp + mem_val1;
                break;
            case 1:
                md_array[idx2][idx3][idx1] = subreg_temp + mem_val2;
                break;
            case 2:
                md_array[idx3][idx1][idx2] = subreg_temp + mem_val3;
                break;
            default:
                /* Complex addressing with pointer arithmetic */
                volatile int *chain_ptr = &md_array[0][0][0];
                chain_ptr += (idx1 * 64 + idx2 * 8 + idx3);
                *chain_ptr = subreg_temp;
                break;
        }
        
        /* Update global state */
        global_counter += bf.f1 + split.parts.low + subreg_temp;
        
        /* Prevent loop elimination */
        if (global_counter > 1000000) {
            global_counter = 0;
        }
    }
    
    /* Final computation using all variables */
    global_result = bf.f1 + bf.f2 + bf.f3 + bf.f4 + bf.f5 +
                   split.full + global_counter +
                   md_array[0][0][0] + md_array[7][7][7];
    
    /* Additional inline assembly to force specific register usage */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movw %%ax, %%si\n\t"
        "addl %%esi, %0\n\t"
        : "+r" (global_result)
        :
        : "eax", "si", "esi"
    );
    
    return global_result % 256;
}
