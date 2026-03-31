/* Compile with: gcc -O2 -fschedule-insns -fno-gcse -fno-strict-aliasing -frename-registers -march=native */

#include <stdint.h>
#include <stdlib.h>

/* Volatile struct with bitfields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
} __attribute__((packed));

/* Union for STRICT_LOW_PART operations */
union type_pun {
    volatile uint32_t full;
    volatile struct {
        uint16_t low;
        uint16_t high;
    } parts;
    volatile uint8_t bytes[4];
};

/* Complex memory structure */
struct nested {
    int data[3][4];
    struct nested *next;
};

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function with inline assembly for register manipulation */
static inline void reg_manipulation(volatile int *out) {
    register int eax_var asm("eax") = *out;
    register short ax_var asm("ax") = (short)eax_var;
    
    /* Inline assembly suggesting subregister use */
    asm volatile (
        "addw $1, %0\n\t"
        "movw %0, %1"
        : "+r"(ax_var), "=m"(*out)
        : 
        : "cc"
    );
    
    /* Force SUBREG through vector operations */
    v4si vec = {1, 2, 3, 4};
    int element = vec[2];  /* This may generate SUBREG */
    *out += element;
}

int main(void) {
    struct bitfield_struct bf = {0};
    union type_pun pun = {.full = 0x12345678};
    volatile int *heap_array = (volatile int*)malloc(100 * sizeof(int));
    struct nested *nested_ptr = (struct nested*)calloc(1, sizeof(struct nested));
    
    /* Initialize complex memory structure */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            nested_ptr->data[i][j] = i * 10 + j;
        }
    }
    
    /* Loop combining all patterns */
    for (volatile int i = 0; i < 100; i++) {
        /* 1. ZERO_EXTRACT through bitfield assignments */
        if (i & 1) {
            bf.field1 = (i & 0x7);           /* 3-bit field */
            bf.field3 = (i & 0xFF);          /* 8-bit field */
        } else {
            bf.field2 = (i & 0x1F);          /* 5-bit field */
            bf.field4 = (i & 0xFFFF);        /* 16-bit field */
        }
        
        /* 2. STRICT_LOW_PART through partial updates */
        switch (i % 4) {
            case 0:
                pun.parts.low = i & 0xFFFF;  /* Update low 16 bits */
                break;
            case 1:
                pun.bytes[1] = i & 0xFF;     /* Update single byte */
                break;
            case 2:
                ((volatile short*)&pun.full)[0] = i & 0xFFFF; /* Pointer cast */
                break;
            case 3:
                pun.parts.high = (i >> 16) & 0xFFFF;
                break;
        }
        
        /* 3. SUBREG through register variables and vector operations */
        volatile int subreg_temp = i;
        reg_manipulation(&subreg_temp);
        
        /* 4. Complex MEM addressing through pointer chains */
        int idx = i % 3;
        int jdx = (i * 7) % 4;
        
        /* Multi-level pointer dereferencing */
        volatile int *ptr1 = &nested_ptr->data[idx][jdx];
        volatile int **ptr2 = &ptr1;
        volatile int ***ptr3 = &ptr2;
        
        /* Complex memory access pattern */
        heap_array[i] = ***ptr3 + bf.field1;
        nested_ptr->data[idx][jdx] = heap_array[i] * 2;
        
        /* Chain pointer access */
        if (nested_ptr->next) {
            nested_ptr->next->data[idx][jdx] = ***ptr3;
        }
        
        /* Additional inline assembly with constraints */
        register int eax_val asm("eax") = i;
        register int ebx_val asm("ebx") = subreg_temp;
        
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0\n\t"
            "movb %%al, %b2\n\t"
            : "+r"(global_result), "+r"(eax_val), "+r"(ebx_val)
            : 
            : "cc", "eax"
        );
        
        /* Conditional to create more complex CFG */
        if (global_result > 1000) {
            global_result /= 2;
        }
        
        /* Prevent loop elimination */
        global_counter += (bf.field1 | pun.parts.low | subreg_temp);
    }
    
    /* Final computation using all variables */
    int result = 
        bf.field1 + 
        bf.field2 * 2 + 
        bf.field3 * 3 + 
        bf.field4 +
        pun.full +
        heap_array[50] +
        nested_ptr->data[1][2] +
        global_counter +
        global_result;
    
    /* Cleanup */
    free((void*)heap_array);
    free(nested_ptr);
    
    return result % 256;
}
