/* Target RTL patterns: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, MEM */
#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Bitfield struct for ZERO_EXTRACT */
struct bitfields {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 11;
    volatile unsigned int d : 13;
} __attribute__((packed));

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex memory structure */
struct level3 {
    volatile int data[3];
};

struct level2 {
    volatile struct level3 *sub;
    volatile int extra;
};

struct level1 {
    volatile struct level2 *chain;
    volatile int id;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Inline assembly helper */
static inline void clobber_registers(void) {
    /* Clobber multiple registers to force spills/reloads */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
}

int main(void) {
    /* 1. Bitfield operations (ZERO_EXTRACT) */
    struct bitfields bf = {0};
    volatile struct bitfields *bf_ptr = &bf;
    
    /* 2. STRICT_LOW_PART via union */
    union split_int split = {0};
    volatile union split_int *split_ptr = &split;
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    volatile register short reg_short asm("bx") = 0;
    
    /* 4. Complex memory addressing */
    struct level3 l3 = {{1, 2, 3}};
    struct level2 l2 = {&l3, 4};
    struct level1 l1 = {&l2, 5};
    volatile struct level1 *l1_ptr = &l1;
    
    /* 5. Vector for SUBREG operations */
    v4si vec = {10, 20, 30, 40};
    volatile v4si *vec_ptr = &vec;
    
    /* 6. Multi-dimensional array */
    volatile int md_array[4][8][16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 16; k++)
                md_array[i][j][k] = i * 100 + j * 10 + k;
    
    /* Loop with conditional branches */
    for (int i = 0; i < 100; i++) {
        /* Conditional to create complex control flow */
        if (i & 1) {
            /* ZERO_EXTRACT: Bitfield assignment with non-byte-aligned field */
            bf_ptr->c = (i * 7) & 0x7FF;  /* 11-bit field */
            bf_ptr->b = (i * 3) & 0x1F;   /* 5-bit field */
            
            /* Complex MEM: Multi-level pointer dereference */
            int val = l1_ptr->chain->sub->data[(i >> 1) % 3];
            l1_ptr->chain->extra = val;
            
            /* Multi-dimensional array access with non-constant indices */
            int idx = (i * 13) % 4;
            int jdx = (i * 17) % 8;
            int kdx = (i * 23) % 16;
            md_array[idx][jdx][kdx] += i;
        } else {
            /* STRICT_LOW_PART: Partial register update */
            split_ptr->parts.low = i & 0xFFFF;
            split_ptr->bytes[1] = (i >> 8) & 0xFF;
            
            /* Another STRICT_LOW_PART pattern */
            ((volatile short*)&split.full)[0] = (i * 3) & 0xFFFF;
        }
        
        /* SUBREG: Register variable with smaller-type operation */
        reg_short = (reg_var >> 16) & 0xFFFF;
        reg_var = (reg_var * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Vector SUBREG access */
        int elem = vec[i % 4];
        vec_ptr[0][(i + 1) % 4] = elem * 2;
        
        /* Inline assembly with constraints */
        int temp;
        asm volatile (
            "movl %1, %%eax\n\t"
            "movw %%ax, %0\n\t"
            : "=r" (temp)
            : "r" (reg_var)
            : "eax"
        );
        
        /* More complex MEM addressing with pointer arithmetic */
        volatile int *mem_ptr = &md_array[0][0][0];
        mem_ptr += (i * 7) % 512;
        *mem_ptr = i;
        
        /* Switch statement for additional control flow complexity */
        switch (i % 4) {
            case 0:
                bf_ptr->a = i & 0x7;
                break;
            case 1:
                split_ptr->parts.high = (i >> 4) & 0xFFFF;
                break;
            case 2:
                /* Nested pointer chain */
                l1_ptr->chain->sub->data[0] = 
                    l1_ptr->chain->sub->data[1] + 
                    l1_ptr->chain->sub->data[2];
                break;
            case 3:
                /* Mixed operations */
                reg_var = split.full + bf_ptr->c;
                break;
        }
        
        /* Clobber registers to force spills */
        clobber_registers();
        
        /* Update global to prevent elimination */
        global_counter += bf_ptr->c + split.parts.low + reg_short;
    }
    
    /* Final computation using all variables */
    global_result = 
        bf.a * 1000000 +
        bf.b * 10000 +
        bf.c * 100 +
        split.full +
        reg_var +
        l1_ptr->chain->sub->data[0] +
        md_array[0][0][0];
    
    return global_result & 0xFF;
}
