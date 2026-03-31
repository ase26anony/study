/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o test_program test.c */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 32;
} bitfield_struct;

/* For SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* For complex memory addressing */
typedef struct node {
    int data;
    struct node *next;
    int array[4][4];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function to create complex memory addressing */
int complex_memory_access(node_t **ptr_chain, int i, int j) {
    /* Creates MEM with complex addressing: ptr_chain[0]->next->array[i][j] */
    return ptr_chain[0]->next->array[i][j];
}

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    volatile bitfield_struct bf = {0};
    
    /* 2. Integer for STRICT_LOW_PART operations */
    volatile uint32_t int_var = 0x12345678;
    union {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
    } int_union;
    int_union.full = 0;
    
    /* 3. Register variable for SUBREG operations */
    register int reg_var asm("eax") = 0;
    register short reg_short asm("ax") = 0;
    
    /* 4. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* 5. Complex memory structures */
    node_t node1, node2;
    node_t *ptr_array[3];
    int multi_array[10][10];
    
    /* Initialize structures */
    node1.next = &node2;
    node2.next = NULL;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node1.array[i][j] = i * 4 + j;
            node2.array[i][j] = 100 + i * 4 + j;
        }
    }
    ptr_array[0] = &node1;
    
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            multi_array[i][j] = i * 10 + j;
        }
    }
    
    /* Main loop combining all operations */
    for (int i = 0; i < 100; i++) {
        /* Conditional to create branching */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignments */
            bf.field1 = (i & 0x7);           /* 3-bit field */
            bf.field2 = ((i >> 3) & 0x1F);   /* 5-bit field */
            bf.field3 = ((i * 7) & 0xFF);    /* 8-bit field */
            bf.field4 = ((i * 13) & 0xFFFF); /* 16-bit field */
        } else if (i % 3 == 1) {
            /* STRICT_LOW_PART: Partial register updates */
            /* Update low 16 bits */
            ((volatile uint16_t*)&int_var)[0] = (uint16_t)(i * 11);
            
            /* Using union for type-punning */
            int_union.parts.low = (uint16_t)(i * 17);
            int_union.parts.high = (uint16_t)(i * 19);
            
            /* Another STRICT_LOW_PART pattern */
            *((volatile char*)&int_var + 1) = (char)(i & 0xFF);
        } else {
            /* SUBREG operations */
            /* Register variable with smaller type operation */
            reg_short = (short)(reg_var + i);
            reg_var = reg_short * 2;
            
            /* Vector operations that may generate SUBREG */
            vec = vec + vec2;
            /* Extract element - may generate SUBREG */
            int element = ((int*)&vec)[i % 4];
            reg_var += element;
        }
        
        /* Complex memory addressing - creates MEM expressions */
        int mem_val = 0;
        if (i % 5 == 0) {
            /* Multi-level pointer dereferencing */
            mem_val = ptr_array[0]->next->array[i % 4][(i / 4) % 4];
        } else if (i % 5 == 1) {
            /* Array indexing with non-constant expressions */
            mem_val = multi_array[i % 10][(i * 3) % 10];
        } else if (i % 5 == 2) {
            /* Pointer arithmetic chain */
            mem_val = *(*(&multi_array[i % 10] + (i % 5)) + (i % 3));
        } else if (i % 5 == 3) {
            /* Structure pointer chain with computation */
            mem_val = complex_memory_access(ptr_array, i % 4, (i * 2) % 4);
        } else {
            /* Multi-dimensional with pointer arithmetic */
            int (*array_ptr)[10] = &multi_array[i % 9];
            mem_val = (*array_ptr)[(i * 7) % 10];
        }
        
        /* Inline assembly to influence RTL generation */
        asm volatile (
            /* Suggest subregister use */
            "movw %w[input], %%ax\n\t"
            "addw $1, %%ax\n\t"
            "movw %%ax, %w[output]\n\t"
            /* Clobber registers to force spills/reloads */
            :
            [output] "=r" (reg_short)
            : [input] "r" (reg_short)
            : "ax", "memory"
        );
        
        /* Another inline assembly with memory constraint */
        int temp = mem_val;
        asm volatile (
            "addl %1, %0\n\t"
            : "+Q" (temp)  /* Memory constraint */
            : "r" (i)
            : "cc"
        );
        mem_val = temp;
        
        /* Combine results to prevent dead code elimination */
        global_counter += bf.field1 + bf.field2 + int_union.parts.low + reg_var + mem_val;
        
        /* Additional STRICT_LOW_PART in loop */
        if (i % 7 == 0) {
            volatile short *short_ptr = (volatile short*)&int_var;
            short_ptr[0] = (short)(short_ptr[0] + 1);  /* Update low part */
        }
        
        /* Additional ZERO_EXTRACT with conditional */
        if (i % 11 == 0) {
            bf.field3 = (bf.field3 ^ i) & 0xFF;  /* Ensure 8-bit extract */
        }
    }
    
    /* Final computations using all variables */
    global_result = 
        (bf.field1 << 0) |
        (bf.field2 << 3) |
        (bf.field3 << 8) |
        (bf.field4 << 16) |
        (int_var & 0xFF) |
        (reg_var << 24);
    
    /* More complex memory access at the end */
    int final_mem = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            final_mem += ptr_array[0]->next->array[i][j];
        }
    }
    
    /* Return value depends on all operations */
    return (global_counter + global_result + final_mem + reg_var) & 0xFF;
}
