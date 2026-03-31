/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int low : 3;    /* Not byte-aligned */
    volatile unsigned int mid : 11;   /* Not byte-aligned */
    volatile unsigned int high : 18;  /* Not byte-aligned */
} bitfield_struct;

/* 2. Union for STRICT_LOW_PART operations */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* 3. Complex memory structure for MEM addressing */
typedef struct node {
    volatile int val;
    volatile struct node* next;
    volatile int array[3][4];
} mem_node;

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for direct RTL influence */
static inline void asm_clobber_registers(void) {
    /* Clobber specific registers to force reloads */
    asm volatile (
        "movl %%eax, %%ebx\n\t"
        "movl %%ecx, %%edx\n\t"
        : : : "eax", "ebx", "ecx", "edx", "memory"
    );
}

/* Function that uses register variables for SUBREG */
static int use_register_vars(int input) {
    /* Explicit register variables */
    register int reg_a asm("eax") = input;
    register short reg_b asm("bx");
    
    /* Operations that require truncation (SUBREG) */
    reg_b = (short)(reg_a & 0xFFFF);
    
    /* Mix with volatile operations */
    volatile int temp = reg_b * 2;
    
    return temp + (reg_a >> 16);
}

int main(void) {
    /* Initialize variables */
    bitfield_struct bf = {0};
    split_int split = {.full = 0x12345678};
    v4si vec = {1, 2, 3, 4};
    
    /* Allocate and initialize memory structure */
    mem_node* node = (mem_node*)malloc(sizeof(mem_node));
    if (!node) return -1;
    
    node->val = 100;
    node->next = node;  /* Self-reference for complexity */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            node->array[i][j] = i * 10 + j;
        }
    }
    
    /* Multi-dimensional array with pointer chains */
    volatile int*** complex_ptr = (volatile int***)malloc(5 * sizeof(int**));
    for (int i = 0; i < 5; i++) {
        complex_ptr[i] = (volatile int**)malloc(4 * sizeof(int*));
        for (int j = 0; j < 4; j++) {
            complex_ptr[i][j] = (volatile int*)malloc(3 * sizeof(int));
            for (int k = 0; k < 3; k++) {
                complex_ptr[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context */
        if (i % 3 == 0) {
            /* 1. Bitfield assignments (ZERO_EXTRACT) */
            bf.low = (i & 0x7);           /* 3-bit field */
            bf.mid = ((i * 3) & 0x7FF);   /* 11-bit field */
            bf.high = ((i * 7) & 0x3FFFF); /* 18-bit field */
            
            global_counter += bf.low + bf.mid;
        } else if (i % 3 == 1) {
            /* 2. STRICT_LOW_PART assignments */
            split.parts.low = (uint16_t)(i * 5);
            split.bytes[1] = (uint8_t)(i + 10);
            
            /* Additional type-punning */
            *((volatile short*)&split.full + 1) = (short)(i * 3);
        } else {
            /* 3. SUBREG operations with register variables */
            int reg_result = use_register_vars(i);
            global_result ^= reg_result;
            
            /* Vector operations that may generate SUBREG */
            vec[0] = vec[1] + vec[2];
            volatile int elem = vec[0];
            global_counter += elem;
        }
        
        /* 4. Complex memory addressing (MEM) */
        /* Multi-level pointer dereferencing */
        int mem_val = complex_ptr[i % 5][(i / 5) % 4][i % 3];
        node->array[i % 3][(i + 1) % 4] = mem_val;
        
        /* Structure pointer chain */
        volatile int chain_val = node->next->array[(i + 2) % 3][(i + 3) % 4];
        node->val += chain_val;
        
        /* 5. Inline assembly to influence RTL */
        asm_clobber_registers();
        
        /* Additional memory operation with addressing mode */
        volatile int* ptr = &node->array[0][0];
        ptr[(i * 7) % 12] = ptr[(i * 3) % 12] + 1;
    }
    
    /* Switch statement for additional control flow */
    volatile int switch_var = global_counter % 7;
    switch (switch_var) {
        case 0:
            bf.low = 1;
            break;
        case 1:
            split.parts.high = 0xABCD;
            break;
        case 2:
            node->array[0][0] = use_register_vars(switch_var);
            break;
        case 3:
            complex_ptr[0][0][0] = split.full;
            break;
        default:
            global_result += node->val;
            break;
    }
    
    /* Combine all results to prevent dead code elimination */
    int final_result = 
        (int)bf.low + 
        (int)bf.mid + 
        (int)bf.high + 
        split.full + 
        node->val + 
        vec[0] + 
        complex_ptr[0][0][0] + 
        global_counter + 
        global_result;
    
    /* Cleanup */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            free((void*)complex_ptr[i][j]);
        }
        free((void*)complex_ptr[i]);
    }
    free(complex_ptr);
    free(node);
    
    return final_result % 256;
}
