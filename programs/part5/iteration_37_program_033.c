/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* Requirement 1: Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} bitfield_struct;

/* Requirement 3: Register variables for SUBREG */
register uint32_t reg_var asm("r12");

/* Requirement 4: Complex memory structures */
typedef struct {
    int data[4][4];
    struct inner *next;
} inner;

typedef struct {
    inner *sub;
    long matrix[8][8];
    volatile int counter;
} outer;

/* Requirement 2: Union for STRICT_LOW_PART */
union type_pun {
    volatile uint32_t full;
    volatile uint16_t half[2];
    volatile uint8_t byte[4];
};

/* Vector extension for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Initialize variables */
    volatile bitfield_struct bf = {0};
    union type_pun pun = {0};
    outer *obj = (outer*)malloc(sizeof(outer));
    obj->sub = (inner*)malloc(sizeof(inner));
    
    /* Initialize array for memory accesses */
    int ***multi_array = (int***)malloc(10 * sizeof(int**));
    for (int i = 0; i < 10; i++) {
        multi_array[i] = (int**)malloc(8 * sizeof(int*));
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = (int*)malloc(16 * sizeof(int));
        }
    }
    
    /* Vector variable */
    v4si vec = {1, 2, 3, 4};
    
    /* Loop with conditional contexts (Requirement 5) */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 100;
    
    int result = 0;
    reg_var = 0x12345678;
    
    for (int i = 0; i < iterations; i++) {
        /* Requirement 1: Bitfield assignments triggering ZERO_EXTRACT */
        if (i % 3 == 0) {
            bf.field1 = (i & 0x7);           /* 3-bit field */
            bf.field3 = (i * 7) & 0xFF;      /* 8-bit field */
        } else if (i % 3 == 1) {
            bf.field2 = (i & 0x1F);          /* 5-bit field */
            bf.field4 = (i * 13) & 0xFFFF;   /* 16-bit field */
        }
        
        /* Requirement 2: STRICT_LOW_PART via union/pointer */
        if (i % 4 == 0) {
            /* Update low 16 bits */
            pun.half[0] = (i * 3) & 0xFFFF;
        } else if (i % 4 == 1) {
            /* Update specific byte */
            pun.byte[2] = (i * 5) & 0xFF;
        }
        
        /* Requirement 3: SUBREG operations */
        if (i % 5 == 0) {
            /* Register variable with truncation */
            uint16_t temp = (uint16_t)reg_var;
            reg_var = (reg_var >> 8) | (temp << 24);
            
            /* Vector element extraction */
            int elem = vec[i % 4];
            vec[i % 4] = elem * 2;
        }
        
        /* Requirement 4: Complex memory addressing */
        if (obj && obj->sub) {
            /* Multi-level pointer dereferencing */
            obj->sub->next = obj->sub;  /* Self-reference for complexity */
            obj->sub->data[i % 4][(i * 7) % 4] = i;
            
            /* Chain dereference */
            int val = obj->sub->data[(i * 3) % 4][i % 4];
            obj->matrix[val % 8][i % 8] = val;
            
            /* Multi-dimensional array with non-constant indices */
            if (i < 10 && i < 8) {
                multi_array[i][(i * 2) % 8][(i * 3) % 16] = val;
                result += multi_array[(i * 5) % 10][i % 8][(i * 7) % 16];
            }
        }
        
        /* Requirement 6: Inline assembly influencing RTL */
        if (i % 7 == 0) {
            uint32_t asm_var = i;
            uint16_t asm_short;
            uint8_t asm_byte;
            
            /* Assembly with register constraints */
            asm volatile (
                "movw %[input], %%ax\n\t"
                "movb %%ah, %[byte]\n\t"
                "movw %%ax, %[short]"
                : [byte] "=m" (asm_byte), [short] "=m" (asm_short)
                : [input] "r" (asm_var)
                : "ax", "memory"
            );
            
            pun.byte[1] = asm_byte;
            pun.half[0] = asm_short;
        }
        
        /* Conditional branch creating optimization opportunities */
        switch (i % 6) {
            case 0:
                bf.field1 = (bf.field2 + bf.field3) & 0x7;
                break;
            case 1:
                pun.full = (pun.full << 3) | (bf.field1 & 0x7);
                break;
            case 2:
                reg_var = (reg_var * 3) + (pun.full & 0xFF);
                break;
            case 3:
                if (obj) {
                    obj->counter = (obj->counter * 5) + (reg_var & 0xFFF);
                }
                break;
            case 4:
                vec = vec + (v4si){1, 1, 1, 1};
                break;
            default:
                result += i & 0xF;
        }
        
        /* Loop exit condition based on manipulated variables */
        if ((bf.field4 > 50000) || (pun.full > 0xFFFFFF) || (result > 1000000)) {
            break;
        }
    }
    
    /* Combine results to prevent dead code elimination */
    result += bf.field1 + bf.field2 + bf.field3 + bf.field4;
    result += pun.full;
    result += reg_var;
    result += obj ? obj->counter : 0;
    result += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 8; j++) {
            free(multi_array[i][j]);
        }
        free(multi_array[i]);
    }
    free(multi_array);
    
    if (obj) {
        free(obj->sub);
        free(obj);
    }
    
    return result & 0xFF;  /* Return non-zero result */
}
