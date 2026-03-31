/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -o coverage_test coverage_test.c */

#include <stdint.h>
#include <stdlib.h>

/* 1. Bitfield operations for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} __attribute__((packed));

/* 2. Union for STRICT_LOW_PART operations */
union split_int {
    volatile uint32_t full;
    struct {
        volatile uint16_t low;
        volatile uint16_t high;
    } parts;
};

/* 3. Complex memory structure for MEM operations */
struct nested {
    int data[4][4];
    struct nested *next;
};

/* 4. Vector type for potential SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile struct bitfield_struct bf;
volatile union split_int split;
volatile struct nested *nested_ptr;
volatile v4si vec_var;
volatile int *mem_ptr_array[10];

/* Inline assembly helper to force register usage */
#define FORCE_REGISTER_USE(var, reg) \
    asm volatile("" : "+" #reg (var))

/* Main function with loop containing all required patterns */
int main(int argc, char *argv[]) {
    /* Initialize variables */
    struct bitfield_struct local_bf = {0};
    union split_int local_split = {0};
    struct nested nested_array[5];
    v4si local_vec = {1, 2, 3, 4};
    int i, j, k;
    
    /* Initialize nested structure */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                nested_array[i].data[j][k] = i * 16 + j * 4 + k;
            }
        }
        nested_array[i].next = (i < 4) ? &nested_array[i + 1] : NULL;
    }
    
    nested_ptr = &nested_array[0];
    
    /* Initialize memory pointer array */
    for (i = 0; i < 10; i++) {
        mem_ptr_array[i] = (int*)malloc(16 * sizeof(int));
        for (j = 0; j < 16; j++) {
            mem_ptr_array[i][j] = i * 100 + j;
        }
    }
    
    /* Register variable for SUBREG operations */
    register int reg_var asm("eax") = argc;
    register short reg_short asm("bx");
    
    /* Main loop with all required patterns */
    for (i = 0; i < 100; i++) {
        /* Pattern 1: Bitfield assignments (ZERO_EXTRACT) */
        if (i % 3 == 0) {
            local_bf.field1 = (i & 0x7);
            local_bf.field2 = ((i * 2) & 0x1F);
            local_bf.field3 = ((i * 3) & 0xFF);
            local_bf.field4 = ((i * 100) & 0xFFFF);
            
            /* Copy to volatile global */
            bf = local_bf;
        }
        
        /* Pattern 2: STRICT_LOW_PART via union/pointer */
        if (i % 5 == 0) {
            /* Update low part only */
            local_split.parts.low = i & 0xFFFF;
            
            /* Alternative: pointer cast for strict low part */
            *((volatile short*)&local_split.full) = (i * 7) & 0xFFFF;
            
            /* Copy to volatile global */
            split = local_split;
        }
        
        /* Pattern 3: SUBREG operations with register variables */
        if (i % 7 == 0) {
            /* Force use of register variable with smaller type */
            reg_short = (short)(reg_var + i);
            
            /* Inline assembly that suggests subregister use */
            asm volatile (
                "movw %w[input], %[output]"
                : [output] "=r" (reg_short)
                : [input] "r" (reg_var)
            );
            
            /* Vector operations that may generate SUBREG */
            local_vec += (v4si){1, 1, 1, 1};
            int element = local_vec[2];  /* This may use SUBREG */
            
            /* Copy to volatile global */
            vec_var = local_vec;
        }
        
        /* Pattern 4: Complex memory addressing (MEM) */
        if (i % 11 == 0) {
            /* Multi-level pointer dereferencing */
            int ***triple_ptr = (int***)mem_ptr_array;
            int val1 = ***triple_ptr;
            
            /* Array indexing with non-constant expressions */
            int idx1 = i % 10;
            int idx2 = (i * 3) % 16;
            int val2 = mem_ptr_array[idx1][idx2];
            
            /* Structure pointer chains */
            int val3 = nested_ptr->next->next->data[i % 4][(i * 2) % 4];
            
            /* Complex calculation with memory operands */
            mem_ptr_array[idx1][idx2] = val1 + val2 + val3 + i;
            
            /* Update nested pointer */
            nested_ptr = nested_ptr->next ? nested_ptr->next : &nested_array[0];
        }
        
        /* Pattern 5: Combined operation in conditional */
        if (i % 13 == 0) {
            /* Mix bitfield and memory operations */
            local_bf.field1 = nested_ptr->data[0][0] & 0x7;
            
            /* Update memory based on bitfield */
            mem_ptr_array[i % 10][0] = local_bf.field2;
            
            /* Use register variable with memory */
            reg_var = mem_ptr_array[i % 10][0] + local_split.parts.low;
        }
        
        /* Pattern 6: Switch statement with different operations */
        switch (i % 4) {
            case 0:
                /* Bitfield in switch */
                local_bf.field3 = i & 0xFF;
                break;
            case 1:
                /* STRICT_LOW_PART in switch */
                local_split.parts.high = (i >> 8) & 0xFFFF;
                break;
            case 2:
                /* Memory operation in switch */
                nested_ptr->data[i % 4][i % 4] = i;
                break;
            case 3:
                /* Combined operation */
                reg_var = local_bf.field4 + local_split.full;
                break;
        }
        
        /* Force register variable usage */
        FORCE_REGISTER_USE(reg_var, eax);
        
        /* Loop exit condition based on all operations */
        if (reg_var > 10000) {
            break;
        }
    }
    
    /* Calculate return value using all manipulated variables */
    int result = 0;
    result += bf.field1;
    result += bf.field2;
    result += bf.field3;
    result += bf.field4;
    result += split.full;
    result += vec_var[0];
    result += nested_ptr->data[0][0];
    result += *mem_ptr_array[0];
    result += reg_var;
    
    /* Cleanup */
    for (i = 0; i < 10; i++) {
        free((void*)mem_ptr_array[i]);
    }
    
    return result % 256;
}
