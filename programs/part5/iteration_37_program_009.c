/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -march=native -frename-registers -fno-tree-vectorize */

#include <stdint.h>
#include <stdlib.h>

/* Bitfield struct for ZERO_EXTRACT */
typedef struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 9;
    volatile unsigned int field4 : 11;
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
} nested_struct;

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Inline assembly helper */
static inline void clobber_registers(void) {
    /* Clobber registers to force reloads and subreg operations */
    asm volatile (
        "movl %%eax, %%ebx\n\t"
        "movw %%ax, %%cx\n\t"
        :
        :
        : "eax", "ebx", "ecx", "memory"
    );
}

int main(void) {
    /* 1. Bitfield operations (ZERO_EXTRACT) */
    bitfield_struct bf = {0};
    volatile bitfield_struct *bf_ptr = &bf;
    
    /* 2. STRICT_LOW_PART operations */
    split_int si = {0};
    volatile split_int *si_ptr = &si;
    
    /* 3. Register variable for SUBREG */
    register int reg_var asm("eax") = 0x12345678;
    volatile register int *reg_ptr = &reg_var;
    
    /* 4. Complex memory structures */
    nested_struct ns;
    nested_struct *ns_ptr = &ns;
    int array[10][10];
    volatile int (*array_ptr)[10] = array;
    
    /* Initialize inner structure */
    struct inner inner_obj;
    inner_obj.ptr = (int*)malloc(sizeof(int) * 4);
    ns.inner_ptr = &inner_obj;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array[i][j] = i * 10 + j;
        }
    }
    
    /* Main loop with combined operations */
    for (int i = 0; i < 100; i++) {
        /* Conditional to create complex control flow */
        if (i % 3 == 0) {
            /* ZERO_EXTRACT: Bitfield assignment */
            bf_ptr->field2 = (i * 7) & 0x7F;
            bf_ptr->field3 = (i * 13) & 0x1FF;
        } else if (i % 3 == 1) {
            /* STRICT_LOW_PART: Partial register update */
            si_ptr->parts.low = (i * 17) & 0xFFFF;
            si_ptr->bytes[1] = (i * 23) & 0xFF;
        } else {
            /* Mixed operations */
            bf_ptr->field1 = (i * 11) & 0x1F;
            si_ptr->parts.high = (i * 29) & 0xFFFF;
        }
        
        /* SUBREG operations with register variable */
        {
            /* Force subregister access */
            volatile short *short_ptr = (volatile short*)&reg_var;
            *short_ptr = (i * 31) & 0xFFFF;
            
            /* Vector operation that may generate SUBREG */
            v4si vec1 = {1, 2, 3, 4};
            v4si vec2 = {5, 6, 7, 8};
            v4si vec3 = vec1 + vec2;
            
            /* Extract element (potential SUBREG) */
            reg_var += vec3[0] + vec3[2];
        }
        
        /* Complex memory addressing (MEM) */
        {
            /* Multi-level pointer dereferencing */
            int val1 = ns_ptr->inner_ptr->matrix[i % 3][(i + 1) % 3];
            
            /* Array indexing with non-constant expression */
            int val2 = array_ptr[(i * 3) % 10][(i * 7) % 10];
            
            /* Chain of operations */
            ns_ptr->inner_ptr->ptr[(i * 11) % 4] = 
                val1 + val2 + bf_ptr->field1;
        }
        
        /* Inline assembly to influence RTL generation */
        {
            int temp = reg_var + si.full;
            asm volatile (
                "movl %1, %%eax\n\t"
                "movw %%ax, %0\n\t"
                : "=m" (global_counter)
                : "r" (temp)
                : "eax", "memory"
            );
        }
        
        /* Additional clobbering */
        clobber_registers();
        
        /* Update global result to prevent dead code elimination */
        global_result += bf.field2 + si.parts.low + reg_var;
        
        /* Complex exit condition */
        if (global_result > 1000000) {
            break;
        }
    }
    
    /* Final computation using all variables */
    int final_result = 
        bf.field1 * 2 +
        bf.field2 * 3 +
        bf.field3 * 4 +
        bf.field4 * 5 +
        si.full +
        reg_var +
        global_result +
        ns.inner_ptr->matrix[0][0] +
        array[5][5];
    
    /* Cleanup */
    free(inner_obj.ptr);
    
    return final_result % 256;
}
