/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned int pad : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bf;
    unsigned int raw;
    unsigned short parts[2];
};

/* Function with complex operations to generate target RTL patterns */
int process_resources(int argc, char **argv) {
    volatile struct bitfield_pack bf_data = {0, 0, 0, 0};
    volatile union mixed_access *bf_ptr = (volatile union mixed_access *)&bf_data;
    
    /* Multi-word operations to generate SUBREG */
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    
    /* Array with complex indexing */
    volatile int data_array[256];
    for (int i = 0; i < 256; i++) {
        data_array[i] = i * 3;
    }
    
    /* Loop with volatile control to prevent optimization */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int result = 0;
    
    for (volatile int i = 0; i < loop_limit; i++) {
        /* ========== BIT-FIELD MANIPULATIONS ========== */
        /* These should generate ZERO_EXTRACT/STRICT_LOW_PART */
        
        /* Direct bit-field assignment */
        bf_data.flag1 = i & 1;
        bf_data.flag2 = (i >> 1) & 0x7;
        bf_data.value = (bf_data.value + i) & 0xFFF;
        
        /* Manual bit extraction using shifts and masks */
        unsigned int extracted = (bf_ptr->raw >> 4) & 0xFFF;  /* ZERO_EXTRACT pattern */
        
        /* Combined bit-field operations */
        bf_ptr->raw = (bf_ptr->raw & ~0xFFF) | ((bf_ptr->raw + extracted) & 0xFFF);
        
        /* ========== MULTI-WORD OPERATIONS ========== */
        /* These should generate SUBREG expressions */
        
        /* Operations on long long (multi-register on 32-bit) */
        ll_var = ll_var + (long long)(i * 0x10001LL);
        
        /* Access high and low parts separately */
        unsigned int low_part = (unsigned int)(ll_var & 0xFFFFFFFF);
        unsigned int high_part = (unsigned int)(ll_var >> 32);
        
        /* Mix high and low parts - forces SUBREG usage */
        ll_var = ((long long)(high_part ^ low_part) << 32) | (low_part + high_part);
        
        /* Double precision operations (also multi-word) */
        dbl_var = dbl_var * 1.01 + (double)i;
        
        /* Cast between double and long long */
        ll_var = ll_var ^ (long long)dbl_var;
        
        /* ========== COMPLEX MEMORY ADDRESSING ========== */
        /* Generate MEM with complex address expressions */
        
        /* Complex array indexing */
        int idx = ((bf_data.value * 13 + i * 17) & 0xFF);
        int idx2 = ((idx * 3 + 7) & 0xFF);
        
        /* Read-modify-write with bit manipulation */
        data_array[idx] = (data_array[idx] & ~0xFF) | (extracted & 0xFF);
        
        /* Pointer arithmetic with casting */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&data_array[idx2];
        byte_ptr[1] = byte_ptr[1] ^ (unsigned char)(bf_data.flag2 << 5);
        
        /* ========== CONTROL FLOW BASED ON RESULTS ========== */
        /* Force scheduling/reload to analyze the patterns */
        
        if (bf_data.flag1) {
            /* Branch 1: More bit-field operations */
            unsigned int temp = bf_ptr->parts[0];
            bf_ptr->parts[0] = bf_ptr->parts[1];
            bf_ptr->parts[1] = temp;
            
            /* Additional SUBREG operations */
            ll_var = ll_var >> (bf_data.flag2 + 1);
        } else {
            /* Branch 2: Different memory access pattern */
            for (int j = 0; j < 4; j++) {
                int offset = ((i + j) * 11) & 0xFF;
                data_array[offset] = data_array[offset] + (int)(ll_var & 0xFF);
            }
        }
        
        /* Switch based on extracted bits */
        switch (extracted & 0x7) {
            case 0:
                dbl_var = dbl_var + 1.0;
                break;
            case 1:
                ll_var = ll_var - 0x100000000LL;
                break;
            case 2:
                bf_data.value = (bf_data.value << 1) | bf_data.flag1;
                break;
            default:
                /* Complex addressing mode */
                data_array[(extracted + i) & 0xFF] = 
                    data_array[(extracted + i) & 0xFF] * 2 - 1;
                break;
        }
        
        /* Accumulate result to prevent dead code elimination */
        result += bf_data.value + (int)(ll_var & 0xFFFF) + data_array[i & 0xFF];
    }
    
    /* Final aggregation */
    result += (int)(ll_var >> 32) + (int)dbl_var;
    
    /* Use inline assembly to hint at specific register usage */
    /* This may generate patterns similar to STRICT_LOW_PART */
    asm volatile (
        "# Force register constraints\n"
        : "+r" (result)
        : 
        : "cc", "memory"
    );
    
    /* Additional inline assembly that operates on partial registers */
    unsigned int low_word, high_word;
    low_word = (unsigned int)(ll_var & 0xFFFFFFFF);
    high_word = (unsigned int)(ll_var >> 32);
    
    asm volatile (
        "addl %1, %0\n\t"
        "rorl $8, %0"
        : "+r" (low_word)
        : "r" (high_word)
        : "cc"
    );
    
    result += low_word;
    
    return result;
}

/* Main driver function */
int main(int argc, char **argv) {
    int final_result = process_resources(argc, argv);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Also return it from main */
    return final_result & 0xFF;
}
