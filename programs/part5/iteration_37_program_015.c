/* Target RTL expressions: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM */
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
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* Complex nested structure for MEM addressing */
struct level3 {
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *l3;
    volatile int offset;
};

struct level1 {
    volatile struct level2 *l2_array[8];
    volatile int index;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct g_bf = {0};
volatile union split_int g_split = {0};
volatile struct level1 g_l1;
volatile int g_result = 0;
volatile int *g_mem_base = NULL;

/* Function with inline assembly for SUBREG influence */
void manipulate_with_subreg(void) {
    /* Register variable for explicit register usage */
    register int reg_var asm("eax") = g_result;
    register short reg_short asm("ax") = (short)reg_var;
    
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movw %w[short], %[short]"
        : [short] "+r" (reg_short)
        :
        : "cc"
    );
    
    /* Operations that may generate SUBREG */
    reg_var = (reg_var & 0xFFFF0000) | reg_short;
    g_result = reg_var;
}

int main(void) {
    int i, j;
    volatile int temp;
    
    /* Initialize memory for complex addressing */
    g_mem_base = (volatile int*)malloc(256 * sizeof(int));
    for (i = 0; i < 256; i++) {
        g_mem_base[i] = i;
    }
    
    /* Initialize nested structures */
    struct level3 *l3_objs = (struct level3*)malloc(4 * sizeof(struct level3));
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            l3_objs[i].data[j] = i * 10 + j;
        }
    }
    
    struct level2 *l2_objs = (struct level2*)malloc(8 * sizeof(struct level2));
    for (i = 0; i < 8; i++) {
        l2_objs[i].l3 = &l3_objs[i % 4];
        l2_objs[i].offset = i * 2;
    }
    
    for (i = 0; i < 8; i++) {
        g_l1.l2_array[i] = &l2_objs[i];
    }
    g_l1.index = 3;
    
    /* Main loop combining all required patterns */
    for (i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT through bit-field assignments */
        if (i & 1) {
            g_bf.a = (i & 0x7);           /* 3-bit field */
            g_bf.c = (i & 0xFF);          /* 8-bit field - not byte aligned */
        } else {
            g_bf.b = (i & 0x1F);          /* 5-bit field */
            g_bf.d = (i & 0xFFFF);        /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART through partial register updates */
        if (i & 2) {
            /* Update low 16 bits only */
            g_split.parts.low = (uint16_t)(i * 3);
        } else {
            /* Update high 16 bits only */
            g_split.parts.high = (uint16_t)(i * 5);
        }
        
        /* Alternative STRICT_LOW_PART via pointer cast */
        volatile uint32_t *int_ptr = &g_split.full;
        volatile uint16_t *short_ptr = (volatile uint16_t*)int_ptr;
        short_ptr[(i & 1)] = (uint16_t)(i * 7);
        
        /* 3. SUBREG through register variables and vector operations */
        manipulate_with_subreg();
        
        /* Vector operations that may generate SUBREG */
        v4si vec1 = {i, i+1, i+2, i+3};
        v4si vec2 = {i*2, i*3, i*4, i*5};
        v4si vec3 = vec1 + vec2;
        temp = vec3[2];  /* Element extraction */
        
        /* 4. Complex MEM addressing modes */
        /* Multi-level pointer dereferencing */
        int idx1 = (i * 7) % 8;
        int idx2 = (i * 11) % 4;
        int idx3 = (i * 13) % 256;
        
        /* Chain: g_l1.l2_array[idx1]->l3->data[idx2] */
        volatile int *mem_ptr = 
            &g_l1.l2_array[idx1]->l3->data[idx2];
        
        /* Complex addressing with multiple indices */
        volatile int *complex_ptr = 
            &g_mem_base[
                idx3 + 
                g_l1.l2_array[idx1]->offset + 
                g_l1.index
            ];
        
        /* Use in assignments (both LHS and RHS) */
        *mem_ptr = *complex_ptr + temp;
        g_result = *mem_ptr + g_split.full + g_bf.d;
        
        /* Additional complex MEM with array indexing */
        volatile int (*matrix_ptr)[16] = (volatile int (*)[16])g_mem_base;
        matrix_ptr[i % 16][(i * 3) % 16] = g_result;
        
        /* Conditional break to prevent infinite loops in analysis */
        if (g_result > 10000) {
            break;
        }
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = 
        g_result + 
        g_split.full + 
        g_bf.a + g_bf.b + g_bf.c + g_bf.d +
        temp;
    
    /* Cleanup */
    free((void*)g_mem_base);
    free(l3_objs);
    free(l2_objs);
    
    return final_result & 0xFF;
}
