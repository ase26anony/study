/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

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
    volatile uint8_t bytes[4];
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
    volatile struct level2 *l2_array[8];
    volatile int idx;
};

/* Vector type for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile struct bitfields g_bf = {0};
volatile union split_int g_split = {0};
volatile struct level1 g_l1;
volatile int g_counter = 0;
volatile int g_result = 0;

/* Function with inline assembly for register manipulation */
void manipulate_registers(volatile int *out) {
    register int reg_var asm("eax") = *out;
    register short reg_short asm("bx");
    
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movw %%ax, %0\n\t"
        "movb %%ah, %1\n\t"
        : "=r"(reg_short), "=r"(*out)
        : "0"(reg_var)
        : "memory"
    );
    
    /* More complex assembly with memory constraints */
    asm volatile (
        "addl $1, %0\n\t"
        "movl %0, %1\n\t"
        : "+r"(reg_var), "=m"(*out)
        :
        : "cc"
    );
}

int main() {
    volatile struct bitfields bf = {0};
    volatile union split_int split = {0};
    volatile v4si vec = {1, 2, 3, 4};
    volatile int temp;
    
    /* Initialize complex memory structure */
    struct level3 *l3 = malloc(sizeof(struct level3));
    struct level2 *l2 = malloc(sizeof(struct level2));
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            l3->data[i][j] = i * 4 + j;
        }
    }
    
    l2->l3 = l3;
    l2->offset = 7;
    g_l1.l2_array[0] = l2;
    g_l1.idx = 0;
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        if (i & 1) {
            bf.f1 = (i & 0x7);           /* 3-bit field */
            bf.f2 = (i >> 3) & 0x1F;     /* 5-bit field */
            bf.f3 = (i * 3) & 0xFF;      /* 8-bit field */
            bf.f4 = (i * 7) & 0xFFFF;    /* 16-bit field */
            g_bf = bf;  /* Force memory write */
        }
        
        /* 2. STRICT_LOW_PART through union/pointer */
        if (i & 2) {
            /* Update low 16 bits */
            split.parts.low = i & 0xFFFF;
            /* Update specific byte */
            split.bytes[1] = (i >> 8) & 0xFF;
            
            /* Pointer cast for strict low part */
            *((volatile uint16_t*)&split.full) = (i * 2) & 0xFFFF;
            
            g_split = split;
        }
        
        /* 3. SUBREG through vector operations and register variables */
        if (i & 4) {
            /* Vector operation that may generate SUBREG */
            v4si vec2 = vec + (v4si){i, i, i, i};
            temp = vec2[0] + vec2[1];  /* Element extraction */
            
            /* Register variable with smaller type operation */
            register int reg_eax asm("eax") = temp;
            register short reg_bx asm("bx");
            
            /* Force subregister operation */
            reg_bx = (short)reg_eax;
            reg_eax = (reg_eax & 0xFFFF0000) | reg_bx;
            temp = reg_eax;
        }
        
        /* 4. Complex memory addressing (MEM) */
        if (i & 8) {
            /* Multi-level pointer dereferencing */
            int val = g_l1.l2_array[g_l1.idx]->l3->data
                     [i % 4][(i * 3) % 4];
            
            /* Array indexing with non-constant expression */
            g_l1.l2_array[g_l1.idx]->l3->data
                [(i + 1) % 4][(i * 2) % 4] = val + i;
            
            /* Chain of pointer accesses */
            volatile int ***ptr_chain = (volatile int***)&g_l1.l2_array[0]->l3->data;
            temp = (*ptr_chain)[i % 4][(i + 2) % 4];
        }
        
        /* 5. Inline assembly influencing RTL generation */
        manipulate_registers(&temp);
        
        /* Combine results to prevent elimination */
        g_counter += bf.f1 + split.parts.low + temp;
        
        /* Conditional branch with complex expression */
        switch (i % 5) {
            case 0:
                g_result ^= bf.f2;
                break;
            case 1:
                g_result += split.parts.high;
                break;
            case 2:
                g_result |= temp;
                break;
            case 3:
                g_result &= g_l1.l2_array[0]->l3->data[0][0];
                break;
            case 4:
                g_result = g_result << 1;
                break;
        }
    }
    
    /* Cleanup */
    free(l2);
    free(l3);
    
    /* Return non-deterministic result to prevent optimization */
    return (g_result + g_counter + bf.f3 + split.bytes[2]) & 0xFF;
}
