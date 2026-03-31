/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */
#include <stdint.h>
#include <stdlib.h>

/* Requirement 1: Bitfield operations for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int a : 3;
    volatile unsigned int b : 5;
    volatile unsigned int c : 8;
    volatile unsigned int d : 16;
} bitfield_struct;

/* Requirement 2: Union for STRICT_LOW_PART */
typedef union {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
} split_int;

/* Requirement 3: Register variables for SUBREG */
register uint32_t reg_var asm("r12");
register uint16_t reg_short asm("r13");

/* Requirement 4: Complex memory structures */
typedef struct {
    int data[8];
    struct inner *next;
} inner;

typedef struct {
    inner *first;
    inner *second;
    int matrix[4][4];
} outer;

/* Requirement 5: Loop and conditional contexts */
volatile int loop_counter = 100;
volatile int condition_seed = 7;

/* Inline assembly helpers for Requirement 6 */
#define FORCE_SUBREG_ACCESS(var) \
    asm volatile("" : "+r"(var) : : "memory")

#define MEMORY_BARRIER() \
    asm volatile("" : : : "memory")

int main(void) {
    /* Initialize bitfield struct */
    bitfield_struct bf = {0};
    
    /* Initialize split integer */
    split_int si = {0};
    
    /* Initialize register variables */
    reg_var = 0x12345678;
    reg_short = 0;
    
    /* Initialize complex memory structure */
    outer *outer_ptr = (outer*)malloc(sizeof(outer));
    if (!outer_ptr) return -1;
    
    outer_ptr->first = (inner*)malloc(sizeof(inner));
    outer_ptr->second = (inner*)malloc(sizeof(inner));
    
    if (!outer_ptr->first || !outer_ptr->second) {
        free(outer_ptr->first);
        free(outer_ptr->second);
        free(outer_ptr);
        return -1;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            outer_ptr->matrix[i][j] = i * 4 + j;
        }
    }
    
    /* Main loop targeting all requirements */
    int result = 0;
    
    for (int i = 0; i < loop_counter; i++) {
        /* Requirement 5: Conditional context */
        if ((i % condition_seed) == 0) {
            /* Requirement 1: Bitfield assignment (ZERO_EXTRACT) */
            bf.a = (i & 0x7);           /* 3-bit field */
            bf.b = ((i >> 3) & 0x1F);   /* 5-bit field */
            bf.c = ((i >> 8) & 0xFF);   /* 8-bit field */
            bf.d = i * 2;               /* 16-bit field */
            
            /* Combine bitfields to force ZERO_EXTRACT in RTL */
            result ^= (bf.a << 24) | (bf.b << 16) | (bf.c << 8) | bf.d;
        } else if ((i % 3) == 0) {
            /* Requirement 2: STRICT_LOW_PART via union */
            si.parts.low = i * 3;       /* Updates low 16 bits */
            si.bytes[1] = i & 0xFF;     /* Updates specific byte */
            
            /* Requirement 6: Inline assembly influencing RTL */
            asm volatile (
                "movw %w[low], %[short]\n\t"
                : [short] "=r" (reg_short)
                : [low] "r" (si.parts.low)
                : "cc"
            );
        } else {
            /* Requirement 3: SUBREG operations */
            /* Force truncation through register variable */
            reg_short = reg_var & 0xFFFF;
            
            /* Use inline assembly to suggest subregister access */
            asm volatile (
                "movzwl %k[short], %k[var]\n\t"
                : [var] "+r" (reg_var)
                : [short] "r" (reg_short)
                : "cc"
            );
            
            /* More SUBREG: access high byte */
            uint8_t high_byte = (reg_var >> 24) & 0xFF;
            result += high_byte;
        }
        
        /* Requirement 4: Complex memory addressing (MEM) */
        /* Multi-level pointer dereferencing */
        if (outer_ptr->first && outer_ptr->second) {
            /* Chain of pointer accesses */
            outer_ptr->first->data[i % 8] = result;
            outer_ptr->second->data[(i + 1) % 8] = outer_ptr->first->data[i % 8] * 2;
            
            /* Complex array indexing with non-constant expressions */
            int idx1 = (i * 3) % 4;
            int idx2 = (i * 5) % 4;
            outer_ptr->matrix[idx1][idx2] += 
                outer_ptr->first->data[i % 8] + 
                outer_ptr->second->data[(i + 1) % 8];
            
            /* Pointer arithmetic creating complex address */
            int *ptr = &outer_ptr->matrix[0][0];
            ptr += (idx1 * 4 + idx2);
            *ptr ^= result;
            
            /* Another MEM pattern: structure pointer chain */
            if (outer_ptr->first->next) {
                outer_ptr->first->next->data[0] = i;
            }
        }
        
        /* Mix operations to create complex RTL patterns */
        switch (i % 4) {
            case 0:
                /* Combine bitfield and memory access */
                bf.a = outer_ptr->matrix[0][0] & 0x7;
                break;
            case 1:
                /* Combine STRICT_LOW_PART and register variable */
                si.parts.low = reg_short;
                reg_var = si.full;
                break;
            case 2:
                /* Complex MEM with pointer arithmetic */
                int *mem_ptr = (int*)((char*)outer_ptr + sizeof(outer*) * 2);
                *mem_ptr = reg_var;
                break;
            case 3:
                /* All combined */
                bf.b = si.bytes[0];
                reg_short = outer_ptr->matrix[i % 4][0] & 0xFFFF;
                si.parts.high = reg_short;
                break;
        }
        
        /* Memory barrier to prevent too much optimization */
        MEMORY_BARRIER();
        
        /* Force subregister access */
        FORCE_SUBREG_ACCESS(reg_short);
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int final_result = 
        bf.a + bf.b + bf.c + bf.d +
        si.full +
        reg_var +
        reg_short +
        outer_ptr->matrix[0][0] +
        outer_ptr->matrix[3][3];
    
    /* Cleanup */
    free(outer_ptr->first);
    free(outer_ptr->second);
    free(outer_ptr);
    
    return final_result & 0xFF;  /* Return non-zero result */
}
