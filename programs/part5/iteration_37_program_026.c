/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int low : 5;
    volatile unsigned int middle : 11;
    volatile unsigned int high : 16;
} bitfield_t;

/* 2. Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int_t;

/* 3. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* 4. Complex structure for memory addressing */
typedef struct node {
    volatile int value;
    volatile struct node* next;
    volatile int array[3][4];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
static inline void manipulate_registers(volatile uint32_t* val) {
    register uint32_t reg_eax asm("eax") = *val;
    register uint16_t reg_ax asm("ax");
    register uint8_t reg_al asm("al");
    
    /* Inline assembly that forces SUBREG usage */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=r" (reg_ax)
        : "r" (reg_eax)
        : "%eax"
    );
    
    /* More assembly with byte operations */
    asm volatile (
        "movb %%al, %0\n\t"
        : "=r" (reg_al)
        : 
        : 
    );
    
    *val = reg_eax;
}

int main(void) {
    /* Initialize variables */
    volatile bitfield_t bf = {0};
    volatile split_int_t split = {.full = 0x12345678};
    volatile v4si vec = {1, 2, 3, 4};
    volatile node_t* nodes = (node_t*)malloc(3 * sizeof(node_t));
    
    /* Initialize node structure */
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 2) ? &nodes[i + 1] : NULL;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                nodes[i].array[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* 5. Loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context */
        if (i % 3 == 0) {
            /* 1. Bitfield assignment - should generate ZERO_EXTRACT */
            bf.middle = (i * 7) & 0x7FF;  /* 11-bit field */
            bf.low = (i * 3) & 0x1F;      /* 5-bit field */
            bf.high = (i * 11) & 0xFFFF;  /* 16-bit field */
        } else if (i % 3 == 1) {
            /* 2. STRICT_LOW_PART through union/pointer */
            split.parts.low = (i * 13) & 0xFFFF;  /* Update low 16 bits */
            
            /* Alternative: pointer cast for STRICT_LOW_PART */
            *((volatile uint16_t*)&split.full) = (i * 17) & 0xFFFF;
        } else {
            /* 3. SUBREG operations with vector */
            volatile int element = vec[i % 4];  /* Extract element */
            vec[i % 4] = element * 2;           /* Store back */
            
            /* Register variable operations */
            volatile uint32_t temp = split.full;
            manipulate_registers(&temp);
            split.full = temp;
        }
        
        /* 4. Complex memory addressing - multi-level with indexing */
        volatile int* addr = &nodes[i % 3].next->array[(i * 2) % 3][(i * 3) % 4];
        *addr = *addr + i;
        
        /* More complex addressing with pointer arithmetic */
        volatile int val = nodes[0].array[1][2] + 
                          nodes[1].array[2][3] * 
                          nodes[2].array[0][1];
        
        /* Switch statement for additional control flow */
        switch (i % 4) {
            case 0:
                /* Additional bitfield operation */
                bf.low = (bf.low ^ i) & 0x1F;
                break;
            case 1:
                /* Byte access through union */
                split.bytes[1] = i & 0xFF;
                break;
            case 2:
                /* Vector operation */
                vec = vec + (v4si){1, 1, 1, 1};
                break;
            case 3:
                /* Complex memory chain */
                volatile int** ptr_ptr = (volatile int**)&nodes[i % 3].next;
                if (*ptr_ptr) {
                    **ptr_ptr = **ptr_ptr + 1;
                }
                break;
        }
        
        /* Update global counter to prevent dead code elimination */
        global_counter += bf.middle + split.parts.low + vec[0] + *addr;
    }
    
    /* 6. Compute final result from all operations */
    global_result = bf.low + bf.middle + bf.high;
    global_result += split.full;
    global_result += vec[0] + vec[1] + vec[2] + vec[3];
    
    /* Complex memory access in result computation */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                global_result += nodes[i].array[j][k];
            }
        }
    }
    
    free((void*)nodes);
    
    /* Return non-zero to ensure all code paths matter */
    return (global_result > 0) ? 0 : 1;
}
