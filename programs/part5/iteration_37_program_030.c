/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */
/* For RTL analysis: add -dP -fdump-rtl-expand -fdump-rtl-sched1 -fdump-rtl-reload */

#include <stdint.h>
#include <stdlib.h>

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
    volatile unsigned int padding : 32;
} __attribute__((packed));

/* Union for STRICT_LOW_PART operations */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex nested structure for memory addressing */
struct level3 {
    volatile int data[3][2];
    volatile struct level3* next;
};

struct level2 {
    volatile struct level3 inner[2];
    volatile int extra;
};

struct level1 {
    volatile struct level2* ptr;
    volatile int index;
};

/* Global variables to prevent optimization */
volatile struct bitfield_struct g_bf = {0};
volatile union split_int g_split = {0};
volatile struct level1 g_root = {0};
volatile int** g_dynamic_array = NULL;
volatile int g_counter = 0;

/* Register variable for SUBREG operations */
register volatile uint32_t reg_var asm ("r12");

/* Vector type for potential SUBREG extraction */
typedef int v4si __attribute__ ((vector_size (16)));
volatile v4si g_vector;

/* Inline assembly helper */
#define FORCE_SUBREG_OP(var, val) \
    asm volatile ("add {%1, %0 | %0, %1}" \
                  : "+r" (var) \
                  : "ri" ((uint16_t)(val)) \
                  : "cc")

int main(void) {
    int i, j, k;
    volatile int result = 0;
    
    /* Initialize structures */
    struct level2 l2[4];
    struct level3 l3[8];
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 2; k++) {
                l3[i].data[j][k] = i * 10 + j * 3 + k;
            }
        }
        l3[i].next = (i < 7) ? &l3[i + 1] : NULL;
    }
    
    for (i = 0; i < 4; i++) {
        l2[i].inner[0] = l3[i * 2];
        l2[i].inner[1] = l3[i * 2 + 1];
        l2[i].extra = i * 100;
    }
    
    g_root.ptr = &l2[0];
    g_root.index = 1;
    
    /* Dynamic array for complex memory addressing */
    g_dynamic_array = (int**)malloc(8 * sizeof(int*));
    for (i = 0; i < 8; i++) {
        g_dynamic_array[i] = (int*)malloc(16 * sizeof(int));
        for (j = 0; j < 16; j++) {
            g_dynamic_array[i][j] = i * 100 + j;
        }
    }
    
    /* Initialize register variable */
    reg_var = 0x12345678;
    
    /* Initialize vector */
    g_vector = (v4si){1, 2, 3, 4};
    
    /* Main loop - designed to generate target RTL expressions */
    for (volatile int iter = 0; iter < 100; iter++) {
        /* 1. BIT-FIELD OPERATIONS (ZERO_EXTRACT) */
        if (iter & 1) {
            g_bf.field1 = (iter & 0x7);           /* 3-bit field */
            g_bf.field2 = (iter & 0x1F) >> 3;     /* 5-bit field */
            g_bf.field3 = (iter & 0xFF) >> 8;     /* 8-bit field */
            g_bf.field4 = iter;                   /* 16-bit field */
        } else {
            /* Alternative bit-field pattern */
            g_bf.field3 = g_bf.field1 + g_bf.field2;
            g_bf.field4 = g_bf.field3 << 4;
        }
        
        /* 2. STRICT_LOW_PART operations */
        if (iter & 2) {
            /* Update low 16 bits */
            g_split.parts.low = iter * 3;
            
            /* Update specific byte */
            g_split.bytes[1] = iter & 0xFF;
            
            /* Pointer-based partial update */
            *((volatile uint16_t*)&g_split.full + 1) = iter * 5;
        }
        
        /* 3. SUBREG operations with register variable */
        if (iter & 4) {
            /* Operation that should generate SUBREG for lower part */
            uint16_t temp = reg_var & 0xFFFF;
            temp += iter;
            reg_var = (reg_var & 0xFFFF0000) | temp;
            
            /* Force SUBREG through inline assembly */
            FORCE_SUBREG_OP(reg_var, iter);
            
            /* Vector element extraction (potential SUBREG) */
            int elem = g_vector[iter & 3];
            reg_var += elem;
        }
        
        /* 4. COMPLEX MEMORY ADDRESSING */
        if (iter & 8) {
            /* Multi-level pointer dereferencing */
            result += g_root.ptr->inner[g_root.index].data[iter % 3][(iter >> 2) & 1];
            
            /* Chain with pointer arithmetic */
            result += g_root.ptr[iter % 2].inner[0].next->data[1][0];
            
            /* Dynamic array with complex indexing */
            int idx1 = (iter * 7) % 8;
            int idx2 = (iter * 13) % 16;
            result += g_dynamic_array[idx1][idx2];
            
            /* Pointer chain with array indexing */
            volatile struct level3* cur = &g_root.ptr->inner[0];
            for (int n = 0; n < 2 && cur; n++) {
                result += cur->data[n % 3][n & 1];
                cur = cur->next;
            }
        }
        
        /* 5. COMBINATION in conditional context */
        switch (iter % 5) {
            case 0:
                /* Bit-field in switch context */
                g_bf.field1 = result & 0x7;
                break;
            case 1:
                /* Partial register update in switch */
                g_split.bytes[iter % 4] = result & 0xFF;
                break;
            case 2:
                /* Memory addressing in switch */
                result += g_dynamic_array[result % 8][0];
                break;
            case 3:
                /* Mixed operations */
                g_bf.field2 = g_split.bytes[0];
                g_split.parts.low = g_bf.field3;
                break;
            case 4:
                /* Inline assembly affecting C variables */
                asm volatile ("mov {%0, %%r12b | %%r12b, %0}" 
                              : : "r" ((uint8_t)result) : "%r12");
                break;
        }
        
        /* Loop exit condition based on operations */
        if ((result & 0xFFF) > 0x800) {
            g_counter++;
            if (g_counter > 50) break;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int final_result = 
        (g_bf.field1 + g_bf.field2 + g_bf.field3 + g_bf.field4) +
        g_split.full +
        (reg_var & 0xFFFFFFFF) +
        result +
        g_counter;
    
    /* Cleanup */
    for (i = 0; i < 8; i++) {
        free((void*)g_dynamic_array[i]);
    }
    free((void*)g_dynamic_array);
    
    return final_result & 0xFF;
}
