/* Target RTL expressions: ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, MEM */
#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
} bitfield_struct;

/* 2. Union for STRICT_LOW_PART operations */
typedef union {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* 3. Complex memory structure */
typedef struct {
    int data[8];
    struct inner {
        int matrix[3][3];
        int *ptr;
    } *inner_ptr;
} nested_struct;

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for direct RTL influence */
static inline void manipulate_with_asm(volatile uint32_t *val) {
    uint32_t temp;
    /* Suggest use of subregisters through constraints */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (temp)
        : "m" (*val)
        : "%eax"
    );
    *val ^= temp;
}

int main(void) {
    /* Initialize bitfield struct */
    bitfield_struct bf = {0};
    
    /* Initialize split integer */
    split_int si = { .full = 0x12345678 };
    
    /* Initialize complex memory structures */
    nested_struct outer;
    nested_struct *outer_ptr = &outer;
    nested_struct inner_actual;
    outer.inner_ptr = &inner_actual;
    
    /* Initialize array for complex addressing */
    int array[4][8][16];
    int *ptr_array[4];
    
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &array[i][0][0];
    }
    
    /* Register variable for SUBREG operations */
    register int reg_var asm("ebx") = 0xDEADBEEF;
    
    /* Vector for SUBREG operations */
    v4si vec = { 1, 2, 3, 4 };
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional to create branching */
        if (i & 1) {
            /* 1. ZERO_EXTRACT: Bitfield assignments */
            bf.a = (i & 0x7);           /* 3-bit field */
            bf.b = (i >> 3) & 0x1F;     /* 5-bit field */
            bf.c = (i * 3) & 0xFF;      /* 8-bit field */
            bf.d = (i * 100) & 0xFFFF;  /* 16-bit field */
            
            /* Use bitfields in computation */
            global_counter += bf.a + bf.b;
        } else {
            /* 2. STRICT_LOW_PART: Partial register updates */
            /* Update low 16 bits */
            si.parts.low = (i * 7) & 0xFFFF;
            
            /* Update specific byte */
            si.bytes[1] = (i * 13) & 0xFF;
            
            /* Pointer-based partial update */
            *((volatile uint16_t*)&si.full) = (i * 11) & 0xFFFF;
        }
        
        /* 3. SUBREG: Register variable operations */
        /* Truncate register variable */
        volatile uint16_t short_val = (uint16_t)reg_var;
        reg_var = (reg_var >> 8) | (short_val << 24);
        
        /* Vector element extraction (triggers SUBREG) */
        volatile int vec_element = vec[i % 4];
        vec[i % 4] = vec_element * 2;
        
        /* 4. Complex MEM addressing */
        /* Multi-level pointer dereferencing */
        if (outer.inner_ptr) {
            outer.inner_ptr->matrix[i % 3][(i + 1) % 3] = i;
            
            /* Chain of pointer accesses */
            int ***triple_ptr = (int***)&ptr_array;
            int val = (*triple_ptr)[i % 4][(i * 3) % 8 * 16 + (i * 5) % 16];
            global_result ^= val;
        }
        
        /* Array with complex indexing */
        array[i % 4][(i * 2) % 8][(i * 3) % 16] = i * i;
        
        /* 5. Inline assembly influencing RTL generation */
        manipulate_with_asm(&si.full);
        
        /* Combine values to prevent dead code elimination */
        global_result += bf.d + si.parts.low + reg_var + vec_element;
        
        /* Complex loop condition */
        if (global_result > 1000000) {
            global_result /= 2;
        }
    }
    
    /* Final computation using all manipulated variables */
    int final_result = 
        (bf.a << 24) | (bf.b << 16) | (bf.c << 8) | bf.d |
        si.full |
        reg_var |
        vec[0] |
        global_counter |
        global_result |
        array[0][0][0] |
        (outer.inner_ptr ? outer.inner_ptr->matrix[0][0] : 0);
    
    return final_result & 0xFF;
}
