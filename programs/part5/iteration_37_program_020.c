/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} __attribute__((packed));

/* 2. Union for STRICT_LOW_PART */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* 3. Complex memory structure */
struct nested {
    int data[4][4];
    struct nested *next;
};

/* 4. Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
static inline void manipulate_registers(void) {
    /* 6. Inline assembly with register constraints */
    register uint32_t reg_var asm("eax") = global_counter;
    register uint16_t reg_low asm("ax");
    
    asm volatile (
        "movw %%ax, %0\n\t"
        "addl $1, %%eax\n\t"
        : "=m" (global_counter)
        : "a" (reg_var)
        : "cc"
    );
    
    /* SUBREG trigger: operate on part of register */
    reg_low = (uint16_t)(reg_var & 0xFFFF);
    asm volatile ("" : "+r" (reg_low));
}

int main(void) {
    struct bitfield_struct bf = {0};
    union split_int split = {0};
    v4si vec = {1, 2, 3, 4};
    int *dynamic_array[8];
    
    /* Initialize complex memory structure */
    struct nested *nested_ptr = malloc(sizeof(struct nested));
    struct nested *current = nested_ptr;
    
    for (int i = 0; i < 3; i++) {
        current->next = malloc(sizeof(struct nested));
        current = current->next;
    }
    current->next = NULL;
    
    /* Initialize dynamic array */
    for (int i = 0; i < 8; i++) {
        dynamic_array[i] = malloc(16 * sizeof(int));
    }
    
    /* Main loop with combined operations */
    for (int i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT triggers */
        bf.field1 = (i & 0x7);           /* 3-bit field */
        bf.field3 = (i * 3) & 0xFF;      /* 8-bit field */
        bf.field4 = (i << 8) & 0xFFFF;   /* 16-bit field */
        
        /* 2. STRICT_LOW_PART triggers */
        split.parts.low = i & 0xFFFF;
        split.parts.high = (i >> 16) & 0xFFFF;
        
        /* Update via pointer cast (another STRICT_LOW_PART pattern) */
        volatile uint16_t *short_ptr = (volatile uint16_t*)&split.full;
        short_ptr[0] = (i * 2) & 0xFFFF;
        
        /* 3. SUBREG triggers with vector operations */
        int element = vec[i % 4];  /* Vector element extraction */
        vec[i % 4] = element + 1;  /* Vector element update */
        
        /* Register variable manipulation */
        manipulate_registers();
        
        /* 4. Complex memory addressing */
        current = nested_ptr;
        int level = 0;
        while (current != NULL) {
            /* Multi-level pointer dereference with array indexing */
            current->data[i % 4][(i + level) % 4] = 
                dynamic_array[level][i % 16] + bf.field3;
            
            /* Chain of memory accesses */
            int temp = current->data[(i + 1) % 4][level % 4];
            dynamic_array[(level + 1) % 8][(i + temp) % 16] = 
                temp * split.parts.low;
            
            current = current->next;
            level++;
        }
        
        /* Conditional context with switch */
        switch (i % 5) {
            case 0:
                bf.field2 = (i * 5) & 0x1F;
                break;
            case 1:
                split.full = split.full ^ 0xAAAAAAAA;
                break;
            case 2:
                /* More complex memory addressing */
                int ***triple_ptr = (int***)&dynamic_array;
                (*triple_ptr)[i % 8][0] = i;
                break;
            case 3:
                /* Nested array access */
                for (int j = 0; j < 4; j++) {
                    nested_ptr->data[j][j] += 
                        dynamic_array[j][i % 16] * j;
                }
                break;
            case 4:
                /* Mixed operations */
                global_result += bf.field1 + split.parts.low + 
                                vec[i % 4] + dynamic_array[0][i % 16];
                break;
        }
        
        /* Loop exit condition based on operations */
        if (global_result > 1000000) {
            break;
        }
    }
    
    /* Cleanup */
    current = nested_ptr;
    while (current != NULL) {
        struct nested *next = current->next;
        free(current);
        current = next;
    }
    
    for (int i = 0; i < 8; i++) {
        free(dynamic_array[i]);
    }
    
    /* Return value based on all operations to prevent elimination */
    return (bf.field1 + split.parts.low + vec[0] + global_result) & 0xFF;
}
