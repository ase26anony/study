/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile inputs to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Use union with bitfields to force ZERO_EXTRACT */
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
    
    /* Multiple extractions to increase chances */
    int result = 0;
    result += u.bits.low;          /* Should generate ZERO_EXTRACT */
    result += u.bits.middle << 4;  /* Another extraction with shift */
    result += u.bits.high << 8;    /* Third extraction */
    
    /* Use result to prevent dead code elimination */
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    /* Structure with small member to force partial register updates */
    struct {
        char low_byte;
        int full_word;
    } s;
    
    /* Initialize with volatile */
    int temp = volatile_seed;
    
    /* This assignment to low byte should generate STRICT_LOW_PART */
    s.low_byte = temp & 0xFF;
    
    /* Also try through pointer */
    char *ptr = &s.low_byte;
    *ptr = (temp >> 8) & 0xFF;
    
    /* Use the structure */
    s.full_word = temp;
    use_int(s.full_word);
    return s.low_byte;
}

/* Function 3: Generate SUBREG pattern */
__attribute__((noinline))
static int pattern_subreg(void) {
    /* Array with type punning to generate SUBREG accesses */
    int array[16];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    int result = 0;
    
    /* Access through different type pointers - should generate SUBREG */
    short *short_ptr = (short*)array;
    for (int i = 0; i < 8; i++) {
        result += short_ptr[i];  /* Accesses int as two shorts */
    }
    
    /* More complex: pointer arithmetic with char* */
    char *char_ptr = (char*)array;
    char_ptr += volatile_index;
    short *alias_ptr = (short*)char_ptr;  /* Misaligned access */
    result += *alias_ptr;
    
    use_int(result);
    return result;
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
static int pattern_mem_complex(void) {
    /* Multi-dimensional array for complex addressing */
    int matrix[8][8];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = volatile_seed + i * 8 + j;
        }
    }
    
    int result = 0;
    volatile int idx1 = volatile_index;
    volatile int idx2 = volatile_index + 1;
    
    /* Complex addressing mode */
    result += matrix[idx1][idx2];
    result += matrix[idx2][idx1];
    
    /* Pointer with offset */
    int *row = matrix[idx1];
    result += row[idx2 * 2];
    
    use_int(result);
    return result;
}

/* Function 5: Combined patterns in control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    int result = 0;
    
    /* Use volatile for control flow */
    volatile int control = volatile_seed & 0xF;
    
    /* Switch with different patterns in each case */
    switch (control) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            union {
                uint32_t val;
                struct {
                    uint32_t a: 3;
                    uint32_t b: 5;
                    uint32_t c: 8;
                    uint32_t d: 16;
                } fields;
            } u;
            u.val = volatile_seed;
            
            for (int i = 0; i < 4; i++) {
                switch (i) {
                    case 0: result += u.fields.a; break;
                    case 1: result += u.fields.b; break;
                    case 2: result += u.fields.c; break;
                    case 3: result += u.fields.d; break;
                }
            }
            break;
        }
        
        case 1: {
            /* STRICT_LOW_PART with pointer */
            struct {
                unsigned char a, b, c, d;
            } packed;
            
            int temp = volatile_seed;
            unsigned char *p = &packed.a;
            for (int i = 0; i < 4; i++) {
                p[i] = (temp >> (i * 8)) & 0xFF;
            }
            result = packed.a + packed.b + packed.c + packed.d;
            break;
        }
        
        case 2: {
            /* SUBREG and MEM combined */
            long buffer[4];
            for (int i = 0; i < 4; i++) {
                buffer[i] = volatile_seed + i;
            }
            
            /* Access as different types */
            int *as_int = (int*)buffer;
            short *as_short = (short*)buffer;
            
            result = as_int[1] + as_short[3];
            break;
        }
        
        default: {
            /* Complex memory addressing */
            int arr[10];
            for (int i = 0; i < 10; i++) {
                arr[i] = volatile_seed * i;
            }
            
            volatile int idx = volatile_index;
            int *ptr = arr + idx;
            result = ptr[0] + ptr[1] + ptr[2];
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Function 6: Nested patterns for deep recursion */
__attribute__((noinline))
static int pattern_nested(void) {
    /* Structure with bitfields inside union */
    typedef union {
        struct {
            struct {
                unsigned int low: 4;
                unsigned int high: 28;
            } inner;
        } outer;
        unsigned int full;
    } complex_t;
    
    complex_t c;
    c.full = volatile_seed;
    
    /* Extract and modify partial fields */
    unsigned int temp = c.outer.inner.low;
    c.outer.inner.high = (c.outer.inner.high & 0x0FFFFFFF) | (temp << 28);
    
    /* Access through memory with pointer arithmetic */
    complex_t array[2];
    array[0] = c;
    array[1].full = volatile_seed ^ 0xFFFFFFFF;
    
    /* Complex addressing */
    complex_t *ptr = array + (volatile_index & 1);
    result = ptr->outer.inner.low + ptr->outer.inner.high;
    
    use_int(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Call all pattern functions */
    checksum += pattern_zero_extract();
    checksum += pattern_strict_low_part();
    checksum += pattern_subreg();
    checksum += pattern_mem_complex();
    checksum += pattern_combined();
    checksum += pattern_nested();
    
    /* Print checksum to ensure execution */
    printf("Resource pattern checksum: %d\n", checksum);
    
    return 0;
}

/* External function definitions (weak) to satisfy linker */
#ifdef __GNUC__
__attribute__((weak))
#endif
void use_int(int x) {
    /* Do nothing - just to prevent optimization */
    volatile int dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_short(short x) {
    volatile short dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_ptr(void* x) {
    volatile void* dummy = x;
    (void)dummy;
}

#ifdef __GNUC__
__attribute__((weak))
#endif
void use_long(long x) {
    volatile long dummy = x;
    (void)dummy;
}
