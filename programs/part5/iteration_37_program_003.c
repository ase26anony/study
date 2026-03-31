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

/* 3. Complex nested structure for memory addressing */
typedef struct node {
    volatile int value;
    volatile struct node *next;
    volatile int array[3][2];
} node_t;

/* Global variables to prevent over-optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
static inline void manipulate_registers(volatile uint32_t *val) {
    register uint32_t reg_eax asm("eax") = *val;
    register uint16_t reg_ax asm("ax");
    register uint8_t reg_al asm("al");
    
    /* Inline assembly that forces SUBREG usage */
    asm volatile (
        "movl %1, %0\n\t"
        "movw %%ax, %w0\n\t"
        "movb %%al, %b0\n\t"
        : "+r" (reg_eax)
        : "r" (reg_eax)
        : "cc"
    );
    
    /* More assembly with constraints */
    asm volatile (
        "addl $1, %0\n\t"
        : "+h" (reg_ax)  /* 'h' constraint for high byte */
        :
        : "cc"
    );
    
    *val = reg_eax;
}

int main(void) {
    /* Initialize variables */
    volatile bitfield_t bf = {0};
    volatile split_int_t split = {.full = 0x12345678};
    volatile node_t *nodes = malloc(sizeof(node_t) * 4);
    volatile int multi_array[4][3][2] = {{{0}}};
    
    /* Register variable for SUBREG */
    register int reg_var asm("ebx") = 0xABCDEF01;
    
    /* Initialize node chain */
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = &nodes[i + 1];
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 2; k++) {
                nodes[i].array[j][k] = i + j + k;
            }
        }
    }
    nodes[3].next = NULL;
    
    /* Main loop targeting all RTL patterns */
    for (volatile int i = 0; i < 100; i++) {
        global_counter++;
        
        /* 1. ZERO_EXTRACT patterns - bitfield assignments */
        if (i & 1) {
            bf.low = (i & 0x1F);          /* 5-bit field */
            bf.middle = (i >> 5) & 0x7FF; /* 11-bit field */
            bf.high = (i << 3) & 0xFFFF;  /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART patterns - partial register updates */
        switch (i % 4) {
            case 0:
                split.parts.low = i * 2;      /* Update low 16 bits */
                break;
            case 1:
                split.bytes[1] = i & 0xFF;    /* Update single byte */
                break;
            case 2:
                ((volatile uint16_t*)&split.full)[0] = i * 3; /* Type punning */
                break;
            case 3:
                split.parts.high = (i >> 8) & 0xFFFF;
                break;
        }
        
        /* 3. SUBREG patterns - register variable operations */
        reg_var += i;
        volatile uint16_t short_part = (reg_var >> 8) & 0xFFFF;
        volatile uint8_t byte_part = reg_var & 0xFF;
        
        /* Mix with inline assembly */
        manipulate_registers(&split.full);
        
        /* 4. Complex MEM patterns - nested pointer dereferencing */
        volatile node_t *current = nodes;
        int depth = 0;
        while (current && depth < 3) {
            /* Complex addressing: current->array[global_counter % 3][depth % 2] */
            volatile int *elem = &current->array[global_counter % 3][depth % 2];
            *elem += i + depth;
            
            /* Even more complex: multi-level array with variable indices */
            multi_array[depth][(i + depth) % 3][global_counter % 2] = 
                nodes[depth].array[(i * depth) % 3][(global_counter + 1) % 2];
            
            current = current->next;
            depth++;
        }
        
        /* Additional pointer chain for MEM */
        volatile int ***triple_ptr = (volatile int***)&multi_array;
        volatile int val = *(**triple_ptr + (i % 2));
        
        /* Conditional that uses all results */
        if ((bf.low + split.parts.low + reg_var + val) > 1000) {
            global_result += i;
        }
        
        /* Prevent loop unrolling */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute final result using all variables */
    int result = bf.low + bf.middle + bf.high +
                 split.full + reg_var + global_result +
                 nodes[0].array[0][0] + multi_array[0][0][0];
    
    free((void*)nodes);
    
    /* Return non-deterministic result to prevent optimization */
    return result % 256;
}
