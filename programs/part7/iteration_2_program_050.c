/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
volatile int volatile_input = 42;
volatile int volatile_index = 0;
volatile short volatile_short = 100;
volatile char volatile_char = 7;

/* Pattern 1: ZERO_EXTRACT through bitfield union */
__attribute__((noinline))
int pattern_zero_extract(void) {
    /* Union with bitfields to generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = volatile_input;
    
    /* Multiple extractions to increase chances */
    unsigned int result1 = u.bits.low;
    unsigned int result2 = u.bits.middle;
    unsigned int result3 = u.bits.high;
    
    /* Complex extraction pattern */
    unsigned int masked = (u.full >> 4) & 0xFFF;
    
    /* Use results to prevent elimination */
    use_int(result1 + result2 + result3 + masked);
    
    return result1;
}

/* Pattern 2: STRICT_LOW_PART through partial structure assignment */
__attribute__((noinline))
int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        int full_word;
    } s;
    
    struct {
        short low_half;
        short high_half;
    } s2;
    
    /* Initialize with volatile to prevent constant propagation */
    int temp = volatile_input;
    short temp_short = volatile_short;
    
    /* These assignments should generate STRICT_LOW_PART */
    s.low_byte = temp & 0xFF;
    s2.low_half = temp_short;
    
    /* More complex low-part assignment */
    int* ptr = &s.full_word;
    *ptr = temp;
    
    /* Use results */
    use_int(s.low_byte);
    use_short(s2.low_half);
    
    return s.low_byte + s2.low_half;
}

/* Pattern 3: SUBREG and MEM_P with complex addressing */
__attribute__((noinline))
int pattern_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    short short_array[32];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        array[i] = i * volatile_input;
    }
    
    /* Complex pointer arithmetic for SUBREG generation */
    volatile int idx = volatile_index;
    
    /* SUBREG pattern 1: Access through different type */
    short* short_ptr = (short*)((char*)array + idx);
    *short_ptr = volatile_short;
    
    /* SUBREG pattern 2: Nested subreg access */
    char* char_ptr = (char*)array + idx * 2;
    int* int_ptr = (int*)(char_ptr + 1);
    *int_ptr = volatile_input;
    
    /* Complex MEM_P with address computation */
    int (*array_ptr)[4] = (int (*)[4])&array[idx % 12];
    int value = (*array_ptr)[2];
    
    /* Use results */
    use_int(value);
    use_ptr(short_ptr);
    
    return value + *short_ptr;
}

/* Pattern 4: Combined patterns in control flow */
__attribute__((noinline))
int pattern_combined(void) {
    int result = 0;
    volatile int control = volatile_input;
    
    /* Loop with varying patterns */
    for (int i = 0; i < 4; i++) {
        if (control & (1 << i)) {
            /* ZERO_EXTRACT in conditional */
            union {
                unsigned long long full;
                struct {
                    unsigned int low;
                    unsigned int high;
                } parts;
            } u;
            
            u.full = (unsigned long long)control << 32 | i;
            result += u.parts.low;
            
            /* STRICT_LOW_PART in loop */
            struct {
                signed char a;
                signed char b;
            } chars;
            
            chars.a = (control + i) & 0x7F;
            chars.b = (control - i) & 0x7F;
            
            result += chars.a + chars.b;
        } else {
            /* SUBREG/MEM in else branch */
            int buffer[8];
            short* sp = (short*)&buffer[i % 4];
            *sp = volatile_short + i;
            result += *sp;
        }
    }
    
    /* Switch with different patterns */
    switch (control & 3) {
        case 0: {
            /* ZERO_EXTRACT in switch */
            union {
                int val;
                struct {
                    unsigned int a:3;
                    unsigned int b:5;
                    unsigned int c:24;
                } bits;
            } u;
            u.val = volatile_input;
            result += u.bits.b;
            break;
        }
        case 1: {
            /* STRICT_LOW_PART in switch */
            struct {
                int x;
                char c[4];
            } s;
            s.c[1] = volatile_char;
            result += s.c[1];
            break;
        }
        case 2: {
            /* SUBREG/MEM in switch */
            long long big_array[4];
            int* ip = (int*)((char*)big_array + 2);
            *ip = volatile_input;
            result += *ip;
            break;
        }
        default:
            result += volatile_input;
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Nested patterns for deep recursion */
__attribute__((noinline))
int pattern_nested(void) {
    /* Complex structure with bitfields */
    struct {
        union {
            struct {
                unsigned int a:4;
                unsigned int b:4;
                unsigned int c:24;
            } bits;
            unsigned int full;
        } u;
        
        struct {
            char low;
            char high;
        } bytes;
        
        int* ptr;
    } complex;
    
    complex.u.full = volatile_input;
    complex.bytes.low = volatile_char;
    
    /* Array with complex indexing */
    int data[10];
    complex.ptr = &data[volatile_index % 8];
    
    /* Nested access patterns */
    short* short_view = (short*)&complex.u.full;
    *short_view = volatile_short;
    
    /* Memory access through computed pointer */
    char* byte_ptr = (char*)complex.ptr + (complex.u.bits.a * 2);
    int* int_ptr = (int*)byte_ptr;
    *int_ptr = complex.u.full;
    
    use_int(complex.u.bits.c);
    use_ptr(complex.ptr);
    
    return complex.u.bits.a + complex.bytes.low + *int_ptr;
}

/* Main function to execute all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation...\n");
    
    /* Execute all patterns */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg_mem();
    checksum += pattern_combined();
    checksum += pattern_nested();
    
    /* Add volatile dependency */
    checksum += volatile_input;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy implementations of external functions */
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_short(short x) {
    volatile static short sink;
    sink = x;
}

void use_ptr(void* x) {
    volatile static void* sink;
    sink = x;
}

void use_long(long x) {
    volatile static long sink;
    sink = x;
}
