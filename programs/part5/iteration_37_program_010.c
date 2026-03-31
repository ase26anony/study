/* Target: resource.cc - mark_referenced_resources function
 * Specifically targets ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM_P handling
 */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t field1 : 3;    /* Not byte-aligned */
    volatile uint32_t field2 : 5;    /* Crosses byte boundary */
    volatile uint32_t field3 : 12;   /* Multiple bytes */
    volatile uint32_t field4 : 7;    /* Odd size */
    volatile uint32_t field5 : 5;    /* Final odd-sized field */
} bitfield_struct_t;

/* 2. Union for STRICT_LOW_PART operations */
typedef union {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int_t;

/* 3. Complex memory structure for MEM operations */
typedef struct {
    volatile int data[8];
    volatile struct {
        volatile int x;
        volatile int y;
        volatile int z;
    } coords;
} nested_t;

typedef struct {
    volatile nested_t* nested;
    volatile int matrix[4][4];
    volatile int* indirect;
} complex_mem_t;

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for direct RTL influence */
static inline void asm_subreg_ops(volatile uint32_t* val) {
    /* Explicit register variable for SUBREG operations */
    register uint32_t reg_eax asm("eax") = *val;
    register uint16_t reg_ax asm("ax");
    register uint8_t reg_al asm("al");
    
    /* Force SUBREG operations through inline assembly */
    asm volatile (
        "movw %%ax, %[low16]\n\t"
        "movb %%al, %[low8]\n\t"
        : [low16] "=m" (reg_ax), [low8] "=m" (reg_al)
        : "a" (reg_eax)
        : "memory"
    );
    
    /* Use the results */
    *val = reg_ax + reg_al;
}

/* Main function with combined operations */
int main(void) {
    /* Initialize all variables */
    volatile bitfield_struct_t bf = {0};
    volatile split_int_t split = {0};
    volatile complex_mem_t cmem;
    volatile nested_t nested_obj;
    volatile int array_3d[3][4][5];
    volatile v4si vec = {1, 2, 3, 4};
    
    /* Allocate and setup complex memory structure */
    cmem.nested = &nested_obj;
    cmem.indirect = (int*)&array_3d[0][0][0];
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 5; k++) {
                array_3d[i][j][k] = (i * 100) + (j * 10) + k;
            }
        }
    }
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context to affect scheduling */
        if (i % 3 == 0) {
            /* 1. ZERO_EXTRACT: Bitfield assignments */
            bf.field1 = (i & 0x7);           /* 3-bit field */
            bf.field2 = ((i >> 3) & 0x1F);   /* 5-bit field crossing boundary */
            bf.field3 = ((i * 7) & 0xFFF);   /* 12-bit field */
            
            /* Combined bitfield operation */
            volatile uint32_t temp = bf.field4;
            bf.field4 = (temp + i) & 0x7F;   /* 7-bit field */
            bf.field5 = (i & 0x1F);          /* 5-bit field */
        }
        
        /* 2. STRICT_LOW_PART: Partial register updates */
        switch (i % 4) {
            case 0:
                /* Update low 16 bits */
                split.parts.low = (i * 3) & 0xFFFF;
                break;
            case 1:
                /* Update high 16 bits */
                split.parts.high = (i * 5) & 0xFFFF;
                break;
            case 2:
                /* Update single byte - likely STRICT_LOW_PART */
                split.bytes[1] = (i * 7) & 0xFF;
                break;
            case 3:
                /* Update via pointer cast */
                *((volatile uint16_t*)&split.full + 1) = (i * 11) & 0xFFFF;
                break;
        }
        
        /* 3. SUBREG: Vector and register operations */
        if (i % 5 == 0) {
            /* Vector element extraction - causes SUBREG */
            volatile int elem = vec[i % 4];
            vec[i % 4] = elem + i;
            
            /* Register variable operations */
            asm_subreg_ops((uint32_t*)&split.full);
        }
        
        /* 4. Complex MEM addressing modes */
        /* Multi-level pointer dereferencing */
        volatile int* ptr1 = &array_3d[0][0][0];
        volatile int** ptr2 = &ptr1;
        volatile int*** ptr3 = &ptr2;
        
        /* Complex addressing with non-constant indices */
        int idx1 = i % 3;
        int idx2 = (i * 2) % 4;
        int idx3 = (i * 3) % 5;
        
        /* Chain of memory accesses */
        cmem.matrix[idx1][idx2] = ***ptr3 + i;
        cmem.nested->coords.x = array_3d[idx1][idx2][idx3];
        
        /* Even more complex addressing */
        volatile int val = cmem.nested->data[(i * 7) % 8] + 
                          cmem.matrix[(i + 1) % 4][(i + 2) % 4];
        
        /* Structure pointer chain */
        cmem.indirect[i % 20] = val;
        
        /* Update global state */
        global_counter += bf.field1 + split.parts.low + elem;
        
        /* Prevent loop unrolling */
        asm volatile ("" : : "r"(i) : "memory");
    }
    
    /* Final computation using all variables */
    global_result = bf.field3 + 
                   split.full + 
                   cmem.matrix[0][0] + 
                   cmem.nested->coords.x + 
                   vec[0] + 
                   global_counter;
    
    /* Return non-zero result */
    return (global_result > 0) ? 0 : 1;
}
