/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Packed structs with bitfields for ZERO_EXTRACT generation */
struct __attribute__((packed)) bitfield_struct {
    unsigned int header: 4;
    unsigned int data: 12;
    unsigned int footer: 8;
    unsigned int checksum: 8;
};

struct __attribute__((packed)) mixed_bitfield {
    unsigned short low: 5;
    unsigned short mid: 7;
    unsigned short high: 4;
    unsigned char extra: 3;
    unsigned char pad: 5;
};

/* Global volatile variables to prevent optimization */
volatile unsigned int global_counter = 0;
volatile unsigned int global_result = 0;
volatile unsigned char global_byte = 0;

/* Test 1: Bitfield extraction from volatile integers */
void test_bitfield_extraction_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int bits_4_11 = (source >> 4) & 0xFF;
        
        /* Extract bits 12-19 with arithmetic */
        unsigned int bits_12_19 = ((source >> 12) + i) & 0xFF;
        
        /* Extract bits 20-27 using mask */
        unsigned int mask = (1U << 8) - 1;
        unsigned int bits_20_27 = (source >> 20) & mask;
        
        /* Combine extractions */
        result = bits_4_11 + bits_12_19 + bits_20_27;
        
        /* Modify source to create variation */
        source = (source * 1103515245U + 12345U) & 0xFFFFFFFFU;
    }
    
    global_result ^= result;
    COMPILER_BARRIER();
}

/* Test 2: Packed struct bitfield operations */
void test_packed_struct_bitfields(void) {
    volatile struct bitfield_struct bs = {0};
    volatile struct mixed_bitfield mb = {0};
    
    bs.header = 0xA;
    bs.data = 0xABC;
    bs.footer = 0x42;
    bs.checksum = 0xFF;
    
    mb.low = 0x1F;
    mb.mid = 0x7F;
    mb.high = 0x0F;
    mb.extra = 0x7;
    
    volatile unsigned int sum = 0;
    
    /* Loop with bitfield reads and writes */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int data_val = bs.data;
        unsigned int checksum_val = bs.checksum;
        
        /* Cross-struct operation */
        unsigned int combined = (mb.mid << 4) | mb.low;
        
        /* Conditional bitfield update */
        if (i % 3 == 0) {
            bs.data = (bs.data + combined) & 0xFFF;  /* 12-bit mask */
        }
        
        /* Mixed bitfield arithmetic */
        mb.high = (mb.low + mb.mid) & 0x0F;
        mb.extra = (checksum_val >> 4) & 0x07;
        
        sum += data_val + checksum_val + combined;
        
        /* Nested loop for scheduling complexity */
        for (int j = 0; j < 10; ++j) {
            bs.footer = (bs.footer + j) & 0xFF;
        }
    }
    
    global_result += sum;
    COMPILER_BARRIER();
}

/* Test 3: Complex bitfield extraction with memory operations */
void test_complex_extraction_memory(void) {
    volatile unsigned int buffer[64];
    volatile unsigned int results[64];
    
    /* Initialize buffer with pattern */
    for (int i = 0; i < 64; ++i) {
        buffer[i] = (i * 0x1234567) & 0xFFFFFFFF;
    }
    
    /* Perform various bitfield extractions from memory */
    for (int i = 0; i < 63; ++i) {
        /* Extract different bit ranges */
        unsigned int val1 = buffer[i];
        unsigned int val2 = buffer[i + 1];
        
        /* Multiple ZERO_EXTRACT patterns */
        unsigned int low_bits = (val1 >> 0) & 0x3FF;      /* bits 0-9 */
        unsigned int mid_bits = (val1 >> 10) & 0x7FF;     /* bits 10-20 */
        unsigned int high_bits = (val1 >> 21) & 0x7FF;    /* bits 21-31 */
        
        /* Cross-element extraction */
        unsigned int cross = ((val1 >> 24) & 0xFF) | ((val2 << 8) & 0xFF00);
        
        /* Conditional merge operation */
        if (low_bits > 0x200) {
            /* Partial update pattern that might generate STRICT_LOW_PART */
            buffer[i] = (buffer[i] & ~0x3FF) | ((low_bits + mid_bits) & 0x3FF);
        }
        
        results[i] = low_bits + mid_bits + high_bits + cross;
    }
    
    /* Sum results to prevent elimination */
    volatile unsigned int total = 0;
    for (int i = 0; i < 64; ++i) {
        total += results[i];
    }
    
    global_result ^= total;
    COMPILER_BARRIER();
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional low-byte update - may generate STRICT_LOW_PART */
        if (flag) {
            /* Pattern: (reg & ~mask) | (new_val & mask) */
            unsigned char new_byte = (i * 7) & 0xFF;
            reg = (reg & ~0xFF) | (new_byte & 0xFF);
        }
        
        /* Conditional low-word update */
        if (i % 5 == 0) {
            unsigned short new_word = (i * 13) & 0xFFFF;
            reg = (reg & ~0xFFFF) | (new_word & 0xFFFF);
        }
        
        /* Update flag */
        flag = (flag + i) & 1;
        
        /* Memory barrier to prevent reordering */
        COMPILER_BARRIER();
    }
    
    global_result += reg;
}

/* Test 5: Inline assembly for partial register updates */
void test_inline_asm_partial_updates(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int temp;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that hints at partial register updates */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r"(temp)
            : "r"(value), "i"(0x00FFFFFF)  /* Keep lower 24 bits */
        );
        
        /* Another pattern with shift and mask */
        asm volatile (
            "lsr %0, %1, #8\n\t"
            "and %0, %0, %2\n\t"
            : "=r"(temp)
            : "r"(value), "i"(0x0000FFFF)
        );
        
        /* Update value */
        value = (value * 3 + i) & 0xFFFFFFFF;
        
        /* Mix with bitfield extraction */
        unsigned int low_byte = value & 0xFF;
        unsigned int high_byte = (value >> 24) & 0xFF;
        
        /* Conditional partial store */
        if (low_byte > 0x80) {
            value = (value & ~0xFF00) | ((low_byte << 8) & 0xFF00);
        }
    }
    
    global_result ^= value;
    COMPILER_BARRIER();
}

/* Test 6: Mixed operations with switch statement */
void test_mixed_with_switch(void) {
    volatile struct bitfield_struct bs = {0};
    volatile unsigned int state = 0;
    
    bs.data = 0x555;
    bs.checksum = 0xAA;
    
    for (int i = 0; i < 100; ++i) {
        /* Extract bitfield for switch control */
        unsigned int control = bs.data & 0x7;  /* Lower 3 bits */
        
        switch (control) {
            case 0:
                /* ZERO_EXTRACT pattern */
                state = (bs.checksum >> 2) & 0x3F;
                break;
            case 1:
                /* STRICT_LOW_PART-like update */
                state = (state & ~0xF) | (i & 0xF);
                break;
            case 2:
                /* Memory operation */
                bs.data = (bs.data + state) & 0xFFF;
                break;
            case 3:
                /* Complex extraction */
                state = ((bs.data >> 4) & 0xFF) + ((bs.checksum << 2) & 0xFC);
                break;
            default:
                /* Mixed operation */
                state = (state * 11) & 0xFFFF;
                bs.checksum = (bs.checksum + 1) & 0xFF;
                break;
        }
        
        /* Update bitfields */
        bs.data = (bs.data + i) & 0xFFF;
        if (i % 7 == 0) {
            bs.checksum ^= (state >> 8) & 0xFF;
        }
    }
    
    global_result += state + bs.data + bs.checksum;
    COMPILER_BARRIER();
}

/* Test 7: Pointer-based partial updates */
void test_pointer_partial_updates(void) {
    volatile unsigned int data = 0x9ABCDEF0;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    
    for (int i = 0; i < 100; ++i) {
        /* Partial update via byte pointer - may generate partial store RTL */
        byte_ptr[1] = (i * 3) & 0xFF;      /* Update second byte */
        byte_ptr[3] ^= 0x55;               /* Update fourth byte */
        
        /* Extract bits using pointer arithmetic */
        unsigned int low_word = data & 0xFFFF;
        unsigned int high_word = (data >> 16) & 0xFFFF;
        
        /* Conditional merge of low bytes */
        if (low_word > 0x8000) {
            data = (data & ~0xFFFF) | ((low_word + high_word) & 0xFFFF);
        }
        
        /* Bitfield extraction from the updated value */
        unsigned int bits_8_15 = (data >> 8) & 0xFF;
        unsigned int bits_16_23 = (data >> 16) & 0xFF;
        
        /* Update based on extracted bits */
        byte_ptr[0] = bits_8_15 ^ bits_16_23;
    }
    
    global_result ^= data;
    COMPILER_BARRIER();
}

/* Main driver function */
int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    /* Parse command line argument if provided */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Initialize volatile data */
    volatile unsigned int base_value = 0x12345678;
    volatile unsigned char control_flag = 1;
    
    /* Run selected test or all tests */
    if (test_to_run == 0 || test_to_run == 1) {
        test_bitfield_extraction_volatile();
        printf("Test 1 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 2) {
        test_packed_struct_bitfields();
        printf("Test 2 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 3) {
        test_complex_extraction_memory();
        printf("Test 3 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 4) {
        test_strict_low_part_conditional();
        printf("Test 4 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 5) {
        test_inline_asm_partial_updates();
        printf("Test 5 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 6) {
        test_mixed_with_switch();
        printf("Test 6 completed\n");
    }
    
    if (test_to_run == 0 || test_to_run == 7) {
        test_pointer_partial_updates();
        printf("Test 7 completed\n");
    }
    
    /* Use all volatile results to prevent dead code elimination */
    volatile unsigned int final_result = global_result + global_counter + global_byte + base_value + control_flag;
    
    /* Print something to ensure execution */
    printf("Final checksum: %u\n", final_result & 0xFF);
    
    return (final_result > 0) ? 0 : 1;
}
