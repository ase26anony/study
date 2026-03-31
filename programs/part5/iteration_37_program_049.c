/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile uint32_t low : 5;
    volatile uint32_t mid : 11;
    volatile uint32_t high : 16;
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

/* Complex nested structure for memory addressing */
typedef struct level3 {
    volatile int data[3][3];
} level3_t;

typedef struct level2 {
    volatile level3_t *arr[4];
    volatile int extra;
} level2_t;

typedef struct level1 {
    volatile level2_t *inner[2];
    volatile long counter;
} level1_t;

/* Global variables to prevent over-optimization */
volatile bitfield_struct g_bf = {0};
volatile split_int g_split = {0};
volatile level1_t *g_root = NULL;
volatile int g_index = 0;

/* Function with inline assembly for SUBREG influence */
static void manipulate_with_subreg(volatile int *result) {
    /* Register variable for potential SUBREG */
    register int reg_var asm("eax") = *result;
    register short reg_short asm("si") = 0;
    
    /* Inline assembly suggesting subregister use */
    asm volatile (
        "movw %%ax, %%si\n\t"
        "addl $1, %%eax\n\t"
        : "+r" (reg_var), "=r" (reg_short)
        : 
        : "cc"
    );
    
    /* Operation requiring truncation */
    reg_short = (reg_short & 0xFF) + 1;
    
    /* Combine back - may generate SUBREG */
    reg_var = (reg_var & 0xFFFF0000) | reg_short;
    
    *result = reg_var;
}

int main(void) {
    volatile int i, j, k;
    volatile int array[10][10];
    volatile int *ptr_array[10];
    volatile int result = 0;
    
    /* Initialize pointer array */
    for (i = 0; i < 10; i++) {
        ptr_array[i] = &array[i][0];
    }
    
    /* Allocate and initialize nested structure */
    g_root = (volatile level1_t*)malloc(sizeof(level1_t));
    for (i = 0; i < 2; i++) {
        g_root->inner[i] = (volatile level2_t*)malloc(sizeof(level2_t));
        for (j = 0; j < 4; j++) {
            g_root->inner[i]->arr[j] = (volatile level3_t*)malloc(sizeof(level3_t));
            for (k = 0; k < 3; k++) {
                g_root->inner[i]->arr[j]->data[k][0] = i + j + k;
                g_root->inner[i]->arr[j]->data[k][1] = i * j * k;
                g_root->inner[i]->arr[j]->data[k][2] = i ^ j ^ k;
            }
        }
    }
    
    /* Main loop with combined operations */
    for (i = 0; i < 100; i++) {
        /* 1. Bitfield assignments (ZERO_EXTRACT) */
        if (i & 1) {
            g_bf.low = (i & 0x1F);        /* 5-bit field */
            g_bf.mid = ((i * 3) & 0x7FF); /* 11-bit field */
            g_bf.high = (i & 0xFFFF);     /* 16-bit field */
        } else {
            g_bf.low = (i * 7) & 0x1F;
            g_bf.mid = (i * 11) & 0x7FF;
            g_bf.high = (i * 13) & 0xFFFF;
        }
        
        /* 2. STRICT_LOW_PART via union/pointer */
        if (i & 2) {
            /* Update low 16 bits only */
            g_split.parts.low = (i * 17) & 0xFFFF;
        } else {
            /* Update specific byte */
            g_split.bytes[1] = (i * 19) & 0xFF;
        }
        
        /* 3. Complex memory addressing (MEM) */
        /* Multi-level pointer dereferencing with non-constant indices */
        volatile int idx1 = i % 2;
        volatile int idx2 = (i * 7) % 4;
        volatile int idx3 = (i * 11) % 3;
        volatile int idx4 = (i * 13) % 3;
        
        /* Chain: g_root->inner[idx1]->arr[idx2]->data[idx3][idx4] */
        int mem_val = g_root->inner[idx1]->arr[idx2]->data[idx3][idx4];
        
        /* 4. Array access through pointer array (more MEM complexity) */
        ptr_array[i % 10][(i * 3) % 10] = mem_val + g_split.parts.low;
        
        /* 5. SUBREG-related operations */
        manipulate_with_subreg(&result);
        
        /* 6. Additional inline assembly with constraints */
        register int temp asm("ebx") = result;
        asm volatile (
            "movl %1, %%eax\n\t"
            "shrl $8, %%eax\n\t"
            "movb %%al, %0\n\t"
            : "=m" (g_split.bytes[2])
            : "r" (temp)
            : "eax", "cc"
        );
        
        /* Conditional based on operations */
        if (g_bf.low > 16) {
            result += g_split.parts.high;
        } else if (g_bf.mid < 500) {
            result -= g_root->inner[0]->arr[0]->data[0][0];
        } else {
            result ^= ptr_array[i % 5][0];
        }
        
        /* Loop exit condition depends on manipulated values */
        if (result > 10000 || result < -10000) {
            break;
        }
    }
    
    /* Combine all results to prevent elimination */
    int final_result = result;
    final_result += g_bf.low + g_bf.mid + g_bf.high;
    final_result += g_split.full;
    final_result += g_root->inner[0]->arr[0]->data[0][0];
    final_result += ptr_array[0][0];
    
    /* Cleanup */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            free((void*)g_root->inner[i]->arr[j]);
        }
        free((void*)g_root->inner[i]);
    }
    free((void*)g_root);
    
    return final_result & 0xFF;  /* Ensure non-zero exit if everything worked */
}
