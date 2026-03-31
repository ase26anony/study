/* test_resource.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_result(int);
extern void use_result_ptr(void*);
extern void use_result_long(long);

/* Volatile inputs to prevent constant folding */
volatile int volatile_input = 0x12345678;
volatile int volatile_index = 2;
volatile short volatile_short = 0xABCD;
volatile char volatile_char = 0x42;

/* Pattern 1: ZERO_EXTRACT through bitfield unions */
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
    
    /* Combine with arithmetic to keep values alive */
    int combined = (result1 << 16) | (result2 << 8) | result3;
    use_result(combined);
    
    return combined;
}

/* Pattern 2: STRICT_LOW_PART through partial assignments */
__attribute__((noinline))
int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        char mid_byte;
        int full_word;
    } data;
    
    int temp = volatile_input;
    
    /* These should generate STRICT_LOW_PART for byte assignments */
    data.low_byte = temp & 0xFF;           /* Low 8 bits */
    data.mid_byte = (temp >> 8) & 0xFF;    /* Next 8 bits */
    data.full_word = temp;                 /* Full word */
    
    /* Force memory operations */
    use_result_ptr(&data);
    
    return data.low_byte + data.mid_byte;
}

/* Pattern 3: SUBREG through type punning */
__attribute__((noinline))
int pattern_subreg(void) {
    int array[16];
    volatile int idx = volatile_index;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 0x11111111;
    }
    
    /* Access through different pointer types - should generate SUBREG */
    short *short_ptr = (short *)((char *)array + idx * sizeof(int));
    *short_ptr = volatile_short;
    
    /* Another SUBREG pattern */
    char *char_ptr = (char *)array + 8;
    *char_ptr = volatile_char;
    
    /* Complex addressing mode */
    int *int_ptr = (int *)((char *)array + idx);
    int result = *int_ptr;
    
    use_result(result);
    return result;
}

/* Pattern 4: MEM_P with complex addressing */
__attribute__((noinline))
int pattern_mem_complex(void) {
    struct {
        int data[4];
        long extra;
    } buffer;
    
    volatile int offset = volatile_index;
    
    /* Complex memory addressing */
    int *ptr1 = &buffer.data[offset];
    int *ptr2 = ptr1 + 1;
    
    *ptr1 = volatile_input;
    *ptr2 = volatile_input >> 16;
    
    /* Pointer arithmetic creating complex MEM addresses */
    long *long_ptr = (long *)((char *)&buffer + offset * 2);
    *long_ptr = (long)volatile_input << 32 | volatile_input;
    
    use_result_long(*long_ptr);
    return *ptr1 + *ptr2;
}

/* Pattern 5: Combined patterns in control flow */
__attribute__((noinline))
int pattern_combined(void) {
    int result = 0;
    volatile int selector = volatile_input & 0x3;
    
    /* Switch with different patterns in each case */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a:4;
                    uint32_t b:4;
                    uint32_t c:24;
                } fields;
            } u;
            
            u.val = volatile_input;
            for (int i = 0; i < 4; i++) {
                result += u.fields.a << i;
                result += u.fields.b >> i;
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with conditionals */
            struct {
                unsigned char bytes[4];
                int word;
            } s;
            
            int temp = volatile_input;
            if (temp & 1) {
                s.bytes[0] = temp & 0xFF;
            }
            if (temp & 2) {
                s.bytes[1] = (temp >> 8) & 0xFF;
            }
            s.word = temp;
            result = s.bytes[0] + s.bytes[1];
            break;
        }
        
        case 2: {
            /* SUBREG and MEM in nested loops */
            int matrix[3][3];
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    short *sp = (short *)&matrix[i][j];
                    *sp = volatile_short + i + j;
                    result += matrix[i][j];
                }
            }
            break;
        }
        
        default: {
            /* Complex addressing computation */
            int arr[10];
            volatile int idx = volatile_index;
            int *p = (int *)((char *)arr + idx * sizeof(int) + 1);
            *p = volatile_input;
            result = *p;
            break;
        }
    }
    
    use_result(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg();
    checksum ^= pattern_mem_complex();
    checksum ^= pattern_combined();
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile int final = checksum;
    printf("Final checksum: 0x%08X\n", final);
    
    return final & 0xFF;
}

/* Dummy external function definitions (link separately) */
#ifdef COMPILE_WITH_DUMMIES
void use_result(int x) { (void)x; }
void use_result_ptr(void *p) { (void)p; }
void use_result_long(long l) { (void)l; }
#endif
