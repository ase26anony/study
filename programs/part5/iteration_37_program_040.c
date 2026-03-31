/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int low : 5;      /* Not byte-aligned */
    volatile unsigned int middle : 11;  /* Not byte-aligned */
    volatile unsigned int high : 16;    /* Aligned to 16-bit boundary */
} bitfield_struct;

/* For complex MEM addressing */
typedef struct {
    int data[4][4];
} matrix;

typedef struct {
    matrix *m1;
    matrix *m2;
    int index;
} container;

/* For SUBREG operations with register variables */
#ifdef __x86_64__
register uint64_t reg_var asm("r12");
#else
register uint32_t reg_var asm("eax");
#endif

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Union for type-punning (STRICT_LOW_PART) */
union type_pun {
    uint32_t full;
    volatile uint16_t half[2];
    volatile uint8_t bytes[4];
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for direct RTL influence */
void inline_asm_ops(volatile int *ptr, int idx) {
    /* Use 'h' modifier for high byte, 'Q' for memory */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%ah, %0\n\t"
        : "=Q"(*ptr)
        : "r"(idx)
        : "%eax", "memory"
    );
}

int main(void) {
    /* Initialize bitfield struct for ZERO_EXTRACT */
    bitfield_struct bf = {0};
    
    /* Initialize union for STRICT_LOW_PART */
    union type_pun pun = {0x12345678};
    
    /* Initialize vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    
    /* Setup complex memory structure for MEM operations */
    matrix m1 = {{{0}}};
    matrix m2 = {{{0}}};
    container c = {&m1, &m2, 0};
    
    /* Initialize register variable */
    reg_var = 0xDEADBEEF;
    
    /* Loop with multiple operations */
    for (int i = 0; i < 100; i++) {
        /* ZERO_EXTRACT: Bitfield assignments (not byte-aligned) */
        bf.low = i & 0x1F;           /* 5-bit field */
        bf.middle = (i * 3) & 0x7FF; /* 11-bit field */
        bf.high = (i * 7) & 0xFFFF;  /* 16-bit field */
        
        /* STRICT_LOW_PART: Partial register updates */
        if (i & 1) {
            /* Update low 16 bits */
            pun.half[0] = (uint16_t)(i * 11);
            
            /* Update specific byte */
            pun.bytes[2] = (uint8_t)(i * 13);
        } else {
            /* Update high 16 bits */
            pun.half[1] = (uint16_t)(i * 17);
        }
        
        /* SUBREG: Operations on register variable */
        {
            /* Truncate register variable */
            uint16_t truncated = (uint16_t)reg_var;
            
            /* Vector element extraction (creates SUBREG) */
            int elem = vec[i % 4];
            
            /* Combine with register variable */
            reg_var = (reg_var & 0xFFFF0000) | (truncated + elem);
            
            /* Vector update with scalar */
            vec[i % 4] = (int)(reg_var & 0xFF);
        }
        
        /* Complex MEM addressing: Multi-level pointer dereferencing */
        if (c.m1 && c.m2) {
            /* Chain of pointer accesses with non-constant indices */
            int idx1 = (i * 2) % 4;
            int idx2 = (i * 3) % 4;
            
            /* Complex addressing: ptr->sub->array[i][j] */
            c.m1->data[idx1][idx2] = c.m2->data[idx2][idx1] + i;
            
            /* More complex addressing with pointer arithmetic */
            int *ptr = &c.m1->data[0][0];
            ptr[(idx1 * 4 + idx2) ^ 1] = ptr[(idx2 * 4 + idx1) ^ 1] * 2;
        }
        
        /* Inline assembly influencing surrounding RTL */
        volatile int asm_temp = 0;
        inline_asm_ops(&asm_temp, i);
        
        /* Conditional based on operations */
        if ((bf.low ^ pun.bytes[0]) > 100) {
            c.index++;
        } else if (reg_var & 0x1000) {
            c.index--;
        }
        
        /* Switch statement for additional control flow */
        switch (i % 5) {
            case 0:
                bf.middle = (bf.middle << 1) | 1;
                break;
            case 1:
                pun.full = (pun.full >> 4) ^ 0xABCD;
                break;
            case 2:
                reg_var = (reg_var << 3) | (reg_var >> 29);
                break;
            case 3:
                /* More complex MEM addressing */
                if (c.m1) {
                    int (*arr_ptr)[4] = c.m1->data;
                    arr_ptr[(i+1)%4][(i+2)%4] = arr_ptr[(i+2)%4][(i+1)%4] + 1;
                }
                break;
            case 4:
                /* Vector operation creating SUBREG */
                v4si temp_vec = vec * 2;
                vec = vec + (temp_vec >> 1);
                break;
        }
        
        global_counter++;
    }
    
    /* Combine all results to prevent dead code elimination */
    global_result = 
        bf.low + bf.middle + bf.high +
        pun.full +
        (int)(reg_var & 0xFFFFFFFF) +
        vec[0] + vec[1] + vec[2] + vec[3] +
        c.m1->data[0][0] +
        global_counter;
    
    return global_result % 256;
}
