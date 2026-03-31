/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* Structure with bit-fields for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 9;
    volatile unsigned int field4 : 11;
} bitfield_struct;

/* Union for STRICT_LOW_PART operations */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Complex nested structure for memory addressing */
typedef struct {
    int data[8];
    struct inner {
        int matrix[3][3];
        struct inner *next;
    } *inner_ptr;
} nested_struct;

/* Function using inline assembly with register constraints */
static inline void asm_operations(volatile int *a, volatile int *b) {
    /* Use specific register constraints to influence RTL generation */
    register int reg_a asm("eax") = *a;
    register int reg_b asm("ebx") = *b;
    
    /* Assembly with modifiers that may generate SUBREG */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "movb %%al, %%cl\n\t"      /* Low byte extraction */
        "movw %%ax, %%dx\n\t"      /* Low word extraction */
        : "+a" (reg_a), "=c" (reg_b)
        : "b" (reg_b)
        : "dx", "cc"
    );
    
    *a = reg_a;
    *b = reg_b;
}

int main(void) {
    volatile bitfield_struct bf = {0};
    volatile split_int split = {0};
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Register variable for SUBREG operations */
    register int reg_var asm("esi") = 0x12345678;
    volatile int temp;
    
    /* Multi-dimensional array with pointer chains for complex MEM addressing */
    int array1[16][16];
    int array2[8][8][8];
    nested_struct *nested = malloc(sizeof(nested_struct));
    nested->inner_ptr = malloc(sizeof(nested_struct::inner));
    nested->inner_ptr->next = NULL;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array1[i][j] = i * j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                array2[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Main loop with mixed operations */
    for (counter = 0; counter < 100; counter++) {
        /* 1. Bit-field assignments for ZERO_EXTRACT */
        bf.field1 = (counter & 0x1F);          /* 5-bit field */
        bf.field2 = ((counter * 3) & 0x7F);    /* 7-bit field */
        bf.field3 = ((counter * 5) & 0x1FF);   /* 9-bit field */
        bf.field4 = ((counter * 7) & 0x7FF);   /* 11-bit field */
        
        /* 2. STRICT_LOW_PART operations via union/pointer */
        if (counter & 1) {
            split.parts.low = counter * 2;     /* Update low 16 bits */
        } else {
            split.bytes[1] = counter;          /* Update single byte */
        }
        
        /* 3. SUBREG operations with register variable */
        reg_var = reg_var * 3 + counter;
        /* Force truncation through smaller type operation */
        temp = (int16_t)reg_var;               /* May generate SUBREG */
        
        /* 4. Complex memory addressing for MEM */
        /* Chain of pointer dereferences and array indexing */
        int idx = counter & 0xF;
        nested->inner_ptr->matrix[idx % 3][(idx + 1) % 3] = 
            array1[idx][counter % 16] + 
            array2[idx % 8][(counter / 8) % 8][counter % 8];
        
        /* 5. Inline assembly influencing surrounding RTL */
        asm_operations(&temp, &result);
        
        /* Conditional branching to create complex control flow */
        switch (counter % 4) {
            case 0:
                result += bf.field1 + split.parts.low;
                break;
            case 1:
                result += bf.field2 + temp;
                break;
            case 2:
                /* More complex memory addressing */
                result += nested->inner_ptr->matrix[0][1] * 
                         array1[counter % 16][(counter + 1) % 16];
                break;
            case 3:
                result += reg_var & 0xFFFF;  /* Another SUBREG opportunity */
                break;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r" (counter) : : "memory");
    }
    
    /* Combine all results to prevent dead code elimination */
    int final_result = 
        bf.field1 + bf.field2 + bf.field3 + bf.field4 +
        split.full + temp + result + reg_var +
        nested->inner_ptr->matrix[0][0] +
        array1[0][0] + array2[0][0][0];
    
    /* Cleanup */
    free(nested->inner_ptr);
    free(nested);
    
    return final_result & 0xFF;  /* Return non-zero result */
}
