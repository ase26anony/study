/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers program.c -o program */

#include <stdint.h>
#include <stdlib.h>

/* Requirement 1: Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} bitfield_struct;

/* Requirement 3: Vector extensions for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Requirement 4: Complex memory structures */
typedef struct node {
    int data;
    struct node *next;
    int array[4][4];
} node_t;

/* Requirement 2: Union for STRICT_LOW_PART */
union type_pun {
    volatile uint32_t full;
    volatile uint16_t half[2];
    volatile uint8_t byte[4];
};

/* Inline assembly helper for Requirement 6 */
static inline void clobber_registers(void) {
    __asm__ volatile (
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "ecx", "edx", "memory"
    );
}

int main(void) {
    /* Initialize variables */
    bitfield_struct bf = {0};
    union type_pun pun = {0};
    v4si vec = {1, 2, 3, 4};
    register int reg_var asm("eax") = 0x12345678;
    
    /* Requirement 4: Complex memory structure */
    node_t *nodes = malloc(3 * sizeof(node_t));
    if (!nodes) return 1;
    
    for (int i = 0; i < 3; i++) {
        nodes[i].data = i;
        nodes[i].next = (i < 2) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                nodes[i].array[j][k] = i * 16 + j * 4 + k;
    }
    
    volatile int loop_counter = 100;
    volatile int result = 0;
    
    /* Requirement 5: Loop with combined operations */
    while (loop_counter-- > 0) {
        /* Requirement 1: Bitfield assignments (ZERO_EXTRACT) */
        bf.field1 = (loop_counter & 0x7);
        bf.field2 = ((loop_counter >> 3) & 0x1F);
        bf.field3 = ((loop_counter >> 8) & 0xFF);
        bf.field4 = ((loop_counter * 3) & 0xFFFF);
        
        /* Requirement 2: STRICT_LOW_PART through union */
        pun.half[0] = loop_counter & 0xFFFF;
        pun.byte[2] = (loop_counter >> 16) & 0xFF;
        
        /* Requirement 2: Alternative STRICT_LOW_PART via pointer */
        volatile uint32_t *int_ptr = &pun.full;
        *((volatile uint16_t *)int_ptr + 1) = loop_counter & 0xFFFF;
        
        /* Requirement 3: SUBREG operations with register variable */
        reg_var = pun.full + loop_counter;
        volatile uint8_t low_byte = reg_var & 0xFF;  /* SUBREG extraction */
        
        /* Requirement 3: Vector SUBREG operations */
        vec[0] = reg_var;
        volatile int vec_element = vec[1];  /* SUBREG extraction from vector */
        
        /* Requirement 4: Complex memory addressing (MEM) */
        int idx1 = (loop_counter & 0x3);
        int idx2 = ((loop_counter >> 2) & 0x3);
        
        /* Multi-level pointer dereferencing */
        int mem_val = nodes->next->array[idx1][idx2];
        nodes->next->next->array[idx2][idx1] = mem_val + vec_element;
        
        /* More complex addressing */
        int *ptr = &nodes[(loop_counter & 0x1)].array[(loop_counter >> 1) & 0x3][(loop_counter >> 3) & 0x3];
        *ptr = (*ptr + low_byte) & 0xFF;
        
        /* Requirement 6: Inline assembly influencing RTL */
        __asm__ volatile (
            "movl %[input], %%eax\n\t"
            "movb %%al, %[output]\n\t"
            : [output] "=m" (low_byte)
            : [input] "r" (reg_var)
            : "eax", "memory"
        );
        
        /* Combine results */
        result += bf.field1 + bf.field2 + pun.half[0] + vec_element + mem_val;
        
        /* Clobber registers to force reloads */
        if ((loop_counter & 0xF) == 0) {
            clobber_registers();
        }
    }
    
    /* Cleanup and return */
    free(nodes);
    
    /* Prevent dead code elimination */
    return (result & 0xFF) + (bf.field3 & 0xFF) + (pun.byte[0] & 0xFF);
}
