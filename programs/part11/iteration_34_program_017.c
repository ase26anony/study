/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0x12345678;
    volatile unsigned int result = 0;
    
    /* Loop to increase scheduling opportunities */
    for (int i = 0; i < 100; ++i) {
        /* Multiple bitfield extractions - may generate ZERO_EXTRACT */
        unsigned int bits_5_9 = (source >> 5) & 0x1F;  /* 5 bits starting at bit 5 */
        unsigned int bits_10_20 = (source >> 10) & 0x7FF; /* 11 bits starting at bit 10 */
        
        /* Combine and store to volatile to prevent elimination */
        result = bits_5_9 + bits_10_20;
        
        /* Modify source to create variation */
        source = (source * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    COMPILER_BARRIER();
    printf("Test1 result: %u\n", result);
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) PackedBitfield {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

void test_packed_struct_bitfields(void) {
    volatile struct PackedBitfield s = {0};
    volatile unsigned int results[4] = {0};
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Bitfield reads may generate ZERO_EXTRACT */
        unsigned int a = s.field_a;
        unsigned int b = s.field_b;
        unsigned int c = s.field_c;
        unsigned int d = s.field_d;
        
        /* Complex bitfield assignment - may generate both ZERO_EXTRACT and STRICT_LOW_PART */
        s.field_b = (s.field_a + s.field_c) & 0x7FF;
        s.field_d = (s.field_b ^ s.field_c) & 0x1FF;
        
        /* Store results */
        results[0] += a;
        results[1] += b;
        results[2] += c;
        results[3] += d;
        
        /* Rotate values */
        unsigned int temp = s.field_a;
        s.field_a = s.field_c;
        s.field_c = s.field_d;
        s.field_d = s.field_b;
        s.field_b = temp;
    }
    
    COMPILER_BARRIER();
    printf("Test2 results: %u %u %u %u\n", 
           results[0], results[1], results[2], results[3]);
}

/* Test 3: Inline assembly for partial register updates */
void test_asm_partial_store(void) {
    volatile unsigned int value = 0x12345678;
    volatile unsigned int mask = 0x000000FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that might generate STRICT_LOW_PART */
        unsigned int low_byte;
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (low_byte)
            : "r" (value), "r" (mask)
        );
        
        /* Conditional partial update - may generate STRICT_LOW_PART */
        if (low_byte > 128) {
            /* Update only low byte while preserving high bytes */
            value = (value & ~0xFF) | ((low_byte + i) & 0xFF);
        }
        
        /* Another pattern: merge operation */
        unsigned int new_low = (i * 13) & 0xFF;
        value = (value & ~0xFF) | new_low;
        
        COMPILER_BARRIER();
    }
    
    printf("Test3 value: 0x%08x\n", value);
}

/* Test 4: Pointer-based partial updates and memory operations */
void test_pointer_partial_updates(void) {
    volatile unsigned int data[16];
    volatile unsigned char *byte_ptr;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        data[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 100; ++i) {
        /* Cast to char pointer for byte access - may generate partial store RTL */
        byte_ptr = (volatile unsigned char *)&data[i % 16];
        byte_ptr[0] = i & 0xFF;        /* Update low byte */
        byte_ptr[1] = (i >> 8) & 0xFF; /* Update second byte */
        
        /* Bitfield extraction from memory */
        unsigned int element = data[i % 16];
        unsigned int bits_4_8 = (element >> 4) & 0x1F;
        unsigned int bits_12_16 = (element >> 12) & 0x1F;
        
        /* Conditional partial update based on extracted bits */
        if (bits_4_8 > bits_12_16) {
            /* Update only bits 8-15 */
            data[(i + 1) % 16] = (data[(i + 1) % 16] & ~0xFF00) | 
                                 ((bits_4_8 << 8) & 0xFF00);
        }
        
        COMPILER_BARRIER();
    }
    
    printf("Test4 data[0]: 0x%08x\n", data[0]);
}

/* Test 5: Complex mixed operations with switch statement */
void test_mixed_operations(void) {
    volatile unsigned int counters[8] = {0};
    volatile struct {
        unsigned int status : 4;
        unsigned int command : 4;
        unsigned int data : 24;
    } __attribute__((packed)) device_reg;
    
    device_reg.status = 0;
    device_reg.command = 0;
    device_reg.data = 0x00ABCDEF;
    
    for (int i = 0; i < 200; ++i) {
        /* Extract bitfield for switch control */
        unsigned int opcode = device_reg.status & 0x07;
        
        /* Switch on bitfield value - creates control flow */
        switch (opcode) {
            case 0:
                /* ZERO_EXTRACT pattern */
                device_reg.data = (device_reg.data >> 8) & 0xFFFF;
                break;
            case 1:
                /* Partial update pattern */
                device_reg.data = (device_reg.data & ~0xFF) | (i & 0xFF);
                break;
            case 2:
                /* Complex bitfield operation */
                device_reg.command = (device_reg.status + device_reg.command) & 0x0F;
                break;
            case 3:
                /* Memory operation with bitfield */
                counters[device_reg.status & 0x07] += 
                    (device_reg.data >> 16) & 0xFF;
                break;
            default:
                /* Mixed operation */
                device_reg.data ^= 0x00FF00FF;
                device_reg.status = (device_reg.status + 1) & 0x0F;
                break;
        }
        
        /* Rotate register values */
        unsigned int temp = device_reg.status;
        device_reg.status = device_reg.command;
        device_reg.command = temp & 0x0F;
        device_reg.data = (device_reg.data << 8) | (device_reg.data >> 16);
        
        COMPILER_BARRIER();
    }
    
    printf("Test5 status: %u, command: %u, data: 0x%08x\n",
           device_reg.status, device_reg.command, device_reg.data);
}

/* Main driver function */
int main(int argc, char *argv[]) {
    int run_all = 0;
    int test_num = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_num = atoi(argv[1]);
        }
    } else {
        run_all = 1; /* Default: run all tests */
    }
    
    volatile unsigned int final_result = 0;
    
    if (run_all || test_num == 1) {
        test_bitfield_extract_volatile();
        final_result += 1;
    }
    
    if (run_all || test_num == 2) {
        test_packed_struct_bitfields();
        final_result += 2;
    }
    
    if (run_all || test_num == 3) {
        test_asm_partial_store();
        final_result += 3;
    }
    
    if (run_all || test_num == 4) {
        test_pointer_partial_updates();
        final_result += 4;
    }
    
    if (run_all || test_num == 5) {
        test_mixed_operations();
        final_result += 5;
    }
    
    /* Print aggregate result to prevent dead code elimination */
    printf("Final aggregate: %u\n", final_result);
    
    return 0;
}
