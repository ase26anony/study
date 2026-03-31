/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t field1 : 5;
    volatile uint32_t field2 : 11;
    volatile uint32_t field3 : 7;
    volatile uint32_t field4 : 9;
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

/* 3. Complex nested structure for memory addressing */
typedef struct node {
    volatile int value;
    volatile struct node* next;
    volatile int array[3][4];
} node_t;

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
void manipulate_with_asm(volatile int* val) {
    register int temp asm("eax") = *val;
    
    /* Inline assembly that suggests subregister use */
    asm volatile (
        "movw %%ax, %0\n\t"
        "rorl $16, %%eax\n\t"
        "movw %%ax, %1\n\t"
        : "=m" (((volatile uint16_t*)val)[0])
        , "=m" (((volatile uint16_t*)val)[1])
        : "r" (temp)
        : "eax", "memory"
    );
}

int main() {
    /* Initialize variables */
    bitfield_struct bf = {0};
    split_int split = {0};
    node_t* nodes = malloc(3 * sizeof(node_t));
    v4si vector = {1, 2, 3, 4};
    volatile int* mem_ptr;
    
    /* Initialize node chain */
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 2) ? &nodes[i+1] : NULL;
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                nodes[i].array[j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        if (i & 1) {
            bf.field1 = (i & 0x1F);           /* 5-bit field */
            bf.field3 = ((i * 3) & 0x7F);     /* 7-bit field */
        } else {
            bf.field2 = ((i * 5) & 0x7FF);    /* 11-bit field */
            bf.field4 = ((i * 7) & 0x1FF);    /* 9-bit field */
        }
        
        /* 2. STRICT_LOW_PART operations */
        switch (i % 4) {
            case 0:
                split.parts.low = i & 0xFFFF;         /* Low 16-bit update */
                break;
            case 1:
                split.parts.high = (i >> 16) & 0xFFFF; /* High 16-bit update */
                break;
            case 2:
                split.bytes[1] = (i * 2) & 0xFF;      /* Single byte update */
                break;
            case 3:
                ((volatile uint16_t*)&split.full)[0] = i & 0xFFFF; /* Pointer cast update */
                break;
        }
        
        /* 3. SUBREG operations with vector and register variables */
        {
            register int vec_elem asm("ebx") = vector[i % 4];
            volatile int temp = vec_elem;
            
            /* Force truncation through smaller type operation */
            volatile short truncated = (short)(temp * 3);
            split.parts.low += truncated;
            
            /* Vector element update causing SUBREG */
            vector[i % 4] = temp + 1;
        }
        
        /* 4. Complex memory addressing (MEM) */
        if (nodes) {
            /* Multi-level pointer dereferencing */
            mem_ptr = &nodes[i % 3].array[(i / 3) % 3][i % 4];
            
            /* Chain of pointer accesses */
            volatile int val1 = nodes[0].array[1][2];
            volatile int val2 = nodes[1].next->array[2][3];
            volatile int val3 = nodes[2].next->next ? 
                               nodes[2].next->next->value : 0;
            
            /* Complex address calculation */
            *mem_ptr = (*mem_ptr + val1 + val2 + val3) & 0xFF;
            
            /* Even more complex addressing */
            nodes[(i + 1) % 3].array[(i * 2) % 3][(i * 3) % 4] = 
                nodes[i % 3].array[(i * 7) % 3][(i * 5) % 4];
        }
        
        /* 5. Inline assembly influencing surrounding code */
        manipulate_with_asm((volatile int*)&split.full);
        
        /* Combine results to prevent dead code elimination */
        global_counter += bf.field1 + bf.field2 + split.parts.low;
        
        /* Conditional branch with complex expression */
        if ((bf.field3 * split.parts.high) > 1000) {
            global_result ^= *mem_ptr;
        }
    }
    
    /* Final computation using all variables */
    int final_result = 
        (bf.field1 << 24) |
        (bf.field2 << 13) |
        (bf.field3 << 6) |
        bf.field4 |
        split.full |
        global_counter |
        global_result;
    
    /* Cleanup */
    free(nodes);
    
    return final_result & 0xFF;
}
