/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

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
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Complex nested structure for memory addressing */
struct level3 {
    volatile int data[4];
};

struct level2 {
    volatile struct level3 *sub[2];
    volatile int offset;
};

struct level1 {
    volatile struct level2 *chain;
    volatile int index;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct g_bf = {0};
volatile union type_pun g_pun = {0};
volatile struct level1 g_root = {0};
volatile int g_array[8][8] = {0};
volatile v4si g_vector = {0};

/* Function with inline assembly for SUBREG */
int subreg_operation(volatile int x) {
    /* Register variable for SUBREG */
    register int reg_var asm("eax") = x;
    
    /* Inline assembly that uses byte register */
    asm volatile (
        "movb %%al, %%ah\n\t"          /* Move low byte to high byte */
        "addb $1, %%al\n\t"            /* Modify low byte */
        : "+a" (reg_var)               /* Input/output in eax */
        : 
        : "cc"
    );
    
    /* Operation that requires truncation */
    volatile short trunc = (short)reg_var;
    
    return trunc + (reg_var >> 8);
}

int main(void) {
    volatile int i, j, result = 0;
    volatile int *mem_ptr;
    
    /* Initialize structures */
    struct level3 l3 = {{1, 2, 3, 4}};
    struct level2 l2 = {{&l3, NULL}, 1};
    struct level1 l1 = {&l2, 2};
    g_root = l1;
    
    /* Initialize array */
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            g_array[i][j] = i * 8 + j;
        }
    }
    
    /* Initialize vector */
    g_vector = (v4si){10, 20, 30, 40};
    
    /* Main loop with various operations */
    for (i = 0; i < 100; i++) {
        /* 1. Bit-field assignments (ZERO_EXTRACT) */
        if (i & 1) {
            g_bf.a = (i & 0x7);                /* 3-bit field */
            g_bf.b = ((i >> 3) & 0x1F);        /* 5-bit field */
        } else {
            g_bf.c = (i & 0xFF);               /* 8-bit field */
            g_bf.d = (i * 3) & 0xFFFF;         /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART via union/pointer */
        if (i & 2) {
            /* Update low part through union */
            g_pun.parts.low = (i & 0xFFFF);
        } else {
            /* Update low part through pointer cast */
            *((volatile uint16_t*)&g_pun.full) = (i * 5) & 0xFFFF;
        }
        
        /* 3. SUBREG operations */
        result += subreg_operation(i);
        
        /* 4. Complex memory addressing (MEM) */
        if (g_root.chain && g_root.chain->sub[0]) {
            /* Multi-level pointer dereferencing */
            mem_ptr = &g_root.chain->sub[0]->data[g_root.index];
            *mem_ptr += i;
            
            /* Array indexing with non-constant expression */
            g_array[(i >> 1) & 0x7][(i >> 4) & 0x7] = *mem_ptr;
            
            /* Chain of structure accesses */
            result += g_root.chain->sub[0]->data[g_root.chain->offset];
        }
        
        /* 5. Vector operations with element extraction */
        if (i & 4) {
            volatile int elem = g_vector[1] + g_vector[3];
            g_vector[0] = elem & 0xFF;
            g_vector[2] = (elem >> 8) & 0xFF;
        }
        
        /* 6. Additional inline assembly with constraints */
        register int temp asm("ebx") = result;
        asm volatile (
            "addl %%eax, %%ebx\n\t"
            "movw %%bx, %%cx\n\t"      /* Word operation for subreg */
            : "+b" (temp)
            : "a" (i)
            : "cx", "cc"
        );
        result = temp;
        
        /* Conditional break to prevent infinite loop */
        if (result > 10000) {
            break;
        }
    }
    
    /* Combine all results to prevent dead code elimination */
    volatile int final_result = 
        g_bf.a + g_bf.b + g_bf.c + g_bf.d +
        g_pun.full +
        result +
        g_array[0][0] +
        g_vector[0];
    
    return final_result & 0xFF;
}
