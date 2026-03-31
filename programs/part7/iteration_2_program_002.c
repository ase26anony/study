/* test_resource.c - Generate specific RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_result(int);
extern void use_result_long(long);
extern void use_result_ptr(void*);

/* Volatile variables to prevent constant folding */
volatile int g_volatile_int = 0;
volatile long g_volatile_long = 0;
volatile int g_index = 0;
volatile char g_volatile_char = 0;

/* Function 1: Generate ZERO_EXTRACT pattern */
__attribute__((noinline))
static int generate_zero_extract(void) {
    /* Pattern 1: Union with bitfields for ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:8;
            unsigned int high:16;
        } bits;
    } u;
    
    u.full = g_volatile_int;
    
    /* Multiple extractions to increase chances */
    unsigned int extracted1 = u.bits.low;
    unsigned int extracted2 = u.bits.middle;
    unsigned int extracted3 = u.bits.high;
    
    /* Combine with arithmetic to prevent dead code */
    int result = (extracted1 * 3) + (extracted2 * 5) + (extracted3 * 7);
    
    /* Pattern 2: Manual bitfield extraction with masking */
    unsigned int mask = 0xFF00;
    unsigned int shift = 8;
    unsigned int manual_extract = (g_volatile_int & mask) >> shift;
    
    result += manual_extract * 11;
    
    /* Use external function to keep computation alive */
    use_result(result);
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static int generate_strict_low_part(void) {
    /* Pattern 1: Structure with small integer type */
    struct {
        char low_byte;
        char second_byte;
        short low_word;
    } s;
    
    int temp = g_volatile_int;
    
    /* Multiple low-part assignments */
    s.low_byte = temp & 0xFF;
    s.second_byte = (temp >> 8) & 0xFF;
    s.low_word = temp & 0xFFFF;
    
    /* Pattern 2: Pointer-based low-part assignment */
    long big_temp = g_volatile_long;
    char *byte_ptr = (char*)&big_temp;
    byte_ptr[0] = g_volatile_char;  /* Modify only low byte */
    
    /* Pattern 3: Union-based low-part modification */
    union {
        long full;
        struct {
            int low;
            int high;
        } parts;
    } u;
    
    u.full = g_volatile_long;
    u.parts.low = g_volatile_int & 0x00FFFFFF;
    
    /* Combine results */
    int result = s.low_byte + s.second_byte + s.low_word + byte_ptr[0] + u.parts.low;
    
    use_result(result);
    
    return result;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int generate_subreg_mem(void) {
    int array[32];
    volatile int* volatile_ptr = &g_volatile_int;
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 32; i++) {
        array[i] = g_volatile_int + i;
    }
    
    /* Pattern 1: Complex memory addressing with type punning */
    int index = g_index % 28;
    
    /* Access through different pointer types (SUBREG generation) */
    short* short_ptr = (short*)((char*)array + index);
    *short_ptr = g_volatile_int & 0xFFFF;
    
    /* Pattern 2: Nested memory accesses */
    int** ptr_to_ptr = (int**)&volatile_ptr;
    int value_from_ptr = **ptr_to_ptr;
    
    /* Pattern 3: Array access with computed index (complex MEM address) */
    int complex_index = (g_volatile_int * 3 + 7) % 32;
    array[complex_index] = value_from_ptr;
    
    /* Pattern 4: Structure field access through pointer */
    struct {
        int a;
        int b;
        short c;
        char d;
    } data;
    
    data.a = g_volatile_int;
    data.b = g_volatile_int * 2;
    data.c = g_volatile_int & 0xFFFF;
    data.d = g_volatile_int & 0xFF;
    
    /* Access through byte pointer (forces SUBREG) */
    char* data_bytes = (char*)&data;
    int byte_sum = 0;
    for (int i = 0; i < sizeof(data); i++) {
        byte_sum += data_bytes[i];
    }
    
    /* Combine results */
    int result = *short_ptr + value_from_ptr + array[complex_index] + byte_sum;
    
    use_result(result);
    use_result_ptr(array);
    
    return result;
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int generate_combined_patterns(void) {
    int result = 0;
    
    /* Complex control flow with volatile condition */
    if (g_volatile_int & 1) {
        /* ZERO_EXTRACT in loop */
        for (int i = 0; i < 4; i++) {
            union {
                unsigned int val;
                struct {
                    unsigned int a:3;
                    unsigned int b:5;
                    unsigned int c:24;
                } fields;
            } u;
            
            u.val = g_volatile_int + i;
            result += u.fields.a * i;
            result += u.fields.b * (i + 1);
            
            /* STRICT_LOW_PART in nested conditional */
            if (u.fields.c & 0xFF) {
                struct {
                    char low;
                    char high;
                } s;
                s.low = u.fields.c & 0xFF;
                result += s.low;
            }
        }
    } else {
        /* SUBREG and MEM patterns in switch */
        switch (g_volatile_int & 3) {
            case 0: {
                int buffer[16];
                short* alias = (short*)buffer;
                alias[g_index % 8] = g_volatile_int;
                result += alias[0];
                break;
            }
            case 1: {
                long big_array[8];
                int* int_view = (int*)big_array;
                int_view[g_index % 4] = g_volatile_int;
                result += int_view[0];
                break;
            }
            case 2: {
                /* Mixed pattern */
                union {
                    long full;
                    int halves[2];
                } u;
                u.full = g_volatile_long;
                u.halves[0] = g_volatile_int;  /* STRICT_LOW_PART of long */
                result += u.halves[1];
                break;
            }
            default:
                result = g_volatile_int;
        }
    }
    
    /* Complex memory access with pointer arithmetic */
    char char_array[64];
    for (int i = 0; i < 64; i++) {
        char_array[i] = (g_volatile_int + i) & 0xFF;
    }
    
    int* int_alias = (int*)(char_array + g_index % 60);
    result += *int_alias;
    
    use_result(result);
    
    return result;
}

/* Main function to execute all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting RTL pattern generation...\n");
    
    /* Execute all pattern generators */
    checksum += generate_zero_extract();
    checksum += generate_strict_low_part();
    checksum += generate_subreg_mem();
    checksum += generate_combined_patterns();
    
    /* Additional volatile operations to prevent optimization */
    checksum += g_volatile_int;
    checksum += g_volatile_long;
    checksum += g_index;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy external function definitions to satisfy linker */
void use_result(int val) {
    /* Empty - just to prevent optimization */
    (void)val;
}

void use_result_long(long val) {
    (void)val;
}

void use_result_ptr(void* ptr) {
    (void)ptr;
}
