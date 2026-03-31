/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers */

#include <stdint.h>
#include <stdlib.h>

/* Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t field1 : 5;
    volatile uint32_t field2 : 7;
    volatile uint32_t field3 : 9;
    volatile uint32_t field4 : 11;
} bitfield_struct;

/* Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Complex memory structure */
typedef struct {
    int data[8];
    struct inner {
        int matrix[3][3];
        int *ptr;
    } *inner_ptr;
} complex_struct;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for SUBREG */
static inline int subreg_operation(register int x asm("eax")) {
    int result;
    /* Force SUBREG through byte operation */
    asm volatile (
        "movb %%al, %0\n\t"
        : "=r" (result)
        : "a" (x)
        : "cc"
    );
    return result;
}

int main(void) {
    /* Initialize bitfield struct */
    bitfield_struct bf = {0};
    
    /* Initialize split integer */
    split_int si = { .full = 0x12345678 };
    
    /* Initialize complex memory structure */
    complex_struct cs;
    complex_struct *cs_ptr = &cs;
    cs.inner_ptr = malloc(sizeof(*cs.inner_ptr));
    cs.inner_ptr->ptr = malloc(16 * sizeof(int));
    
    /* Register variable for SUBREG */
    register int reg_var asm("ebx") = 0x89ABCDEF;
    
    /* Multi-dimensional array for complex MEM addressing */
    int ***multi_array = malloc(4 * sizeof(int**));
    for (int i = 0; i < 4; i++) {
        multi_array[i] = malloc(5 * sizeof(int*));
        for (int j = 0; j < 5; j++) {
            multi_array[i][j] = malloc(6 * sizeof(int));
        }
    }
    
    /* Loop combining all operations */
    for (volatile int i = 0; i < 100; i++) {
        global_counter++;
        
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        if (i & 1) {
            bf.field1 = (i * 3) & 0x1F;        /* 5-bit field */
            bf.field3 = (i * 7) & 0x1FF;       /* 9-bit field */
        } else {
            bf.field2 = (i * 5) & 0x7F;        /* 7-bit field */
            bf.field4 = (i * 11) & 0x7FF;      /* 11-bit field */
        }
        
        /* 2. STRICT_LOW_PART through union/pointer */
        switch (i % 4) {
            case 0:
                si.parts.low = i * 2;          /* Update low 16 bits */
                break;
            case 1:
                si.bytes[1] = i * 3;           /* Update single byte */
                break;
            case 2:
                *((volatile uint16_t*)&si.full + 1) = i * 5; /* High 16 bits */
                break;
            case 3:
                si.parts.high = i * 7;         /* Update high 16 bits */
                break;
        }
        
        /* 3. SUBREG through register variable operations */
        reg_var = (reg_var * 1103515245 + 12345) & 0x7FFFFFFF;
        int truncated = subreg_operation(reg_var);
        
        /* 4. Complex MEM addressing with pointer chains */
        int idx1 = i % 4;
        int idx2 = (i * 3) % 5;
        int idx3 = (i * 5) % 6;
        
        /* Chain of pointer dereferences */
        cs.inner_ptr->matrix[idx1][idx2] = i;
        cs.inner_ptr->ptr[idx2] = cs.inner_ptr->matrix[idx2][idx1];
        
        /* Multi-level array access */
        multi_array[idx1][idx2][idx3] = 
            cs.inner_ptr->matrix[idx2][idx1] + 
            cs.inner_ptr->ptr[idx3 % 4];
        
        /* 5. Inline assembly with constraints */
        int temp;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movw %%ax, %0\n\t"      /* STRICT_LOW_PART for 16-bit */
            : "=m" (si.parts.low)
            : "r" (si.full), "r" (truncated)
            : "eax", "cc"
        );
        
        /* Combine results */
        global_result ^= bf.field1 | (bf.field3 << 5);
        global_result += si.full;
        global_result ^= multi_array[idx1][idx2][idx3];
        global_result += truncated;
        
        /* Conditional break based on operations */
        if (global_result > 1000000) {
            global_result %= 1000000;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            free(multi_array[i][j]);
        }
        free(multi_array[i]);
    }
    free(multi_array);
    free(cs.inner_ptr->ptr);
    free(cs.inner_ptr);
    
    /* Return value derived from all operations */
    return (global_result + bf.field2 + (bf.field4 << 7) + si.parts.high) & 0xFF;
}
