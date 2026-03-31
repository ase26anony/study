/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int g_volatile_int = 0x12345678;
static volatile short g_volatile_short = 0xABCD;
static volatile char g_volatile_char = 0x42;
static volatile int g_volatile_index = 3;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Use union with bitfields to generate ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low_bits: 8;
            uint32_t middle_bits: 12;
            uint32_t high_bits: 12;
        } bits;
    } extractor;
    
    /* Volatile input prevents constant folding */
    extractor.full = g_volatile_int;
    
    /* These operations should generate ZERO_EXTRACT RTL */
    int result1 = extractor.bits.low_bits;
    int result2 = extractor.bits.middle_bits;
    int result3 = extractor.bits.high_bits;
    
    /* Use results to prevent dead code elimination */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Structure with small integer type for low part access */
    struct partial_reg {
        unsigned char low_byte;
        unsigned char pad[3];
    } __attribute__((packed));
    
    struct partial_reg reg;
    volatile int input = g_volatile_int;
    
    /* This assignment should generate STRICT_LOW_PART RTL */
    reg.low_byte = input & 0xFF;
    
    /* Additional pattern: modify only low 16 bits */
    volatile short* ptr_short = (volatile short*)&input;
    *ptr_short = g_volatile_short;
    
    /* Complex pattern with control flow */
    if (g_volatile_char & 1) {
        /* Modify low byte through pointer */
        volatile char* ptr_char = (volatile char*)&input;
        ptr_char[1] = g_volatile_char;
    }
    
    use_int(input);
    return reg.low_byte;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    volatile int index = g_volatile_index & 0xF;
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = g_volatile_int + i;
    }
    
    /* SUBREG pattern: access memory through different type */
    short* short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = g_volatile_short;  /* Should generate SUBREG RTL */
    
    /* Complex MEM_P pattern with address computation */
    int* computed_ptr = array + (index ^ 0x3);
    *computed_ptr = g_volatile_int >> 4;
    
    /* Nested memory access with pointer arithmetic */
    char* char_ptr = (char*)array;
    for (int i = 0; i < 8; i++) {
        char_ptr[index + i] = g_volatile_char + i;
    }
    
    /* Use pointer to prevent optimization */
    use_ptr(array);
    
    return array[index] + array[(index + 1) & 0xF];
}

/* Function 4: Combined pattern with all operations */
__attribute__((noinline))
static int pattern_combined(void) {
    /* Combined structure for multiple patterns */
    union {
        uint32_t value;
        struct {
            uint16_t low_half;
            uint16_t high_half;
        } halves;
        struct {
            uint8_t byte0;
            uint8_t byte1;
            uint8_t byte2;
            uint8_t byte3;
        } bytes;
    } data;
    
    data.value = g_volatile_int;
    
    /* ZERO_EXTRACT pattern */
    int extracted = (data.value >> 8) & 0x3FF;  /* Middle 10 bits */
    
    /* STRICT_LOW_PART pattern through pointer */
    volatile uint16_t* low_ptr = (volatile uint16_t*)&data.value;
    *low_ptr = g_volatile_short;  /* Modify only low 16 bits */
    
    /* Memory access with SUBREG */
    int buffer[8];
    for (int i = 0; i < 8; i++) {
        buffer[i] = g_volatile_int + i * 0x100;
    }
    
    /* Access through different-sized pointer */
    short* subreg_ptr = (short*)&buffer[g_volatile_index & 0x7];
    *subreg_ptr = extracted & 0xFFFF;
    
    /* Complex control flow */
    switch (g_volatile_char & 0x3) {
        case 0:
            data.bytes.byte1 = g_volatile_char;
            break;
        case 1:
            data.halves.high_half = g_volatile_short;
            break;
        case 2:
            /* Nested memory access */
            ((char*)buffer)[g_volatile_index] = data.bytes.byte2;
            break;
        default:
            data.value = buffer[g_volatile_index & 0x3];
            break;
    }
    
    use_int(data.value);
    use_int(buffer[0]);
    
    return data.value + buffer[0] + extracted;
}

/* Function 5: Pattern with loops and conditionals */
__attribute__((noinline))
static int pattern_with_control_flow(void) {
    volatile int counter = g_volatile_int & 0xF;
    int result = 0;
    
    /* Loop with memory access pattern */
    int temp_array[10];
    for (int i = 0; i < 10; i++) {
        temp_array[i] = g_volatile_int + i;
    }
    
    while (counter-- > 0) {
        /* Mixed patterns inside loop */
        union {
            int full;
            short parts[2];
        } u;
        
        u.full = temp_array[counter & 0x9];
        
        /* ZERO_EXTRACT-like access */
        int low_part = u.full & 0xFF;
        
        /* STRICT_LOW_PART-like modification */
        u.parts[0] = g_volatile_short + counter;
        
        /* SUBREG memory access */
        char* byte_ptr = (char*)temp_array;
        byte_ptr[counter] = low_part;
        
        result += u.full;
    }
    
    /* Conditional with different patterns */
    if (g_volatile_char > 0x40) {
        /* Access through different pointer types */
        int* int_ptr = temp_array;
        short* short_ptr = (short*)int_ptr;
        short_ptr[1] = result & 0xFFFF;  /* SUBREG store */
    } else {
        /* Bitfield extraction */
        struct {
            unsigned int a: 5;
            unsigned int b: 11;
            unsigned int c: 16;
        } bitfield;
        
        bitfield.a = g_volatile_char & 0x1F;
        bitfield.b = (result >> 5) & 0x7FF;
        bitfield.c = g_volatile_short;
        
        result = bitfield.a + bitfield.b + bitfield.c;
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Execute all pattern functions */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_with_control_flow();
    
    /* Use volatile to ensure all computations are kept */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Pattern generation complete.\n");
    
    return final_result & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
