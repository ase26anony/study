/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t a : 3;
    volatile uint32_t b : 5;
    volatile uint32_t c : 8;
    volatile uint32_t d : 16;
} bitfield_struct;

/* 2. Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* 3. Complex memory structure */
typedef struct {
    int data[8];
    struct inner *next;
} inner;

typedef struct {
    inner *first;
    inner *second;
    int matrix[4][4];
} outer;

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    /* Initialize bitfield struct */
    bitfield_struct bf = {0};
    
    /* Initialize union for partial updates */
    split_int si = {.full = 0x12345678};
    
    /* Initialize complex memory structures */
    inner in1, in2;
    outer out;
    
    in1.next = &in2;
    in2.next = &in1;
    out.first = &in1;
    out.second = &in2;
    
    /* Initialize array for complex addressing */
    int ***multi_array = malloc(3 * sizeof(int**));
    for (int i = 0; i < 3; i++) {
        multi_array[i] = malloc(4 * sizeof(int*));
        for (int j = 0; j < 4; j++) {
            multi_array[i][j] = malloc(5 * sizeof(int));
            for (int k = 0; k < 5; k++) {
                multi_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* 5. Register variable for SUBREG operations */
    register int reg_var asm ("eax") = 0xDEADBEEF;
    register short reg_short asm ("bx");
    
    /* 6. Vector variable */
    v4si vec = {1, 2, 3, 4};
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignments */
            bf.a = (i & 0x7);
            bf.c = (i * 3) & 0xFF;
            bf.d = (i << 8) & 0xFFFF;
        } else if (i % 3 == 1) {
            /* STRICT_LOW_PART: Partial register update */
            si.parts.low = i & 0xFFFF;
            si.bytes[1] = (i * 2) & 0xFF;
            
            /* Another STRICT_LOW_PART pattern */
            ((volatile short*)&si.full)[0] = (i * 3) & 0xFFFF;
        } else {
            /* SUBREG: Register variable with truncation */
            reg_short = (reg_var >> 8) & 0xFFFF;
            
            /* Vector element access (triggers SUBREG) */
            int elem = vec[i % 4];
            reg_var = elem * 2;
            
            /* Inline assembly influencing RTL generation */
            asm volatile (
                "movw %w[input], %%bx\n\t"
                "addw $1, %%bx\n\t"
                : [input] "+r" (reg_short)
                :
                : "bx"
            );
        }
        
        /* Complex memory addressing (MEM) */
        int idx1 = i % 3;
        int idx2 = (i * 2) % 4;
        int idx3 = (i * 3) % 5;
        
        /* Multi-level pointer dereferencing */
        int mem_val = multi_array[idx1][idx2][idx3];
        
        /* Structure pointer chain */
        out.first->data[i % 8] = mem_val;
        out.matrix[idx1][idx2] = out.second->next->data[idx3];
        
        /* Combine with bitfield */
        bf.b = mem_val & 0x1F;
        
        /* Update global state to prevent elimination */
        global_counter += (bf.a + si.parts.low + reg_short + mem_val);
        
        /* Additional inline assembly with constraints */
        asm volatile (
            "movl %[val], %%eax\n\t"
            "rorl $8, %%eax\n\t"
            : 
            : [val] "r" (global_counter)
            : "eax", "cc"
        );
    }
    
    /* Switch statement for additional control flow */
    switch (global_counter & 0x7) {
        case 0:
            bf.a = 1;
            break;
        case 1:
            si.parts.high = 0xAAAA;
            break;
        case 2:
            reg_var = 0x1234;
            break;
        default:
            multi_array[0][0][0] = global_counter;
            break;
    }
    
    /* Final computation using all variables */
    global_result = 
        bf.a + bf.b * 2 + bf.c * 3 + bf.d +
        si.full +
        reg_var +
        multi_array[1][2][3] +
        out.matrix[0][0];
    
    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            free(multi_array[i][j]);
        }
        free(multi_array[i]);
    }
    free(multi_array);
    
    return global_result & 0xFF;
}
