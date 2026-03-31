/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers program.c -o program */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int f1 : 3;
    volatile unsigned int f2 : 5;
    volatile unsigned int f3 : 11;
    volatile unsigned int f4 : 13;
} bitfield_struct;

/* For SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* For complex MEM addressing */
typedef struct node {
    int data;
    struct node *next;
    int arr[3][3];
} node_t;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function to create complex memory addressing */
int complex_mem_access(node_t **ptr_chain, int i, int j) {
    /* Creates: MEM (PLUS (REG) (CONST)) patterns */
    return ptr_chain[0]->next->arr[i][j] +
           ptr_chain[1]->next->next->arr[j][i];
}

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    bitfield_struct bf = {0};
    volatile bitfield_struct *bf_ptr = &bf;
    
    /* 2. Integer for STRICT_LOW_PART via union */
    union {
        volatile uint32_t full;
        volatile uint16_t half[2];
        volatile uint8_t bytes[4];
    } reg_parts = {0};
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    register short reg_short asm("ax");
    
    /* 4. Complex memory structures */
    node_t *nodes[4];
    for (int i = 0; i < 4; i++) {
        nodes[i] = (node_t*)malloc(sizeof(node_t));
        nodes[i]->next = (i < 3) ? nodes[i+1] : NULL;
        for (int x = 0; x < 3; x++)
            for (int y = 0; y < 3; y++)
                nodes[i]->arr[x][y] = i * 100 + x * 10 + y;
    }
    
    /* 5. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Main loop combining all patterns */
    for (int i = 0; i < 100; i++) {
        /* Exit condition based on operations */
        if (global_counter > 1000) break;
        
        /* Pattern 1: ZERO_EXTRACT via bitfield assignment */
        /* Compiler generates ZERO_EXTRACT for non-byte-aligned fields */
        bf_ptr->f2 = (i & 0x1F);           /* 5-bit field */
        bf_ptr->f3 = (i * 3) & 0x7FF;      /* 11-bit field */
        bf_ptr->f4 = (i * 7) & 0x1FFF;     /* 13-bit field */
        
        /* Pattern 2: STRICT_LOW_PART via partial register update */
        /* Update lower 16 bits independently */
        reg_parts.half[0] = (i & 0xFFFF);
        /* Update single byte within the integer */
        reg_parts.bytes[2] = (i * 2) & 0xFF;
        
        /* Pattern 3: SUBREG via register variable truncation */
        /* Forces SUBREG for 32-bit -> 16-bit conversion */
        reg_short = (reg_var & 0xFFFF) + i;
        /* Inline assembly suggesting subregister use */
        asm volatile (
            "movw %w[input], %[output]"
            : [output] "=r" (reg_short)
            : [input] "r" (reg_var)
            : "cc"
        );
        
        /* Pattern 4: Complex MEM addressing with multiple levels */
        int mem_result = complex_mem_access(nodes, i % 3, (i + 1) % 3);
        
        /* Pattern 5: Vector operations generating SUBREG */
        /* Element access may generate SUBREG */
        int vec_element = ((int*)&vec)[i % 4];
        vec = vec + vec2;  /* Vector operation */
        
        /* Combine results to prevent elimination */
        global_counter += bf_ptr->f2 + bf_ptr->f3;
        global_counter += reg_parts.bytes[0] + reg_parts.bytes[2];
        global_counter += reg_short;
        global_counter += mem_result;
        global_counter += vec_element;
        
        /* Additional inline assembly with constraints */
        /* 'h' constraint for high byte register */
        unsigned char high_byte;
        asm volatile (
            "movb %%ah, %[out]"
            : [out] "=q" (high_byte)
            :
            : "eax"
        );
        global_counter += high_byte;
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" ::: "memory");
    }
    
    /* Final computation using all variables */
    global_result = bf_ptr->f4 
                    + reg_parts.full 
                    + reg_var 
                    + complex_mem_access(nodes, 0, 1)
                    + ((int*)&vec)[0];
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        free(nodes[i]);
    }
    
    return global_result % 256;
}
