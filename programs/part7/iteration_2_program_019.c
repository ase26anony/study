/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void *);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
int zero_extract_pattern(void) {
    /* Use union with bitfields to potentially generate ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int middle: 12;
            unsigned int high: 12;
        } bits;
    } u;
    
    /* Volatile assignment prevents constant propagation */
    u.full = volatile_seed;
    
    /* Multiple extraction patterns */
    int result1 = u.bits.middle;        /* Should generate ZERO_EXTRACT */
    int result2 = u.bits.high;          /* Another ZERO_EXTRACT */
    
    /* Manual extraction that might also generate ZERO_EXTRACT */
    int result3 = (u.full >> 4) & 0xFFF;
    
    /* Use results to prevent dead code elimination */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
int strict_low_part_pattern(void) {
    int temp = volatile_seed;
    
    /* Structure with small member to generate partial register store */
    struct {
        char low_byte;
        short low_word;
        int full;
    } s;
    
    /* These assignments might generate STRICT_LOW_PART */
    s.low_byte = temp & 0xFF;           /* Modify only low 8 bits */
    s.low_word = temp & 0xFFFF;         /* Modify only low 16 bits */
    s.full = temp;                      /* Full assignment for contrast */
    
    /* Pointer-based partial modification */
    char *byte_ptr = (char *)&temp;
    byte_ptr[1] = volatile_char;        /* Modify specific byte */
    
    /* Use results */
    use_int(s.full);
    use_short(s.low_word);
    
    return s.low_byte + s.low_word;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
int subreg_mem_pattern(void) {
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    int result = 0;
    
    /* Complex memory addressing with type punning */
    volatile int idx = volatile_index;
    
    /* Access through different pointer types - may generate SUBREG */
    short *short_ptr = (short *)((char *)array + idx * sizeof(int));
    *short_ptr = volatile_short;        /* Store short into int array */
    
    /* Access with pointer arithmetic */
    int *int_ptr = array + idx;
    int_ptr[0] = int_ptr[1] + volatile_seed;
    
    /* Nested memory access with address computation */
    int **ptr_array[4];
    int data[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = (int **)&data[i];
    }
    
    /* Complex memory expression */
    result = **(ptr_array[idx % 4]);
    
    /* Use memory results */
    use_int(array[0]);
    use_int(result);
    use_ptr(ptr_array[0]);
    
    return result + array[0];
}

/* Function 4: Combined pattern with control flow */
__attribute__((noinline))
int combined_pattern_with_cf(void) {
    int result = 0;
    volatile int control = volatile_seed & 0xF;
    
    /* Switch statement with different patterns in each case */
    switch (control) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 3;
                    uint32_t b: 5;
                    uint32_t c: 24;
                } fields;
            } u;
            u.val = volatile_seed;
            
            for (int i = 0; i < 4; i++) {
                result += u.fields.b << i;
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with conditional */
            struct {
                unsigned char a;
                unsigned short b;
            } s;
            
            int temp = volatile_seed;
            if (temp & 1) {
                s.a = temp & 0x7F;
            } else {
                s.b = temp & 0x7FFF;
            }
            result = s.a + s.b;
            break;
        }
        
        case 2: {
            /* SUBREG and MEM with pointer chase */
            int buffer[8];
            for (int i = 0; i < 8; i++) {
                buffer[i] = volatile_seed + i * 2;
            }
            
            char *char_ptr = (char *)buffer;
            int offset = volatile_index * sizeof(int);
            short *aliased = (short *)(char_ptr + offset);
            
            result = *aliased;
            break;
        }
        
        default: {
            /* Mixed pattern */
            union {
                long l;
                int parts[2];
            } ul;
            ul.l = volatile_seed;
            
            ul.parts[0] &= 0xFFFF;  /* STRICT_LOW_PART on array element */
            result = ul.parts[1] >> 16;  /* ZERO_EXTRACT */
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Function 5: Nested patterns in loop */
__attribute__((noinline))
int loop_nested_patterns(void) {
    int sum = 0;
    volatile int iterations = (volatile_seed & 0x7) + 1;
    
    /* Mixed array for various access patterns */
    unsigned char byte_array[32];
    int int_array[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        byte_array[i] = (volatile_seed + i) & 0xFF;
    }
    for (int i = 0; i < 8; i++) {
        int_array[i] = volatile_seed * i;
    }
    
    /* Loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT from byte array */
        unsigned int packed = (byte_array[i*2] << 8) | byte_array[i*2 + 1];
        int extracted = (packed >> 4) & 0xFFF;
        
        /* STRICT_LOW_PART store */
        int_array[i % 8] = extracted & 0xFF;
        
        /* MEM access with computed address */
        int *ptr = &int_array[(i + volatile_index) % 8];
        *ptr += extracted;
        
        /* SUBREG-like access through different type */
        short *short_view = (short *)ptr;
        sum += short_view[0];
    }
    
    use_int(sum);
    return sum;
}

/* Main function that calls all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Call each pattern function */
    checksum ^= zero_extract_pattern();
    checksum ^= strict_low_part_pattern();
    checksum ^= subreg_mem_pattern();
    checksum ^= combined_pattern_with_cf();
    checksum ^= loop_nested_patterns();
    
    /* Additional volatile operations to prevent optimization */
    checksum += volatile_seed;
    checksum -= volatile_index;
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum & 0xFF;
}

/* Dummy definitions for external functions (in same file for testing) */
void use_int(int x) { volatile int dummy = x; (void)dummy; }
void use_short(short x) { volatile short dummy = x; (void)dummy; }
void use_ptr(void *x) { volatile void *dummy = x; (void)dummy; }
void use_long(long x) { volatile long dummy = x; (void)dummy; }
