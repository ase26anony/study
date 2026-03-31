/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* For vector extensions */
typedef int v4si __attribute__((vector_size(16)));

/* Struct with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 9;
    volatile unsigned int field4 : 11;
    volatile unsigned int padding : 10;
};

/* Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* Complex memory structure */
struct level3 {
    volatile int data[4][4];
};

struct level2 {
    volatile struct level3 *sub[2];
};

struct level1 {
    volatile struct level2 *chain;
    volatile int index;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

int main(void) {
    /* 1. Bitfield operations for ZERO_EXTRACT */
    struct bitfield_struct bf = {0};
    volatile struct bitfield_struct *bf_ptr = &bf;
    
    /* 2. Union for STRICT_LOW_PART */
    union split_int split = {0};
    volatile union split_int *split_ptr = &split;
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    volatile register int *reg_ptr = &reg_var;
    
    /* 4. Complex memory structures */
    struct level3 l3 = {{{0}}};
    struct level2 l2 = {&l3, &l3};
    struct level1 l1 = {&l2, 0};
    volatile struct level1 *mem_ptr = &l1;
    
    /* 5. Vector for SUBREG operations */
    v4si vec = {1, 2, 3, 4};
    volatile v4si *vec_ptr = &vec;
    
    /* Initialize array for complex addressing */
    volatile int multi_array[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    volatile int *array_ptr = &multi_array[0][0][0];
    
    /* Main loop with combined operations */
    for (int i = 0; i < 100; i++) {
        /* Conditional to create branching */
        if (i % 3 == 0) {
            /* 1. ZERO_EXTRACT: Bitfield assignments */
            bf_ptr->field1 = (i * 7) & 0x1F;
            bf_ptr->field3 = (i * 13) & 0x1FF;
            
            /* Complex bitfield expression */
            bf_ptr->field2 = ((bf_ptr->field1 << 2) | (bf_ptr->field3 & 0x3F)) & 0x7F;
        }
        else if (i % 3 == 1) {
            /* 2. STRICT_LOW_PART: Partial register update */
            /* Update low 16 bits only */
            split_ptr->parts.low = i * 17;
            
            /* Alternative STRICT_LOW_PART via pointer cast */
            *((volatile uint16_t*)&split.full) = i * 23;
            
            /* Update high 16 bits separately */
            split_ptr->parts.high = i * 29;
        }
        else {
            /* 3. SUBREG: Register variable operations */
            /* Force truncation to smaller type */
            volatile char char_part = (char)(reg_var + i);
            reg_var = reg_var * 3 + char_part;
            
            /* Vector element access (triggers SUBREG) */
            volatile int elem = vec_ptr[0][i % 4];
            vec_ptr[0][(i + 1) % 4] = elem * 2;
            
            /* 6. Inline assembly with register constraints */
            asm volatile (
                "movl %1, %%eax\n\t"
                "addl $1, %%eax\n\t"
                "movb %%al, %0\n\t"
                : "=m" (char_part)
                : "r" (reg_var)
                : "%eax", "cc"
            );
        }
        
        /* 4. Complex memory addressing (MEM) */
        /* Multi-level pointer dereferencing */
        int idx1 = (i * 11) % 8;
        int idx2 = (i * 13) % 8;
        int idx3 = (i * 17) % 8;
        
        /* Complex addressing mode */
        mem_ptr->chain->sub[idx1 % 2]->data[idx2][idx3] = 
            multi_array[idx1][idx2][idx3] + reg_var;
        
        /* Even more complex addressing */
        array_ptr[idx1 * 64 + idx2 * 8 + idx3] = 
            mem_ptr->chain->sub[(idx1 + 1) % 2]->data[idx3][idx2];
        
        /* Pointer arithmetic chain */
        volatile int ***triple_ptr = (volatile int***)multi_array;
        triple_ptr[idx1][idx2][idx3] = 
            triple_ptr[idx3][idx2][idx1] + split.full;
        
        /* Switch for additional control flow */
        switch (i % 5) {
            case 0:
                bf_ptr->field4 = (bf_ptr->field3 << 2) | (split.parts.low & 0x3);
                break;
            case 1:
                split.parts.high = bf_ptr->field2 * 3;
                break;
            case 2:
                reg_var = mem_ptr->chain->sub[0]->data[i % 4][(i + 1) % 4];
                break;
            case 3:
                /* Nested memory access */
                mem_ptr->chain->sub[1]->data[(i + 2) % 4][(i + 3) % 4] =
                    mem_ptr->chain->sub[0]->data[(i + 1) % 4][i % 4];
                break;
            case 4:
                /* Combined operation */
                vec_ptr[0][i % 4] = 
                    (bf_ptr->field1 << 16) | split.parts.low;
                break;
        }
        
        /* Loop exit condition based on all operations */
        global_counter += bf.field1 + split.parts.low + (reg_var & 0xFF) + 
                         mem_ptr->chain->sub[0]->data[0][0];
    }
    
    /* Final computation to prevent dead code elimination */
    global_result = 
        bf.field1 * 1000000 +
        bf.field2 * 10000 +
        bf.field3 * 100 +
        bf.field4 +
        split.full +
        reg_var +
        mem_ptr->chain->sub[0]->data[0][0] +
        mem_ptr->chain->sub[1]->data[0][0] +
        vec_ptr[0][0] +
        vec_ptr[0][1] +
        vec_ptr[0][2] +
        vec_ptr[0][3] +
        multi_array[0][0][0];
    
    /* Additional inline assembly to force specific RTL patterns */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movb %%ah, %0\n\t"
        : "=m" (global_counter)
        : "m" (split.parts.low)
        : "%ax", "cc"
    );
    
    /* Return value based on all computations */
    return (global_result + global_counter) & 0xFF;
}
