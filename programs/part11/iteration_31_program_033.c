/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields of varying widths to force ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag : 1;      /* 1-bit field */
    unsigned int mode : 3;      /* 3-bit field */
    unsigned int value : 12;    /* 12-bit field */
    unsigned int pad : 16;      /* padding to 32 bits */
} __attribute__((packed));

/* Union to enable bit-twiddling access */
union bit_access {
    struct bitfield_pack fields;
    volatile unsigned int raw;
};

/* Function to create complex memory addressing */
static int complex_index(int i, int stride, int offset) {
    return i * stride + offset;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile int i, j;
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile union bit_access data[4];
    volatile int array[256];
    volatile int *ptr_array = array;
    int result = 0;
    
    /* Initialize data */
    for (i = 0; i < 4; i++) {
        data[i].raw = (i * 0x11111111) & 0xFFFFFFFF;
    }
    
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Loop with volatile control to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    /* Main loop - each iteration should generate complex RTL */
    for (volatile int counter = 0; counter < loop_limit; counter++) {
        /* ===== BIT-FIELD MANIPULATIONS ===== */
        /* These should generate ZERO_EXTRACT and STRICT_LOW_PART */
        
        /* 1. Direct bit-field assignment (potential STRICT_LOW_PART) */
        data[0].fields.value = (data[0].fields.value + counter) & 0xFFF;
        
        /* 2. Bit-field extraction using masking (potential ZERO_EXTRACT) */
        unsigned int extracted = (data[1].raw >> 4) & 0xFFF;
        
        /* 3. Cross-bit-field operations */
        data[2].fields.mode = (data[2].fields.flag << 2) | 
                              (data[3].fields.mode & 0x3);
        
        /* 4. Complex bit-field expression */
        data[3].fields.value = ((data[0].fields.value << 4) | 
                               (data[1].fields.mode << 1) | 
                               data[2].fields.flag) & 0xFFF;
        
        /* ===== MULTI-WORD OPERATIONS ===== */
        /* These should generate SUBREG expressions on 32-bit targets */
        
        /* 1. long long arithmetic - forces split into high/low parts */
        ll_var = ll_var + 0x100000001LL;
        ll_var = ll_var ^ (1LL << (counter & 0x3F));
        
        /* 2. Double precision operations */
        dbl_var = dbl_var * 1.01;
        dbl_var = dbl_var + (double)(counter & 0xF);
        
        /* 3. Comparison of high vs low parts */
        int high_part = (int)(ll_var >> 32);
        int low_part = (int)(ll_var & 0xFFFFFFFF);
        
        /* ===== COMPLEX MEMORY ADDRESSING ===== */
        /* These should generate MEM with complex address expressions */
        
        /* 1. Array access with complex indexing */
        int idx = complex_index(counter, 7, 13) & 0xFF;
        array[idx] = array[idx] + extracted;
        
        /* 2. Pointer arithmetic with casting */
        int *ptr = (int *)((char *)ptr_array + (counter * 4) % 256);
        *ptr = *ptr ^ low_part;
        
        /* 3. Misaligned access simulation */
        if (counter & 1) {
            char *byte_ptr = (char *)&array[idx];
            int word = *(int *)(byte_ptr + 1);  /* Potentially unaligned */
            array[(idx + 1) & 0xFF] = word & 0x7FFFFFFF;
        }
        
        /* ===== CONTROL FLOW BASED ON BIT-FIELD RESULTS ===== */
        /* Forces scheduling/reload to analyze the RTL patterns */
        
        if (data[0].fields.flag ^ (counter & 1)) {
            /* Branch 1: More bit-field manipulations */
            data[1].fields.mode = (data[1].fields.mode + 1) & 0x7;
            ll_var = ll_var << 1;
        } else {
            /* Branch 2: Different operations */
            data[2].fields.value = (data[2].fields.value >> 1) & 0xFFF;
            dbl_var = dbl_var / 1.5;
        }
        
        /* Switch based on bit-field value */
        switch (data[3].fields.mode & 0x3) {
            case 0:
                array[counter & 0xFF] = high_part;
                break;
            case 1:
                array[counter & 0xFF] = low_part;
                break;
            case 2:
                array[counter & 0xFF] = (int)dbl_var;
                break;
            default:
                array[counter & 0xFF] = extracted;
                break;
        }
        
        /* Inline assembly to mimic specific patterns */
        /* This asm operates on low 32 bits of ll_var, potentially creating
           STRICT_LOW_PART-like constraints */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (low_part)
            : "r" (high_part & 0xFF)
            : "cc"
        );
        
        /* Another asm that could generate ZERO_EXTRACT-like behavior */
        unsigned int temp;
        asm volatile (
            "movl %1, %0\n\t"
            "andl $0xFFF, %0\n\t"
            : "=r" (temp)
            : "r" (data[0].raw)
            : "cc"
        );
        data[1].fields.value = temp;
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (i = 0; i < 256; i++) {
        result += array[i];
    }
    result += (int)ll_var;
    result += (int)(ll_var >> 32);
    result += (int)dbl_var;
    result += data[0].raw + data[1].raw + data[2].raw + data[3].raw;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
