/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int f1 : 5;    /* Non-byte aligned bitfield */
    volatile unsigned int f2 : 7;    /* Another non-byte aligned */
    volatile unsigned int f3 : 12;   /* Crosses byte boundary */
    volatile unsigned int f4 : 8;    /* Byte aligned */
} bitfield_struct;

/* For complex MEM addressing */
typedef struct {
    int data[4][4];
} matrix;

typedef struct {
    matrix *m1;
    matrix *m2;
    volatile int counter;
} nested;

/* For SUBREG operations with register variables */
#ifdef __x86_64__
register uint64_t reg_var asm("r12");
#else
register uint32_t reg_var asm("eax");
#endif

/* Vector type for SUBREG extraction */
typedef int v4si __attribute__((vector_size(16)));

/* Union for STRICT_LOW_PART type punning */
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Global variables to prevent optimization */
volatile int global_index = 0;
volatile int global_mod = 3;
volatile int global_limit = 1000;

/* Function with inline assembly for direct RTL influence */
static inline void asm_clobber_and_hint(volatile int *ptr) {
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m" (*ptr)
        : "r" (global_index)
        : "%eax", "memory"
    );
}

int main(void) {
    /* Initialize bitfield struct */
    bitfield_struct bf = {0};
    volatile bitfield_struct *bf_ptr = &bf;
    
    /* Initialize union for STRICT_LOW_PART */
    union type_pun pun = {0};
    
    /* Initialize complex memory structures */
    matrix m1, m2;
    nested n1, n2;
    
    /* Initialize matrices */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m1.data[i][j] = i * 4 + j;
            m2.data[i][j] = (i * 4 + j) * 2;
        }
    }
    
    n1.m1 = &m1;
    n1.m2 = &m2;
    n1.counter = 0;
    
    n2.m1 = &m2;
    n2.m2 = &m1;
    n2.counter = 0;
    
    /* Initialize register variable */
    reg_var = 0x12345678;
    
    /* Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    volatile v4si *vec_ptr = &vec;
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < global_limit; i++) {
        int idx = i % 16;
        int row = idx / 4;
        int col = idx % 4;
        
        /* 1. BITFIELD OPERATIONS (ZERO_EXTRACT) */
        /* Multiple non-byte-aligned bitfield assignments */
        if (i & 1) {
            bf_ptr->f1 = (i & 0x1F);          /* 5-bit field */
            bf_ptr->f3 = (i & 0xFFF);         /* 12-bit field crossing boundary */
        } else {
            bf_ptr->f2 = (i & 0x7F);          /* 7-bit field */
            bf_ptr->f4 = (i & 0xFF);          /* 8-bit field */
        }
        
        /* 2. STRICT_LOW_PART operations */
        /* Update parts of integer through union/pointer */
        if (i & 2) {
            /* Update low 16 bits */
            pun.parts.low = (uint16_t)(i * 3);
        } else {
            /* Update single byte */
            pun.bytes[1] = (uint8_t)(i + 5);
        }
        
        /* Pointer cast for STRICT_LOW_PART */
        volatile uint16_t *short_ptr = (volatile uint16_t*)&pun.full;
        short_ptr[(i >> 2) & 1] = (uint16_t)(i * 7);
        
        /* 3. SUBREG operations */
        /* Use register variable with smaller type operations */
        uint32_t temp = reg_var;
        if (i & 4) {
            /* Truncation to 16-bit */
            uint16_t low_half = (uint16_t)temp;
            pun.parts.high = low_half;
        }
        
        /* Vector element extraction (likely generates SUBREG) */
        int vec_elem = vec_ptr[0][(i >> 1) & 3];
        pun.full += vec_elem;
        
        /* 4. COMPLEX MEMORY ADDRESSING */
        /* Multi-level pointer dereferencing with non-constant indices */
        nested *current = (i & 8) ? &n1 : &n2;
        
        /* Complex addressing: ptr->sub->array[i][j] */
        int val1 = current->m1->data[row][col];
        int val2 = current->m2->data[col][row];
        
        /* Even more complex with pointer arithmetic */
        int *base_ptr = &current->m1->data[0][0];
        int offset = ((row * 4 + col) * global_mod) & 15;
        int val3 = *(base_ptr + offset);
        
        /* Update through complex address */
        current->m1->data[row][col] = val1 + val2 - val3;
        
        /* 5. INLINE ASSEMBLY for direct RTL influence */
        asm_clobber_and_hint(&current->counter);
        
        /* Combine results */
        reg_var = (reg_var * 1103515245 + 12345) ^ pun.full;
        
        /* Update global based on bitfield */
        global_index = bf.f1 | (bf.f2 << 5) | (bf.f3 << 12);
        
        /* Conditional break based on complex condition */
        if (reg_var > 0x80000000 && current->counter > 100) {
            global_limit = i + 50;  /* Modify loop limit */
        }
        
        /* Switch statement for additional control flow */
        switch (i & 7) {
            case 0:
                bf_ptr->f1 = reg_var & 0x1F;
                break;
            case 1:
                pun.bytes[2] = (reg_var >> 8) & 0xFF;
                break;
            case 2:
                vec_ptr[0][0] = reg_var & 0xFF;
                break;
            case 3:
                current->m2->data[0][0] = reg_var;
                break;
            default:
                /* Mix operations */
                bf_ptr->f4 = pun.bytes[0];
                pun.parts.low = bf.f3;
                break;
        }
        
        /* Prevent loop elimination */
        if (i == global_limit - 1) {
            global_mod = (global_mod * 2) & 7;
        }
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = 
        (bf.f1 << 0) |
        (bf.f2 << 5) |
        (bf.f3 << 12) |
        (bf.f4 << 24);
    
    result ^= pun.full;
    result ^= (int)(reg_var & 0xFFFFFFFF);
    result ^= n1.m1->data[0][0];
    result ^= n2.m2->data[3][3];
    
    /* Extract from vector */
    result += vec[0] + vec[1] + vec[2] + vec[3];
    
    return result & 0xFF;  /* Return non-zero result */
}
