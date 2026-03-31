/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_result_int(int value);
extern void use_result_short(short value);
extern void use_result_char(char value);
extern void use_result_ptr(void *ptr);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile int volatile_mask = 0xFF00FF00;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline)) 
static int test_zero_extract(void) {
    /* Pattern 1: Using bitfield union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Explicit masking and shifting */
    uint32_t val = volatile_seed;
    int result2 = (val >> 16) & 0xFFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Pattern 3: Multiple extractions in control flow */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        uint32_t temp = volatile_seed + i;
        int extracted = (temp >> (i * 4)) & 0xF;
        total += extracted;
    }
    
    use_result_int(result1 + result2 + total);
    return result1 + result2;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static short test_strict_low_part(void) {
    /* Pattern 1: Structure with small members */
    struct {
        char low_byte;
        short low_word;
        int full;
    } s;
    
    s.full = volatile_seed;
    s.low_byte = (volatile_seed >> 8) & 0xFF;  /* STRICT_LOW_PART candidate */
    s.low_word = volatile_seed & 0xFFFF;       /* Another candidate */
    
    /* Pattern 2: Pointer to low part */
    int temp = volatile_seed;
    char *byte_ptr = (char *)&temp;
    byte_ptr[0] = (volatile_seed >> 16) & 0xFF;  /* Modifies only part */
    
    /* Pattern 3: Union type punning */
    union {
        int32_t i;
        int16_t s[2];
    } pun;
    
    pun.i = volatile_seed;
    pun.s[0] = (volatile_seed >> 8) & 0xFFFF;  /* Low part modification */
    
    use_result_short(s.low_word);
    use_result_char(s.low_byte);
    
    return s.low_word + pun.s[0];
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int test_subreg_mem(void) {
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Pattern 1: SUBREG through pointer arithmetic */
    volatile int idx = volatile_index;
    short *short_ptr = (short *)((char *)array + idx * sizeof(int));
    *short_ptr = volatile_seed & 0xFFFF;  /* MEM with SUBREG addressing */
    
    /* Pattern 2: Complex memory addressing */
    int *ptr = &array[volatile_index & 7];
    int **ptr_to_ptr = &ptr;
    
    /* Pattern 3: Nested memory accesses */
    struct {
        int data[4];
        struct {
            short a;
            short b;
        } nested;
    } complex;
    
    complex.data[2] = volatile_seed;
    complex.nested.a = (volatile_seed >> 8) & 0xFFFF;
    
    /* Access through different type */
    char *char_view = (char *)&complex;
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result += char_view[i * 2];
    }
    
    use_result_ptr(ptr_to_ptr);
    return result + *short_ptr + complex.data[2];
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int test_combined_patterns(void) {
    volatile int control = volatile_seed;
    int result = 0;
    
    /* Switch with different pattern generation */
    switch (control & 0x3) {
        case 0: {
            /* ZERO_EXTRACT + MEM combination */
            union {
                uint32_t full;
                struct {
                    uint32_t a: 4;
                    uint32_t b: 12;
                    uint32_t c: 16;
                } fields;
            } u;
            
            u.full = volatile_seed;
            int buffer[4];
            buffer[u.fields.a] = u.fields.c;  /* MEM with ZERO_EXTRACT index */
            result = buffer[1];
            break;
        }
            
        case 1: {
            /* STRICT_LOW_PART + SUBREG combination */
            struct {
                int32_t full;
                int16_t half[2];
            } s;
            
            s.full = volatile_seed;
            s.half[0] = (volatile_seed >> 8) & 0x7FFF;  /* STRICT_LOW_PART */
            
            /* Access as different type */
            int8_t *byte_view = (int8_t *)&s.half[0];
            result = byte_view[1];  /* SUBREG access */
            break;
        }
            
        case 2: {
            /* All three patterns combined */
            int data = volatile_seed;
            int *ptr = &data;
            
            /* Create SUBREG */
            short *short_ptr = (short *)ptr;
            
            /* Modify low part */
            *short_ptr = volatile_mask & 0xFFFF;  /* STRICT_LOW_PART */
            
            /* Extract bits */
            result = (data >> 16) & 0xFF;  /* ZERO_EXTRACT */
            break;
        }
            
        default: {
            /* Complex memory addressing */
            int arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = volatile_seed + i * 0x100;
            }
            
            volatile int offset = volatile_index;
            int *element = &arr[offset & 7];
            
            /* Multiple indirections */
            result = *(int *)((char *)element + (offset & 3));
            break;
        }
    }
    
    use_result_int(result);
    return result;
}

/* Function 5: Loop-based pattern generation */
__attribute__((noinline))
static int test_loop_patterns(void) {
    int total = 0;
    volatile int iterations = 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Alternate between patterns based on loop index */
        if (i & 1) {
            /* ZERO_EXTRACT in loop */
            uint32_t val = volatile_seed + i;
            int extracted = (val >> (i * 2)) & 0x3;
            total += extracted;
        } else {
            /* STRICT_LOW_PART in loop */
            struct {
                int32_t full;
                int16_t low;
            } s;
            
            s.full = volatile_seed;
            s.low = (volatile_seed + i) & 0xFFFF;
            total += s.low;
        }
        
        /* MEM access with computed address every 4 iterations */
        if ((i & 3) == 0) {
            int buffer[4];
            for (int j = 0; j < 4; j++) {
                buffer[j] = volatile_seed + j;
            }
            
            short *short_ptr = (short *)((char *)buffer + i);
            total += *short_ptr;
        }
    }
    
    use_result_int(total);
    return total;
}

/* Main function that exercises all patterns */
int main(void) {
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Initialize with truly volatile values */
    volatile_seed = 0x89ABCDEF;
    volatile_index = 5;
    volatile_mask = 0x00FF00FF;
    
    /* Execute all test functions */
    int sum = 0;
    
    sum += test_zero_extract();
    printf("After zero_extract: sum = %d\n", sum);
    
    sum += test_strict_low_part();
    printf("After strict_low_part: sum = %d\n", sum);
    
    sum += test_subreg_mem();
    printf("After subreg_mem: sum = %d\n", sum);
    
    sum += test_combined_patterns();
    printf("After combined_patterns: sum = %d\n", sum);
    
    sum += test_loop_patterns();
    printf("After loop_patterns: sum = %d\n", sum);
    
    /* Final checksum */
    printf("Final checksum: %d (0x%08X)\n", sum, sum);
    
    return sum != 0 ? 0 : 1;
}

/* Dummy definitions for external functions to satisfy linker */
void use_result_int(int value) {
    /* Empty - just to prevent optimization */
    asm volatile("" : : "r"(value));
}

void use_result_short(short value) {
    asm volatile("" : : "r"(value));
}

void use_result_char(char value) {
    asm volatile("" : : "r"(value));
}

void use_result_ptr(void *ptr) {
    asm volatile("" : : "r"(ptr));
}
