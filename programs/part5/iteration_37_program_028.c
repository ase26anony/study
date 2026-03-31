/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */
/* For RTL analysis: add -dP -fdump-rtl-expand -fdump-rtl-sched1 -fdump-rtl-sched2 */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
} __attribute__((packed));

/* 2. Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* 3. Complex memory structure */
struct level2 {
    volatile int data[4][4];
};

struct level1 {
    volatile struct level2 *next;
    volatile int idx;
};

struct memory_chain {
    volatile struct level1 *chain[3];
    volatile int offsets[3];
};

/* Global variables to prevent optimization */
volatile struct bitfields g_bf = {0};
volatile union split_int g_split = {0};
volatile struct memory_chain g_mem;
volatile int g_array[8][8] = {{0}};

/* Function with register variables for SUBREG */
static int use_subreg(void) {
    /* 3. Register variable for SUBREG */
    register uint32_t reg_var asm("eax") = 0x12345678;
    register uint16_t reg_short asm("ax");
    
    /* Force SUBREG through truncation */
    asm volatile("" : "+r"(reg_var));
    reg_short = (uint16_t)reg_var;  /* Should generate SUBREG */
    
    /* Inline assembly with register constraints */
    uint32_t temp;
    asm volatile("movw %w1, %w0"  /* %w for word register */
                 : "=r"(temp)
                 : "r"(reg_short)
                 : "cc");
    
    return temp;
}

/* Function with complex memory addressing */
static void complex_mem_access(int i, int j) {
    /* 4. Complex memory addressing for MEM */
    volatile int ***triple_ptr;
    volatile int **double_ptr;
    volatile int *single_ptr;
    volatile int value;
    
    /* Create pointer chain */
    triple_ptr = (volatile int ***)&g_array;
    double_ptr = *triple_ptr;
    single_ptr = double_ptr[j];
    value = single_ptr[i];
    
    /* Write back with offset */
    single_ptr[i + 1] = value + 1;
    
    /* Structure pointer chain */
    if (g_mem.chain[0] && g_mem.chain[0]->next) {
        volatile int idx1 = g_mem.offsets[0];
        volatile int idx2 = g_mem.offsets[1];
        g_mem.chain[0]->next->data[idx1][idx2] = value;
    }
}

int main(void) {
    volatile int i, j, result = 0;
    volatile uint32_t combined = 0;
    
    /* Initialize memory chain */
    struct level2 l2 = {{0}};
    struct level1 l1 = {&l2, 0};
    g_mem.chain[0] = &l1;
    g_mem.offsets[0] = 1;
    g_mem.offsets[1] = 2;
    
    /* 5. Loop with combined operations */
    for (i = 0; i < 100; i++) {
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        g_bf.a = (i & 0x7);           /* 3-bit field */
        g_bf.b = ((i >> 3) & 0x1F);   /* 5-bit field */
        g_bf.c = ((i >> 8) & 0xFF);   /* 8-bit field */
        g_bf.d = ((i >> 16) & 0xFFFF);/* 16-bit field */
        
        /* Combine bitfields - may generate more complex RTL */
        combined = g_bf.a | (g_bf.b << 3) | (g_bf.c << 8) | (g_bf.d << 16);
        
        /* 2. STRICT_LOW_PART through union/pointer */
        if (i & 1) {
            /* Update low part only */
            g_split.parts.low = (i & 0xFFFF);
        } else {
            /* Update high part only */
            g_split.parts.high = ((i >> 16) & 0xFFFF);
        }
        
        /* Pointer cast for STRICT_LOW_PART */
        volatile uint16_t *short_ptr = (volatile uint16_t *)&combined;
        short_ptr[0] = (i & 0xFFFF);  /* Update low 16 bits */
        
        /* 3. SUBREG operations */
        result += use_subreg();
        
        /* 4. Complex memory accesses */
        complex_mem_access(i & 7, (i >> 3) & 7);
        
        /* Conditional with memory operands */
        if (g_array[i & 7][(i >> 3) & 7] > 50) {
            /* More complex addressing */
            volatile int (*array_ptr)[8] = g_array;
            array_ptr[(i >> 2) & 7][i & 7] = result;
        }
        
        /* Switch to create different basic blocks */
        switch (i % 4) {
            case 0:
                /* More bitfield ops */
                g_bf.a = (result & 0x7);
                break;
            case 1:
                /* More partial updates */
                ((volatile uint8_t *)&combined)[1] = (result & 0xFF);
                break;
            case 2:
                /* Register variable with smaller type */
                {
                    register uint32_t r asm("ebx") = combined;
                    register uint8_t rb asm("bl");
                    rb = (r & 0xFF);  /* SUBREG for byte register */
                    asm volatile("" : "+r"(r));
                    combined = r;
                }
                break;
            case 3:
                /* Nested memory addressing */
                if (g_mem.chain[0]) {
                    volatile int idx = g_mem.chain[0]->idx++;
                    g_mem.chain[0]->next->data[idx & 3][(idx >> 2) & 3] = result;
                }
                break;
        }
        
        /* Prevent loop elimination */
        if (combined > 0x7FFFFFFF) break;
    }
    
    /* Final computation using all modified values */
    result += g_bf.a + g_bf.b + g_bf.c + g_bf.d;
    result += g_split.full;
    result += g_array[0][0];
    
    return result & 0xFF;  /* Non-zero exit code */
}
