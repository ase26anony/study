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

/* Requirement 2: Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Requirement 3: Vector extension for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Requirement 4: Complex memory structures */
typedef struct node {
    int data;
    struct node *next;
    int array[3][3];
} node_t;

/* Requirement 3: Register variable declaration */
register int reg_var asm("r12");

/* Inline assembly helper for Requirement 6 */
static inline void clobber_registers(void) {
    asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
}

int main(void) {
    /* Initialize variables */
    volatile bitfield_struct bf = {0};
    volatile split_int split = {.full = 0x12345678};
    volatile v4si vec = {1, 2, 3, 4};
    volatile int *mem_ptr;
    volatile int result = 0;
    
    /* Requirement 4: Multi-level pointer structure */
    node_t *node1 = malloc(sizeof(node_t));
    node_t *node2 = malloc(sizeof(node_t));
    
    if (!node1 || !node2) return -1;
    
    node1->next = node2;
    node2->next = node1;
    
    /* Initialize arrays */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            node1->array[i][j] = i * 3 + j;
            node2->array[i][j] = 9 + i * 3 + j;
        }
    }
    
    /* Requirement 5: Loop with complex operations */
    for (volatile int counter = 0; counter < 100; counter++) {
        /* Requirement 1: Bitfield assignments (ZERO_EXTRACT) */
        bf.field1 = (counter & 0x7);           /* 3-bit field */
        bf.field2 = ((counter >> 3) & 0x1F);   /* 5-bit field */
        bf.field3 = ((counter >> 8) & 0xFF);   /* 8-bit field */
        bf.field4 = counter * 37;              /* 16-bit field */
        
        /* Requirement 2: STRICT_LOW_PART via union */
        split.parts.low = counter * 7;
        split.bytes[1] = counter & 0xFF;
        
        /* Requirement 2: STRICT_LOW_PART via pointer cast */
        *((volatile uint16_t*)&split.full) = counter * 13;
        
        /* Requirement 3: SUBREG via register variable */
        reg_var = counter * 3;
        /* Truncation operation */
        volatile char reg_trunc = (char)reg_var;
        result += reg_trunc;
        
        /* Requirement 3: SUBREG via vector extraction */
        volatile int vec_element = vec[counter % 4];
        vec = vec + (v4si){1, 1, 1, 1};
        
        /* Requirement 4: Complex memory addressing */
        mem_ptr = &node1->array[(counter / 3) % 3][counter % 3];
        *mem_ptr += node2->array[counter % 3][(counter / 3) % 3];
        
        /* Multi-level pointer dereference */
        volatile int chain_value = node1->next->array[1][1];
        node1->data = chain_value + counter;
        
        /* Requirement 6: Inline assembly with constraints */
        int asm_input = counter;
        int asm_output;
        
        /* Assembly that suggests subregister use */
        asm volatile (
            "movl %1, %%eax\n\t"
            "movb %%al, %0\n\t"
            : "=r" (asm_output)
            : "r" (asm_input)
            : "%eax"
        );
        
        result += asm_output;
        
        /* More complex addressing in assembly */
        asm volatile (
            "leaq %1, %%rax\n\t"
            "movl (%%rax), %%ebx\n\t"
            "addl %%ebx, %0\n\t"
            : "+r" (result)
            : "r" (&node1->array[0][0])
            : "%rax", "%rbx", "memory"
        );
        
        /* Clobber registers to force reloads */
        clobber_registers();
        
        /* Conditional branching */
        if (counter % 7 == 0) {
            bf.field1 = 0;
            split.parts.high = counter;
        } else if (counter % 5 == 0) {
            reg_var = counter * 11;
        } else {
            /* Switch statement for more control flow */
            switch (counter % 4) {
                case 0: vec[counter % 4] = result; break;
                case 1: *((volatile short*)&split.full) = result; break;
                case 2: bf.field2 = result & 0x1F; break;
                case 3: reg_var = result; break;
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    result += bf.field1 + bf.field2 + bf.field3 + bf.field4;
    result += split.full;
    result += vec[0] + vec[1] + vec[2] + vec[3];
    result += node1->data + node2->data;
    result += node1->array[0][0] + node2->array[2][2];
    
    /* Cleanup */
    free(node1);
    free(node2);
    
    return result & 0xFF;  /* Return non-zero result */
}
