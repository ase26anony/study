/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Struct with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex nested structure for memory addressing */
struct level3 {
    volatile int data[4][4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2_array[8];
    volatile int idx;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct g_bf = {0};
volatile union type_pun g_pun = {0};
volatile struct level1 g_l1 = {0};
volatile v4si g_vector = {0};
volatile int g_counter = 0;
volatile int g_result = 0;

/* Function to force complex memory addressing */
static inline int complex_mem_access(volatile struct level1 *l1, int iter) {
    /* Multi-level pointer dereferencing with non-constant indices */
    return l1->l2_array[iter & 7]->l3->data[(iter >> 3) & 3][(iter >> 5) & 3];
}

int main(void) {
    /* Register variables for SUBREG operations */
    register uint32_t reg_var asm("eax") = 0x12345678;
    register uint16_t reg_short asm("bx") = 0;
    
    /* Initialize memory structures */
    volatile struct level3 *l3 = malloc(sizeof(struct level3));
    volatile struct level2 *l2 = malloc(sizeof(struct level2));
    
    for (int i = 0; i < 8; i++) {
        g_l1.l2_array[i] = malloc(sizeof(struct level2));
        g_l1.l2_array[i]->l3 = l3;
        g_l1.l2_array[i]->offset = i * 16;
    }
    
    l2->l3 = l3;
    
    /* Initialize array data */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            l3->data[i][j] = i * 4 + j;
        }
    }
    
    /* Vector initialization */
    g_vector = (v4si){1, 2, 3, 4};
    
    /* Main loop with combined operations */
    for (g_counter = 0; g_counter < 1000; g_counter++) {
        int iter = g_counter;
        
        /* 1. BIT-FIELD OPERATIONS (ZERO_EXTRACT) */
        /* Non-byte-aligned assignments */
        g_bf.a = (iter >> 0) & 0x7;      /* 3 bits */
        g_bf.b = (iter >> 3) & 0x1F;     /* 5 bits */
        g_bf.c = (iter >> 8) & 0xFF;     /* 8 bits */
        g_bf.d = (iter >> 16) & 0xFFFF;  /* 16 bits */
        
        /* 2. STRICT_LOW_PART operations */
        /* Partial register updates through union */
        g_pun.parts.low = iter & 0xFFFF;
        g_pun.bytes[1] = (iter >> 8) & 0xFF;
        
        /* Pointer-based partial update */
        volatile uint32_t *int_ptr = (volatile uint32_t *)&g_pun.full;
        ((volatile uint16_t *)int_ptr)[0] = (iter * 3) & 0xFFFF;
        
        /* 3. SUBREG operations with register variables */
        /* Truncation through smaller type operation */
        reg_short = reg_var & 0xFFFF;
        
        /* Vector element extraction (triggers SUBREG) */
        int vec_elem = ((volatile int *)&g_vector)[iter & 3];
        
        /* Inline assembly suggesting subregister use */
        asm volatile (
            "movw %w[input], %[output]"
            : [output] "=r" (reg_short)
            : [input] "r" (reg_var)
            : "cc"
        );
        
        /* 4. COMPLEX MEMORY ADDRESSING */
        /* Multi-level access with computation */
        int mem_val = complex_mem_access(&g_l1, iter);
        
        /* Array indexing with pointer chains */
        volatile int ***triple_ptr = (volatile int ***)&g_l1.l2_array[0]->l3->data;
        int val = (*triple_ptr)[(iter >> 2) & 3][(iter >> 4) & 3];
        
        /* Structure pointer chain */
        int chain_val = g_l1.l2_array[iter & 7]->l3->data[g_l1.idx & 3][(iter >> 1) & 3];
        
        /* 5. COMBINE RESULTS WITH CONDITIONALS */
        if (iter & 0x1) {
            /* Use bit-field result */
            g_result += g_bf.c;
        } else if (iter & 0x2) {
            /* Use partial register result */
            g_result += g_pun.parts.low;
        } else if (iter & 0x4) {
            /* Use memory result */
            g_result += mem_val;
        } else {
            /* Use vector/register result */
            g_result += vec_elem + reg_short;
        }
        
        /* Switch for additional control flow complexity */
        switch (iter & 0x3) {
            case 0:
                g_bf.a = (g_result >> 2) & 0x7;
                break;
            case 1:
                g_pun.bytes[2] = g_result & 0xFF;
                break;
            case 2:
                reg_var = (reg_var << 3) | (g_result & 0x7);
                break;
            case 3:
                /* Complex memory store */
                g_l1.l2_array[iter & 7]->l3->data[0][0] = g_result;
                break;
        }
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r"(g_result) : "memory");
    }
    
    /* Additional inline assembly with constraints */
    uint32_t final_reg;
    asm volatile (
        "movl %%eax, %0\n\t"
        "movb %%ah, %%bl\n\t"
        "movw %%ax, %%cx"
        : "=r" (final_reg)
        : 
        : "eax", "ebx", "ecx", "cc"
    );
    
    /* Combine all results to prevent elimination */
    int final_result = 
        g_bf.a + g_bf.b + g_bf.c + g_bf.d +
        g_pun.full +
        reg_var +
        g_result +
        complex_mem_access(&g_l1, g_counter - 1);
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free((void *)g_l1.l2_array[i]);
    }
    free((void *)l2);
    free((void *)l3);
    
    return final_result & 0xFF;
}
