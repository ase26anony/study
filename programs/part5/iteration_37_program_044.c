/* Target RTL expressions: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, MEM */
#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Complex structure with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
} __attribute__((packed));

/* Union for STRICT_LOW_PART operations */
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Multi-level pointer structure for complex MEM addressing */
struct level2 {
    volatile int data[4][4];
};

struct level1 {
    volatile struct level2 *sub;
    volatile int extra;
};

struct root {
    volatile struct level1 *chain[3];
    volatile int index;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for SUBREG manipulation */
void manipulate_with_asm(volatile int *out) {
    /* Register variable for SUBREG operations */
    register int reg_var asm("eax") = *out;
    register short sub_reg asm("ax");
    
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movw %%ax, %[low]\n\t"
        "rorl $16, %%eax\n\t"
        "movw %%ax, %[high]\n\t"
        "rorl $16, %%eax\n\t"
        : [low] "=m" (((volatile short*)out)[0]),
          [high] "=m" (((volatile short*)out)[1])
        : "a" (reg_var)
        : "cc"
    );
    
    /* Force SUBREG through type conversion */
    sub_reg = (short)reg_var;
    reg_var = (reg_var & 0xFFFF0000) | (sub_reg + 1);
    
    /* More inline assembly with constraints */
    asm volatile (
        "addl $1, %%eax\n\t"
        : "+a" (reg_var)
        :
        : "cc"
    );
    
    *out = reg_var;
}

int main(void) {
    struct bitfield_struct bf = {0};
    union type_pun pun = {0};
    volatile v4si vec = {1, 2, 3, 4};
    
    /* Setup complex memory structure */
    struct level2 l2[2] = {0};
    struct level1 l1[2] = {0};
    struct root r = {0};
    
    l1[0].sub = &l2[0];
    l1[1].sub = &l2[1];
    r.chain[0] = &l1[0];
    r.chain[1] = &l1[1];
    r.index = 0;
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            l2[0].data[i][j] = i * 4 + j;
            l2[1].data[i][j] = 100 + i * 4 + j;
        }
    }
    
    /* Main loop targeting all RTL expressions */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT through bitfield assignments */
        if (i & 1) {
            bf.a = (i & 0x7);           /* 3-bit field */
            bf.b = (i & 0x1F) >> 3;     /* 5-bit field */
            bf.c = (i & 0xFF) >> 8;     /* 8-bit field */
            bf.d = (i & 0xFFFF) >> 16;  /* 16-bit field */
        } else {
            /* Alternative bitfield pattern */
            bf.a = (i >> 2) & 0x7;
            bf.b = (i >> 5) & 0x1F;
            bf.c = (i >> 10) & 0xFF;
            bf.d = (i >> 18) & 0xFFFF;
        }
        
        /* 2. STRICT_LOW_PART through partial updates */
        if (i & 2) {
            /* Update low 16 bits */
            pun.parts.low = i & 0xFFFF;
            /* Update high 16 bits */
            pun.parts.high = (i >> 16) & 0xFFFF;
        } else {
            /* Byte-wise updates */
            pun.bytes[0] = i & 0xFF;
            pun.bytes[1] = (i >> 8) & 0xFF;
            pun.bytes[2] = (i >> 16) & 0xFF;
            pun.bytes[3] = (i >> 24) & 0xFF;
        }
        
        /* 3. SUBREG through vector operations and register variables */
        volatile int temp = i;
        manipulate_with_asm(&temp);
        
        /* Vector operations that may generate SUBREG */
        v4si vec_temp = vec;
        vec_temp[i % 4] = temp;  /* Element access may use SUBREG */
        
        /* Explicit subregister access through pointer */
        volatile short *short_ptr = (volatile short*)&temp;
        short_ptr[0] = short_ptr[0] + 1;  /* STRICT_LOW_PART candidate */
        short_ptr[1] = short_ptr[1] - 1;
        
        /* 4. Complex MEM addressing modes */
        if (r.chain[r.index % 2] && r.chain[r.index % 2]->sub) {
            /* Multi-level pointer dereferencing with non-constant indices */
            int idx1 = (i * 3) % 4;
            int idx2 = (i * 7) % 4;
            
            /* Complex addressing: ptr->sub->array[idx1][idx2] */
            volatile int *mem_ptr = &r.chain[r.index % 2]->sub->data[idx1][idx2];
            
            /* Use in both LHS and RHS */
            int old_val = *mem_ptr;
            *mem_ptr = old_val + i + bf.a + bf.b;
            
            /* Even more complex: chain of pointers */
            if (i % 3 == 0) {
                volatile struct level1 **chain_ptr = &r.chain[r.index % 2];
                if (*chain_ptr) {
                    volatile int *extra_ptr = &(*chain_ptr)->extra;
                    *extra_ptr = (*extra_ptr + *mem_ptr) & 0xFF;
                }
            }
        }
        
        /* 5. Conditional context with switch */
        switch (i % 5) {
            case 0:
                /* More bitfield ops in switch */
                bf.a = (bf.b + bf.c) & 0x7;
                break;
            case 1:
                /* More partial updates */
                ((volatile char*)&pun.full)[i % 4] = i & 0xFF;
                break;
            case 2:
                /* Vector element manipulation */
                vec[i % 4] = vec[(i + 1) % 4] + 1;
                break;
            case 3:
                /* Complex memory with pointer arithmetic */
                if (r.chain[0]) {
                    int *base = &r.chain[0]->sub->data[0][0];
                    base[(i * 2) % 16] = base[(i * 3) % 16] + 1;
                }
                break;
            case 4:
                /* Mixed operations */
                temp = (bf.d << 16) | pun.parts.low;
                manipulate_with_asm(&temp);
                break;
        }
        
        /* Update global state to prevent elimination */
        global_counter += bf.a + pun.parts.low + temp;
        r.index = (r.index + 1) % 3;
        
        /* Prevent infinite loops in case of overflow */
        if (global_counter > 1000000) {
            global_counter = 0;
        }
    }
    
    /* Final computation using all variables */
    global_result = 
        bf.a + bf.b + bf.c + bf.d +
        pun.full +
        vec[0] + vec[1] + vec[2] + vec[3] +
        (r.chain[0] ? r.chain[0]->extra : 0) +
        (r.chain[1] ? r.chain[1]->extra : 0) +
        global_counter;
    
    return global_result & 0xFF;
}
