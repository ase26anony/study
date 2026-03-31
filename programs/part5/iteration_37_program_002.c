/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Struct with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
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

/* Complex nested structure for memory addressing */
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
volatile struct bitfield_struct g_bf = {0};
volatile union split_int g_split = {0};
volatile struct level1 g_l1 = {0};
volatile v4si g_vector = {0};
volatile int g_index = 0;
volatile int g_result = 0;

/* Function with inline assembly for SUBREG */
int use_register_variable(int input) {
    /* Explicit register variable */
    register int reg_var asm("eax") = input;
    register short reg_short asm("ax");
    
    /* Inline assembly that uses subregisters */
    asm volatile (
        "movw %%ax, %0\n\t"
        "addl $1, %%eax\n\t"
        : "=m" (reg_short)
        : "a" (reg_var)
        : "cc"
    );
    
    /* Operation requiring truncation */
    reg_short = (reg_short & 0xFF) + 1;
    
    /* More inline assembly with constraints */
    asm volatile (
        "addb $5, %%al\n\t"
        : "+a" (reg_var)
        :
        : "cc"
    );
    
    return reg_var;
}

int main(int argc, char *argv[]) {
    /* Initialize complex memory structure */
    struct level3 *l3 = malloc(sizeof(struct level3));
    struct level2 *l2 = malloc(sizeof(struct level2));
    
    for (int i = 0; i < 4; i++) {
        l3->data[i] = i * 10;
    }
    
    l2->l3 = l3;
    l2->extra = 99;
    g_l1.l2_array[0] = l2;
    g_l1.count = 1;
    
    /* Loop with multiple RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. BITFIELD assignments (ZERO_EXTRACT) */
        if (i & 1) {
            g_bf.a = (i & 0x7);           /* 3-bit field */
            g_bf.b = ((i >> 3) & 0x1F);   /* 5-bit field */
            g_bf.c = ((i >> 8) & 0xFF);   /* 8-bit field */
            g_bf.d = i * 2;               /* 16-bit field */
        } else {
            /* Different pattern */
            g_bf.a = (i >> 1) & 0x7;
            g_bf.b = (i >> 4) & 0x1F;
            g_bf.c = (i >> 9) & 0xFF;
            g_bf.d = i * 3;
        }
        
        /* 2. STRICT_LOW_PART via union/pointer */
        if (i & 2) {
            /* Update low part only */
            g_split.parts.low = i & 0xFFFF;
            
            /* Pointer cast for strict low part */
            *((volatile uint16_t*)&g_split.full) = (i * 7) & 0xFFFF;
        } else {
            /* Update high part */
            g_split.parts.high = (i >> 16) & 0xFFFF;
            
            /* Array-style access for low part */
            volatile uint16_t *ptr = (volatile uint16_t*)&g_split.full;
            ptr[0] = (i * 11) & 0xFFFF;  /* Low word */
        }
        
        /* 3. SUBREG via register variables and vector operations */
        int reg_result = use_register_variable(i);
        
        /* Vector operations that may generate SUBREG */
        v4si vec1 = {i, i+1, i+2, i+3};
        v4si vec2 = {i*2, i*3, i*4, i*5};
        g_vector = vec1 + vec2;
        
        /* Extract element (potential SUBREG) */
        int elem = ((volatile int*)&g_vector)[i % 4];
        reg_result += elem;
        
        /* 4. Complex MEM addressing */
        if (g_l1.l2_array[0] && g_l1.l2_array[0]->l3) {
            /* Multi-level pointer dereferencing */
            volatile int *mem_ptr = &g_l1.l2_array[0]->l3->data[i % 4];
            
            /* Complex addressing with computation */
            *mem_ptr += reg_result;
            
            /* Even more complex chain */
            volatile int val = g_l1.l2_array[0]->l3->data[(i + 1) % 4] +
                             g_l1.l2_array[0]->extra +
                             g_l1.count;
            
            /* Array indexing with non-constant index */
            volatile int *base = g_l1.l2_array[0]->l3->data;
            base[g_index % 4] = val;
            g_index++;
        }
        
        /* 5. Conditional with mixed operations */
        switch (i % 5) {
            case 0:
                /* More bitfield in switch */
                g_bf.a = (g_bf.b + g_bf.c) & 0x7;
                break;
            case 1:
                /* Memory update */
                if (g_l1.l2_array[0]) {
                    g_l1.l2_array[0]->extra = i;
                }
                break;
            case 2:
                /* Register/SUBREG operation */
                {
                    register uint32_t r asm("ebx") = g_split.full;
                    register uint16_t r_low asm("bx");
                    asm volatile ("" : "=r" (r_low) : "0" (r));
                    r_low += 1;
                    g_split.full = r;
                }
                break;
            case 3:
                /* Vector element update */
                ((volatile int*)&g_vector)[0] = i * 100;
                break;
            case 4:
                /* Combined operation */
                g_bf.d = g_split.parts.low;
                break;
        }
        
        /* Prevent loop elimination */
        g_result += reg_result + g_bf.a + g_bf.d + g_split.full;
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = g_result + 
                      g_bf.b * 2 + 
                      g_bf.c * 3 + 
                      g_split.parts.high +
                      ((int*)&g_vector)[0];
    
    /* Cleanup */
    free(l3);
    free(l2);
    
    return final_result % 256;
}
