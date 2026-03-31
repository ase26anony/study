/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

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
        /* Extract bits 8-15 (width=8, start=8) */
        unsigned int mask = (1U << 8) - 1;
        result = (source >> 8) & mask;
        
        /* Extract bits 4-11 with varying width */
        unsigned int width = (i % 7) + 1;
        mask = (1U << width) - 1;
        result += (source >> 4) & mask;
        
        /* Update source to create data dependencies */
        source = (source * 13 + 17) & 0xFFFFFFFF;
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
void test_zero_extract_struct(void) {
    struct __attribute__((packed)) {
        unsigned int field1 : 5;
        unsigned int field2 : 11;
        unsigned int field3 : 7;
        unsigned int field4 : 9;
    } data;
    
    volatile unsigned int *ptr = (volatile unsigned int*)&data;
    *ptr = 0x12345678;
    
    volatile int sum = 0;
    
    /* Multiple bitfield accesses that should generate ZERO_EXTRACT */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - these become ZERO_EXTRACT */
        sum += data.field1;
        sum += data.field2;
        sum += data.field3;
        
        /* Write to bitfields - complex pattern */
        data.field2 = (data.field1 + i) & 0x7FF;
        data.field3 = (data.field2 >> 3) & 0x7F;
        
        /* Nested loop for scheduling complexity */
        for (int j = 0; j < 10; ++j) {
            data.field4 = (data.field3 + j) & 0x1FF;
        }
    }
    
    COMPILER_BARRIER();
    printf("Test2 sum: %d\n", sum);
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int temp;
    
    for (int i = 0; i < 100; ++i) {
        volatile int condition = (i % 3) == 0;
        
        if (condition) {
            /* This pattern may generate STRICT_LOW_PART:
               Only update low 8 bits, preserve high bits */
            value = (value & ~0xFF) | ((i * 7) & 0xFF);
        } else if ((i % 5) == 0) {
            /* Update low 16 bits only */
            value = (value & ~0xFFFF) | ((i * 13) & 0xFFFF);
        }
        
        /* Mix with memory operations */
        volatile unsigned int array[4] = {1, 2, 3, 4};
        temp = array[i % 4];
        
        /* Another partial update pattern */
        unsigned int mask = (1U << ((i % 8) + 4)) - 1;
        value = (value & ~mask) | (temp & mask);
    }
    
    COMPILER_BARRIER();
    printf("Test3 value: 0x%08x\n", value);
}

/* Test 4: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    register unsigned int r1 asm("r8") = 0x11111111;
    register unsigned int r2 asm("r9") = 0x22222222;
    volatile unsigned int output;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline assembly that operates on partial registers */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (output)
            : "r" (r1), "i" (0x0000FFFF), "r" (r2 & 0xFFFF0000)
            : /* No clobbers */
        );
        
        /* Update registers */
        r1 = (r1 * 3 + i) & 0xFFFFFFFF;
        r2 = (r2 * 5 + i * 2) & 0xFFFFFFFF;
        
        /* Memory operations mixed in */
        volatile unsigned int mem[8];
        for (int j = 0; j < 8; ++j) {
            mem[j] = (i * j) & 0xFF;
        }
        
        /* Another partial update using C */
        output = (output & ~0xFF00) | ((i << 8) & 0xFF00);
    }
    
    COMPILER_BARRIER();
    printf("Test4 output: 0x%08x\n", output);
}

/* Test 5: Combined patterns with switch statement */
void test_combined_patterns(void) {
    volatile unsigned int base = 0xABCD1234;
    volatile int result = 0;
    
    /* Packed struct with bitfields */
    struct __attribute__((packed)) {
        unsigned int flags : 4;
        unsigned int data : 20;
        unsigned int tag : 8;
    } item;
    
    volatile unsigned int *item_ptr = (volatile unsigned int*)&item;
    *item_ptr = base;
    
    for (int i = 0; i < 75; ++i) {
        /* Switch on bitfield value - creates control flow */
        switch (item.flags & 0x7) {
            case 0:
                /* ZERO_EXTRACT from struct */
                result += item.data;
                /* Partial update */
                base = (base & ~0xF) | (i & 0xF);
                break;
            case 1:
                /* Another extraction pattern */
                result += (base >> item.tag) & ((1U << 4) - 1);
                /* STRICT_LOW_PART-like update */
                if (i % 2) {
                    base = (base & ~0xFF00) | ((i << 8) & 0xFF00);
                }
                break;
            case 2:
                /* Complex bitfield manipulation */
                item.data = (item.data + base) & 0xFFFFF;
                item.tag = (item.tag ^ i) & 0xFF;
                break;
            default:
                /* Mixed operations */
                result += (base >> 16) & 0xFF;
                base = (base & 0xFFFF0000) | (i & 0xFFFF);
                break;
        }
        
        /* Update struct */
        item.flags = (item.flags + 1) & 0xF;
        
        /* Array access for memory operations */
        volatile unsigned int buffer[16];
        for (int j = 0; j < 16; ++j) {
            buffer[j] = (i * j + result) & 0xFF;
        }
        
        /* Extract from array element */
        result += (buffer[i % 16] >> 4) & 0xF;
    }
    
    COMPILER_BARRIER();
    printf("Test5 result: %d, base: 0x%08x\n", result, base);
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
        run_all = 1;
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
        test_strict_low_part_conditional();
        final_sum += 3;
    }
    
    if (run_all || test_num == 4) {
        test_strict_low_part_asm();
        final_sum += 4;
    }
    
    if (run_all || test_num == 5) {
        test_combined_patterns();
        final_sum += 5;
    }
    
    /* Prevent dead code elimination */
    printf("Final indicator: %u\n", final_sum);
    
    return 0;
}
