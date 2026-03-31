/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* For STRICT_LOW_PART and ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 9;
    volatile unsigned int field4 : 11;
} bitfield_struct;

/* For complex MEM addressing */
typedef struct {
    int data[8];
    struct inner *next;
} inner;

typedef struct {
    inner *first;
    inner *second;
    int matrix[4][4];
} outer;

/* For SUBREG operations */
#ifdef __GNUC__
register int reg_var asm("eax");
register short reg_short asm("si");
#endif

/* Vector type for SUBREG */
typedef int v4si __attribute__((vector_size(16)));

/* Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for direct RTL influence */
void inline_asm_ops(volatile int *ptr, volatile short *sptr) {
    /* Force SUBREG and register operations */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movw %%ax, %0\n\t"
        : "=m"(*sptr)
        : "r"(*ptr)
        : "%eax", "memory"
    );
    
    /* Complex addressing mode suggestion */
    asm volatile (
        "addl $1, (%1, %2, 4)\n\t"
        : 
        : "r"(ptr), "r"(global_counter)
        : "memory"
    );
}

int main() {
    /* Initialize bitfield struct for ZERO_EXTRACT */
    volatile bitfield_struct bf = {0};
    
    /* Initialize union for STRICT_LOW_PART */
    volatile split_int split = {0};
    
    /* Initialize complex memory structures */
    outer *complex_ptr = malloc(sizeof(outer));
    complex_ptr->first = malloc(sizeof(inner));
    complex_ptr->second = malloc(sizeof(inner));
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            complex_ptr->matrix[i][j] = i * 4 + j;
        }
    }
    
    /* Initialize vector for SUBREG operations */
    volatile v4si vec = {1, 2, 3, 4};
    
    /* Main loop with combined operations */
    for (volatile int i = 0; i < 100; i++) {
        /* Conditional context to affect scheduling */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignment with non-byte-aligned field */
            bf.field2 = (i * 7) & 0x7F;  /* 7-bit field */
            bf.field3 = (i * 13) & 0x1FF; /* 9-bit field */
            
            /* Additional bitfield operation to ensure ZERO_EXTRACT usage */
            bf.field1 = (bf.field2 ^ bf.field3) & 0x1F;
        }
        
        if (i % 5 == 0) {
            /* STRICT_LOW_PART: Partial register update through union */
            split.parts.low = (i * 17) & 0xFFFF;
            
            /* Alternative STRICT_LOW_PART via pointer cast */
            volatile uint32_t *int_ptr = &split.full;
            volatile uint16_t *short_ptr = (volatile uint16_t*)int_ptr;
            short_ptr[1] = (i * 23) & 0xFFFF;  /* High part */
        }
        
        if (i % 7 == 0) {
            /* SUBREG: Vector element extraction */
            volatile int elem = vec[i % 4];
            
            /* Register variable operations for SUBREG */
            #ifdef __GNUC__
            reg_var = elem * 3;
            reg_short = (reg_var >> 8) & 0xFF;
            #endif
            
            /* More SUBREG: Truncation operation */
            volatile short truncated = (volatile short)(elem & 0xFFFF);
            
            /* Use inline assembly with register constraints */
            inline_asm_ops(&elem, &truncated);
        }
        
        /* Complex MEM addressing modes */
        if (i % 11 == 0) {
            /* Multi-level pointer dereferencing */
            complex_ptr->first->data[i % 8] = 
                complex_ptr->second->data[(i + 3) % 8] +
                complex_ptr->matrix[i % 4][(i + 1) % 4];
            
            /* Array indexing with non-constant expression */
            volatile int *chain_ptr = 
                &complex_ptr->matrix[0][0] + 
                complex_ptr->first->data[i % 8];
            
            *chain_ptr = *chain_ptr + i;
            
            /* Structure pointer chain */
            if (complex_ptr->first->next) {
                complex_ptr->first->next->data[0] = i;
            }
        }
        
        /* Switch statement to create more complex control flow */
        switch (i % 13) {
            case 0:
                /* More bitfield operations */
                bf.field4 = (bf.field1 | bf.field2) & 0x7FF;
                break;
            case 1:
                /* More partial updates */
                split.bytes[2] = (i >> 8) & 0xFF;
                break;
            case 2:
                /* More complex memory access */
                volatile int **double_ptr = (volatile int**)&complex_ptr->first->data;
                **double_ptr = **double_ptr + 1;
                break;
            default:
                /* Mixed operations */
                bf.field1 = (i * 29) & 0x1F;
                split.parts.high = (i * 31) & 0xFFFF;
                break;
        }
        
        /* Update global counter to affect loop exit */
        global_counter += (bf.field1 + split.parts.low) & 0xFF;
    }
    
    /* Combine results from all operations to prevent dead code elimination */
    global_result = 
        bf.field1 + bf.field2 * 256 + bf.field3 * 65536 +
        split.full +
        complex_ptr->matrix[0][0] +
        complex_ptr->first->data[0] +
        #ifdef __GNUC__
        reg_var +
        #endif
        vec[0];
    
    /* Cleanup */
    free(complex_ptr->first);
    free(complex_ptr->second);
    free(complex_ptr);
    
    return global_result & 0xFF;
}
