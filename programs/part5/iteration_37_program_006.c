/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

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
    volatile int data[4][4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2;
    volatile int idx;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    volatile struct bitfields bf = {0};
    
    /* 2. Union for STRICT_LOW_PART */
    union split_int split = {0};
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    register short reg_short asm("ax");
    
    /* 4. Vector for SUBREG via vector operations */
    v4si vec = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* 5. Complex memory access structures */
    struct level3 l3_data;
    struct level2 l2_data = {&l3_data, 1};
    struct level1 l1_data = {&l2_data, 2};
    
    /* Initialize array data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            l3_data.data[i][j] = i * 4 + j;
        }
    }
    
    /* Loop with multiple operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional to create branching */
        if (i & 1) {
            /* ZERO_EXTRACT: Bitfield assignments at non-byte boundaries */
            bf.f2 = (i * 3) & 0x1F;      /* 5-bit field */
            bf.f3 = (i * 5) & 0xFF;      /* 8-bit field */
            bf.f4 = (i * 7) & 0xFFFF;    /* 16-bit field */
            
            /* Force use of bitfields in computation */
            global_counter += bf.f2 + bf.f3;
        } else {
            /* STRICT_LOW_PART: Partial register update */
            split.parts.low = (i * 11) & 0xFFFF;
            
            /* Another STRICT_LOW_PART pattern via pointer */
            volatile uint32_t *int_ptr = &split.full;
            ((volatile uint16_t *)int_ptr)[1] = (i * 13) & 0xFFFF; /* high part */
        }
        
        /* SUBREG: Register variable with smaller type operation */
        reg_short = (reg_var >> 8) & 0xFFFF;
        reg_var = (reg_var << 4) | (i & 0xF);
        
        /* SUBREG via vector extraction */
        int elem = vec[2];  /* This may generate SUBREG */
        vec[1] = elem + i;
        
        /* Complex MEM addressing: multi-level pointer dereference */
        int val = l1_data.l2->l3->data[l1_data.idx][l2_data.offset];
        l1_data.l2->l3->data[(i/10) % 4][(i/5) % 4] = val + i;
        
        /* More complex addressing with computation */
        volatile int ***triple_ptr = (volatile int ***)&l1_data.l2->l3->data;
        (*triple_ptr)[(i/3) % 4][(i/7) % 4] += global_counter;
        
        /* Inline assembly to influence RTL generation */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movw %%ax, %0\n\t"
            : "=m" (split.parts.low)
            : "r" (reg_var)
            : "eax"
        );
        
        /* Another inline assembly with memory constraint */
        int temp;
        asm volatile (
            "movl (%1), %0\n\t"
            "addl $1, %0\n\t"
            "movl %0, (%1)"
            : "=r" (temp)
            : "r" (&global_counter)
            : "memory"
        );
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                bf.f1 = (bf.f1 + 1) & 0x7;
                break;
            case 1:
                split.parts.high = (split.parts.high + 2) & 0xFFFF;
                break;
            case 2:
                reg_var = (reg_var * 3) & 0xFFFFFFFF;
                break;
            case 3:
                /* Complex memory chain */
                l1_data.l2->offset = (l1_data.l2->offset + 1) % 4;
                break;
        }
        
        /* Vector operation that may generate SUBREG */
        vec = vec + vec2;
        vec2[0] = vec[3];  /* Potential SUBREG extraction */
    }
    
    /* Combine results to prevent dead code elimination */
    global_result = bf.f1 + bf.f2 + bf.f3 + bf.f4 +
                   split.full + reg_var + vec[0] + vec[1] +
                   l3_data.data[0][0];
    
    return global_result & 0xFF;
}
