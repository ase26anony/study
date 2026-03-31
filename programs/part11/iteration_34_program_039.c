/* test_resource.c - Coverage test for ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & 0xFF;
        result += extracted;
        
        /* Extract bits 16-23 with different width */
        extracted = (source >> 16) & 0xFF;
        result ^= extracted;
        
        /* Extract bits 8-15, store back to volatile */
        volatile unsigned int temp = (source >> 8) & 0xFF;
        result |= temp;
        
        COMPILER_BARRIER();
        source = (source << 1) | (source >> 31); /* Rotate */
    }
    
    /* Use result to prevent dead code elimination */
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 2: Packed struct with bitfields */
struct packed_bitfields {
    unsigned int field1 : 5;
    unsigned int field2 : 11;
    unsigned int field3 : 7;
    unsigned int field4 : 9;
} __attribute__((packed));

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = s.field1;
        unsigned int val2 = s.field2;
        unsigned int val3 = s.field3;
        unsigned int val4 = s.field4;
        
        /* Write bitfields - may generate ZERO_EXTRACT in SET_DEST */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 - val3) & 0x1FF;
        
        accumulator += s.field1 + s.field2 + s.field3 + s.field4;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 3: Mixed bitfield operations with arithmetic */
void test_zero_extract_mixed(void) {
    volatile unsigned int data[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    volatile unsigned int results[4] = {0};
    
    for (int i = 0; i < 75; ++i) {
        /* Complex extraction patterns */
        unsigned int idx = i & 3;
        
        /* Extract varying bit ranges */
        unsigned int ext1 = (data[idx] >> (i % 16)) & ((1U << 8) - 1);
        unsigned int ext2 = (data[(idx + 1) & 3] >> 12) & ((1U << 12) - 1);
        unsigned int ext3 = (data[(idx + 2) & 3] >> 4) & ((1U << 16) - 1);
        
        /* Combine extractions */
        results[idx] = (ext1 << 16) | (ext2 << 8) | ext3;
        
        /* Update source with partial result */
        data[idx] = (data[idx] & 0xFFFF0000) | (results[idx] & 0xFFFF);
        
        COMPILER_BARRIER();
    }
    
    /* Use results */
    volatile unsigned int sum = results[0] + results[1] + results[2] + results[3];
    (void)sum;
}

/* Test 4: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    volatile unsigned int values[8];
    volatile unsigned int masks[8];
    
    /* Initialize */
    for (int i = 0; i < 8; ++i) {
        values[i] = i * 0x11111111;
        masks[i] = (1U << (i + 1)) - 1;
    }
    
    for (int i = 0; i < 100; ++i) {
        int idx = i & 7;
        
        /* Inline assembly that modifies only low bits */
        unsigned int val = values[idx];
        unsigned int mask = masks[idx];
        
        /* Assembly that operates on low part */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (val)
            : "r" (val), "r" (mask)
        );
        
        /* Store back through volatile pointer */
        volatile unsigned int *ptr = &values[idx];
        *ptr = val;
        
        /* Another assembly pattern */
        unsigned int new_val = i * 0x01010101;
        asm volatile (
            "orr %0, %1, %2\n\t"
            : "=r" (new_val)
            : "r" (val), "r" (new_val & 0xFF)
        );
        
        values[(idx + 1) & 7] = new_val;
        COMPILER_BARRIER();
    }
    
    /* Compute checksum */
    volatile unsigned int checksum = 0;
    for (int i = 0; i < 8; ++i) {
        checksum ^= values[i];
    }
    (void)checksum;
}

/* Test 5: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int registers[4] = {0};
    volatile unsigned char conditions[4] = {1, 0, 1, 0};
    
    for (int iter = 0; iter < 200; ++iter) {
        for (int reg = 0; reg < 4; ++reg) {
            if (conditions[reg]) {
                /* Conditional update of low byte only */
                unsigned int new_low = (iter + reg) & 0xFF;
                registers[reg] = (registers[reg] & ~0xFF) | new_low;
            } else {
                /* Conditional update of low word only */
                unsigned int new_low_word = (iter * reg) & 0xFFFF;
                registers[reg] = (registers[reg] & ~0xFFFF) | new_low_word;
            }
            
            /* Toggle condition */
            conditions[reg] ^= 1;
        }
        
        /* Partial store through char pointer */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&registers[iter & 3];
        *byte_ptr = (unsigned char)(iter & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    /* Use results */
    volatile unsigned int total = 0;
    for (int i = 0; i < 4; ++i) {
        total += registers[i];
    }
    (void)total;
}

/* Test 6: Combined patterns with switch statement */
void test_combined_patterns(void) {
    volatile struct packed_bitfields bs = {0};
    volatile unsigned int control = 0;
    volatile unsigned int result = 0;
    
    /* Array to force memory operations */
    volatile unsigned int mem_array[16];
    for (int i = 0; i < 16; ++i) {
        mem_array[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 150; ++i) {
        /* Update bitfields */
        bs.field1 = (i >> 0) & 0x1F;
        bs.field2 = (i >> 5) & 0x7FF;
        bs.field3 = (i >> 16) & 0x7F;
        bs.field4 = (i >> 23) & 0x1FF;
        
        /* Extract and use in switch */
        unsigned int selector = bs.field1;
        
        switch (selector & 0x7) {
            case 0:
                /* ZERO_EXTRACT from memory */
                control = (mem_array[selector] >> 8) & 0xFF;
                break;
            case 1:
                /* STRICT_LOW_PART style update */
                control = (control & ~0xFF) | (selector & 0xFF);
                break;
            case 2:
                /* Another extraction */
                control = (mem_array[selector + 1] >> 16) & 0xFFFF;
                break;
            default:
                /* Mixed operation */
                control = ((control << 4) | selector) & 0xFFF;
                break;
        }
        
        /* Memory operation to trigger MEM_P path elsewhere */
        result ^= mem_array[i & 0xF];
        result += control;
        
        /* Update memory */
        mem_array[i & 0xF] = result;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int final = result + bs.field1 + bs.field2;
    (void)final;
}

/* Driver function */
int main(int argc, char *argv[]) {
    int test_to_run = -1;
    
    /* Parse command line argument */
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    volatile unsigned int total_result = 0;
    
    /* Run selected test or all tests */
    if (test_to_run == -1 || test_to_run == 1) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    if (test_to_run == -1 || test_to_run == 2) {
        test_zero_extract_struct();
        total_result += 2;
    }
    if (test_to_run == -1 || test_to_run == 3) {
        test_zero_extract_mixed();
        total_result += 3;
    }
    if (test_to_run == -1 || test_to_run == 4) {
        test_strict_low_part_asm();
        total_result += 4;
    }
    if (test_to_run == -1 || test_to_run == 5) {
        test_strict_low_part_conditional();
        total_result += 5;
    }
    if (test_to_run == -1 || test_to_run == 6) {
        test_combined_patterns();
        total_result += 6;
    }
    
    /* Print result to prevent optimization */
    printf("Test result indicator: %u\n", total_result);
    
    return 0;
}
