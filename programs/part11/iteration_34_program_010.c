/* test_resource.c - Generate RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (byte 1) */
        unsigned int extracted = (source >> 8) & 0xFF;
        result = extracted + i;  /* Prevent optimization */
        COMPILER_BARRIER();
    }
    
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) bitfield_struct {
    unsigned int header : 4;
    unsigned int data   : 12;
    unsigned int footer : 16;
};

void test_zero_extract_struct(void) {
    volatile struct bitfield_struct bs = {0};
    volatile unsigned int sum = 0;
    
    /* Initialize with pattern */
    bs.header = 0xA;
    bs.data   = 0xABC;
    bs.footer = 0xDEAD;
    
    /* Multiple bitfield accesses that should generate ZERO_EXTRACT */
    for (int i = 0; i < 50; ++i) {
        unsigned int val1 = bs.header;  /* ZERO_EXTRACT expected */
        unsigned int val2 = bs.data;    /* ZERO_EXTRACT expected */
        unsigned int val3 = bs.footer;  /* ZERO_EXTRACT expected */
        
        /* Complex operation to prevent optimization */
        bs.data = (bs.data + val1) & 0xFFF;
        sum += val1 + val2 + val3 + i;
        COMPILER_BARRIER();
    }
    
    printf("Test2 sum: %u\n", sum);
}

/* Test 3: Bitfield extraction with arithmetic */
void test_zero_extract_arithmetic(void) {
    volatile unsigned int buffer[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    volatile unsigned int output[4] = {0};
    
    /* Complex bitfield manipulation in nested loops */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 4; ++j) {
            /* Extract various bit ranges */
            unsigned int low4  = buffer[j] & 0xF;
            unsigned int mid8  = (buffer[j] >> 8) & 0xFF;
            unsigned int high8 = (buffer[j] >> 24) & 0xFF;
            
            /* Combine with arithmetic */
            output[j] = (low4 * mid8) | (high8 << 16);
            
            /* Update buffer with rotated bits */
            buffer[j] = (buffer[j] >> 4) | (buffer[j] << 28);
        }
        COMPILER_BARRIER();
    }
    
    printf("Test3 output[0]: %08X\n", output[0]);
}

/* Test 4: STRICT_LOW_PART via conditional narrow store */
void test_strict_low_part_conditional(void) {
    volatile unsigned int reg = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Pattern: (reg & ~mask) | (new_val & mask) */
            unsigned char new_byte = i & 0xFF;
            reg = (reg & ~0xFF) | (new_byte & 0xFF);
        }
        
        /* Alternate between setting and clearing flag */
        flag = !flag;
        COMPILER_BARRIER();
    }
    
    printf("Test4 reg: %08X\n", reg);
}

/* Test 5: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that modifies only low bits */
        unsigned int temp = value;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (temp), "i" (0xFFFF)  /* Only keep low 16 bits */
        );
        
        result += temp;
        value = (value << 1) | (value >> 31);  /* Rotate */
        COMPILER_BARRIER();
    }
    
    printf("Test5 result: %08X\n", result);
}

/* Test 6: Mixed operations with memory references */
void test_mixed_memory_ops(void) {
    volatile unsigned int array[16];
    volatile struct bitfield_struct bs_array[4];
    volatile unsigned int total = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 4; ++i) {
        bs_array[i].header = i;
        bs_array[i].data = i * 0x111;
        bs_array[i].footer = i * 0x1111;
    }
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 20; ++i) {
        /* Memory access */
        unsigned int mem_val = array[i % 16];
        
        /* Bitfield extraction */
        unsigned int bf_val = bs_array[i % 4].data;
        
        /* Partial store */
        if (mem_val & 1) {
            array[i % 16] = (array[i % 16] & ~0xFFFF) | (bf_val & 0xFFFF);
        }
        
        /* Switch based on bitfield */
        switch (bs_array[i % 4].header & 0x3) {
            case 0: total += mem_val; break;
            case 1: total += bf_val; break;
            case 2: total += i; break;
            case 3: total += mem_val + bf_val; break;
        }
        
        COMPILER_BARRIER();
    }
    
    printf("Test6 total: %08X\n", total);
}

/* Test 7: Register variables with bitfield operations */
void test_register_vars(void) {
    register unsigned int r1 asm("r12") = 0x55555555;
    register unsigned int r2 asm("r13") = 0xAAAAAAAA;
    volatile unsigned int output = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Extract and combine bits from register variables */
        unsigned int low_bits = r1 & 0xFF;      /* May use ZERO_EXTRACT */
        unsigned int high_bits = (r2 >> 24) & 0xFF; /* May use ZERO_EXTRACT */
        
        /* Partial update */
        r1 = (r1 & ~0xFF00) | ((low_bits + high_bits) << 8);
        
        output += r1 + r2;
        
        /* Rotate values */
        r1 = (r1 << 1) | (r1 >> 31);
        r2 = (r2 >> 1) | (r2 << 31);
        
        COMPILER_BARRIER();
    }
    
    printf("Test7 output: %08X\n", output);
}

/* Main driver */
int main(int argc, char *argv[]) {
    int run_all = 0;
    int test_num = 0;
    
    /* Parse command line */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_num = atoi(argv[1]);
        }
    } else {
        run_all = 1;  /* Default: run all tests */
    }
    
    volatile unsigned int final_sum = 0;
    
    if (run_all || test_num == 1) {
        test_zero_extract_volatile();
        final_sum += 1;
    }
    
    if (run_all || test_num == 2) {
        test_zero_extract_struct();
        final_sum += 2;
    }
    
    if (run_all || test_num == 3) {
        test_zero_extract_arithmetic();
        final_sum += 3;
    }
    
    if (run_all || test_num == 4) {
        test_strict_low_part_conditional();
        final_sum += 4;
    }
    
    if (run_all || test_num == 5) {
        test_strict_low_part_asm();
        final_sum += 5;
    }
    
    if (run_all || test_num == 6) {
        test_mixed_memory_ops();
        final_sum += 6;
    }
    
    if (run_all || test_num == 7) {
        test_register_vars();
        final_sum += 7;
    }
    
    /* Print final aggregate to prevent dead code elimination */
    printf("Final aggregate: %u\n", final_sum);
    
    return 0;
}
