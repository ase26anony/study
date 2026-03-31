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
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extracted = (source >> 4) & 0xFF;
        COMPILER_BARRIER();
        
        /* Extract bits 8-15 with variable shift */
        extracted |= ((source >> (i % 8)) & 0xFF) << 8;
        COMPILER_BARRIER();
        
        /* Extract bits 16-23 with mask */
        extracted |= (source & 0x00FF0000) >> 16;
        COMPILER_BARRIER();
        
        result += extracted;
    }
    
    /* Use result to prevent elimination */
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field1 : 5;
    unsigned int field2 : 11;
    unsigned int field3 : 7;
    unsigned int field4 : 9;
};

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field1 = 0x1F;
    s.field2 = 0x7FF;
    s.field3 = 0x7F;
    s.field4 = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int val1 = s.field1;
        unsigned int val2 = s.field2;
        unsigned int val3 = s.field3;
        unsigned int val4 = s.field4;
        
        COMPILER_BARRIER();
        
        /* Write back with transformation - may generate both read and write extracts */
        s.field1 = (val2 + i) & 0x1F;
        s.field2 = (val3 ^ val4) & 0x7FF;
        s.field3 = (val1 * 2) & 0x7F;
        s.field4 = (val2 >> 3) & 0x1FF;
        
        accumulator += val1 + val2 + val3 + val4;
    }
    
    volatile unsigned int sink = accumulator;
    (void)sink;
}

/* Test 3: Complex bitfield operations with arrays */
void test_bitfield_array_operations(void) {
    struct __attribute__((packed)) {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } arr[16];
    
    volatile unsigned int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        arr[i].a = i & 0x7;
        arr[i].b = (i * 3) & 0x1F;
        arr[i].c = (i * 7) & 0x3FF;
        arr[i].d = (i * 13) & 0x3FFF;
    }
    
    /* Nested loops with bitfield accesses */
    for (int outer = 0; outer < 10; ++outer) {
        for (int inner = 0; inner < 16; ++inner) {
            /* Multiple bitfield reads */
            unsigned int tmp = arr[inner].a;
            tmp += arr[inner].b << 3;
            tmp += arr[inner].c << 8;
            tmp += arr[inner].d << 18;
            
            COMPILER_BARRIER();
            
            /* Conditional bitfield write */
            if (tmp & 1) {
                arr[inner].a = (arr[inner].b + 1) & 0x7;
                arr[inner].c = (arr[inner].d >> 2) & 0x3FF;
            }
            
            sum += tmp;
        }
    }
    
    volatile unsigned int sink = sum;
    (void)sink;
}

/* Test 4: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0xDEADBEEF;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte only */
        if (i & 1) {
            /* This pattern may generate STRICT_LOW_PART */
            data = (data & ~0xFF) | ((i * 7) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i & 2) {
            /* Another potential STRICT_LOW_PART pattern */
            unsigned short low_word = (i * 13) & 0xFFFF;
            data = (data & ~0xFFFF) | low_word;
        }
        
        /* Merge operation preserving high bits */
        mask = (mask << 1) | 1;
        data = (data & ~0x00FF00) | (((i * 3) & 0xFF) << 8);
        
        result += data;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = result;
    (void)sink;
}

/* Test 5: Inline assembly for partial register updates */
void test_asm_partial_updates(void) {
    volatile unsigned int reg_var = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline asm that operates on partial register */
        /* May generate STRICT_LOW_PART in RTL representation */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg_var), "i" (0x0000FFFF)
        );
        
        /* Another asm pattern */
        asm volatile (
            "extr %0, %1, %2, #8\n\t"
            : "=r" (temp)
            : "r" (reg_var), "r" (i & 0x1F)
        );
        
        /* Update only low part via C code that may compile to STRICT_LOW_PART */
        reg_var = (reg_var & ~0xFF) | ((i * 11) & 0xFF);
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = reg_var + temp;
    (void)sink;
}

/* Test 6: Mixed operations with switch statement */
void test_mixed_with_switch(void) {
    volatile unsigned int base = 0xABCD1234;
    volatile unsigned int output = 0;
    
    struct __attribute__((packed)) {
        unsigned int opcode : 4;
        unsigned int operand : 12;
        unsigned int flags : 4;
    } instruction;
    
    for (int i = 0; i < 100; ++i) {
        /* Create instruction from bits of base */
        instruction.opcode = (base >> 0) & 0xF;
        instruction.operand = (base >> 4) & 0xFFF;
        instruction.flags = (base >> 16) & 0xF;
        
        /* Switch on bitfield value - creates control flow */
        switch (instruction.opcode) {
            case 0:
                /* ZERO_EXTRACT pattern */
                output = (base >> instruction.operand) & ((1U << instruction.flags) - 1);
                break;
            case 1:
                /* STRICT_LOW_PART-like update */
                base = (base & ~0xFFF) | (instruction.operand & 0xFFF);
                break;
            case 2:
                /* Mixed operation */
                output = instruction.operand + instruction.flags;
                base = (base & ~0xF0000) | ((output & 0xF) << 16);
                break;
            default:
                /* Simple extraction */
                output = instruction.operand;
                break;
        }
        
        /* Rotate base */
        base = (base << 1) | (base >> 31);
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = output + base;
    (void)sink;
}

/* Test 7: Pointer-based partial updates */
void test_pointer_partial_updates(void) {
    volatile unsigned int data_array[32];
    volatile unsigned char *byte_ptr;
    volatile unsigned short *short_ptr;
    
    /* Initialize array */
    for (int i = 0; i < 32; ++i) {
        data_array[i] = i * 0x01010101;
    }
    
    unsigned int sum = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Update through byte pointer - partial store */
        byte_ptr = (volatile unsigned char *)&data_array[i % 32];
        byte_ptr[1] = (i * 3) & 0xFF;  /* Update only one byte */
        
        /* Update through short pointer */
        short_ptr = (volatile unsigned short *)&data_array[(i + 1) % 32];
        short_ptr[0] = (i * 7) & 0xFFFF;  /* Update low 16 bits */
        
        /* Extract byte via pointer */
        sum += byte_ptr[2];
        sum += short_ptr[1];
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink = sum;
    (void)sink;
}

/* Main driver */
int main(int argc, char *argv[]) {
    unsigned int test_mask = 0xFF;  /* Run all tests by default */
    
    /* Parse command line for specific tests */
    if (argc > 1) {
        test_mask = atoi(argv[1]);
    }
    
    volatile unsigned int total = 0;
    
    if (test_mask & 0x01) test_bitfield_extract_volatile();
    if (test_mask & 0x02) test_packed_struct_bitfields();
    if (test_mask & 0x04) test_bitfield_array_operations();
    if (test_mask & 0x08) test_strict_low_part_conditional();
    if (test_mask & 0x10) test_asm_partial_updates();
    if (test_mask & 0x20) test_mixed_with_switch();
    if (test_mask & 0x40) test_pointer_partial_updates();
    
    /* Additional loop to ensure scheduling happens */
    volatile unsigned int counter = 0;
    for (int i = 0; i < 1000; ++i) {
        counter += i;
        COMPILER_BARRIER();
    }
    
    total += counter;
    
    /* Print something to prevent dead code elimination */
    printf("Result: %u\n", total);
    
    return 0;
}
