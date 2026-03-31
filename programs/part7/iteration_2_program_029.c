/* test_resource.cc - Generate RTL patterns for GCC resource.cc coverage */

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
static int generate_zero_extract(void) {
    /* Pattern 1: Using union and bitfields for ZERO_EXTRACT */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = g_volatile_int;
    int result1 = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual masking and shifting */
    unsigned int val = g_volatile_int;
    int result2 = (val >> 16) & 0xFFFF;  /* Alternative ZERO_EXTRACT pattern */
    
    /* Pattern 3: Extract multiple fields */
    int result3 = (val & 0xFF00) >> 8;
    int result4 = (val >> 24) & 0xFF;
    
    /* Use results to prevent elimination */
    use_int(result1 + result2 + result3 + result4);
    return result1;
}

/* Function 2: Generate STRICT_LOW_PART pattern */
__attribute__((noinline))
static void generate_strict_low_part(void) {
    /* Pattern 1: Modify low byte through pointer */
    int temp = g_volatile_int;
    char *byte_ptr = (char*)&temp;
    byte_ptr[0] = g_volatile_char;  /* Modify low byte only */
    use_int(temp);
    
    /* Pattern 2: Structure assignment for low part */
    struct {
        char low;
        char mid;
        short high;
    } s;
    
    s.low = g_volatile_char;  /* STRICT_LOW_PART candidate */
    s.mid = g_volatile_char + 1;
    s.high = g_volatile_short;
    
    /* Pattern 3: Union type punning */
    union {
        int full;
        struct {
            unsigned char b0;
            unsigned char b1;
            unsigned char b2;
            unsigned char b3;
        } bytes;
    } u;
    
    u.full = g_volatile_int;
    u.bytes.b0 = g_volatile_char;  /* Modify only low byte */
    use_int(u.full);
    
    /* Pattern 4: Bitfield assignment */
    struct {
        unsigned int low8: 8;
        unsigned int rest: 24;
    } bits;
    
    bits.low8 = g_volatile_char;  /* STRICT_LOW_PART for bitfield */
    bits.rest = g_volatile_int >> 8;
    use_int(*(int*)&bits);
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
static void generate_subreg(void) {
    /* Pattern 1: Type punning with different sizes */
    long long big_val = (long long)g_volatile_int * 1000LL;
    int small_part = (int)big_val;  /* SUBREG from larger to smaller */
    use_int(small_part);
    
    /* Pattern 2: Access through different pointer types */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = g_volatile_int + i;
    }
    
    /* Access as different types */
    short *short_ptr = (short*)array;
    short_ptr[g_volatile_index] = g_volatile_short;  /* SUBREG in memory access */
    
    /* Pattern 3: Complex pointer arithmetic */
    char *base = (char*)array;
    int offset = g_volatile_index * sizeof(int) / 2;
    short *ptr = (short*)(base + offset + 1);  /* Unaligned access */
    *ptr = g_volatile_short;
    
    /* Pattern 4: Register subreg simulation */
    volatile int reg_var = g_volatile_int;
    short half = (short)reg_var;  /* SUBREG extract low 16 bits */
    use_short(half);
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
static void generate_mem_complex(void) {
    /* Pattern 1: Array with volatile index */
    int buffer[100];
    volatile int idx = g_volatile_index;
    
    /* Complex addressing mode */
    buffer[idx * 4 + 2] = g_volatile_int;
    buffer[idx + 5] = buffer[idx * 3] + 1;
    
    /* Pattern 2: Pointer chasing */
    int *ptr1 = &buffer[10];
    int *ptr2 = &buffer[20];
    int *ptr3 = &buffer[30];
    
    *ptr1 = g_volatile_int;
    *ptr2 = *ptr1 + idx;
    *ptr3 = *ptr2 * 2;
    
    /* Pattern 3: Structure with pointer fields */
    struct node {
        int value;
        struct node *next;
    } nodes[5];
    
    for (int i = 0; i < 4; i++) {
        nodes[i].value = g_volatile_int + i;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[4].value = g_volatile_int;
    nodes[4].next = NULL;
    
    /* Traverse with memory accesses */
    struct node *current = &nodes[0];
    int sum = 0;
    while (current) {
        sum += current->value;  /* MEM_P with structure access */
        current = current->next;
    }
    use_int(sum);
    
    /* Pattern 4: Multi-dimensional array */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 10 + j + g_volatile_int;
        }
    }
    
    /* Complex indexed access */
    int diag_sum = 0;
    for (int i = 0; i < 5; i++) {
        diag_sum += matrix[i][i];  /* MEM_P with 2D indexing */
    }
    use_int(diag_sum);
}

/* Function 5: Combined patterns to trigger recursive marking */
__attribute__((noinline))
static int generate_combined_pattern(void) {
    /* Combined operation that might generate multiple RTL patterns */
    union {
        unsigned int full;
        struct {
            unsigned short low;
            unsigned short high;
        } parts;
    } data;
    
    data.full = g_volatile_int;
    
    /* ZERO_EXTRACT pattern */
    unsigned short extracted = (data.full >> 8) & 0xFFFF;
    
    /* STRICT_LOW_PART pattern */
    data.parts.low = g_volatile_short;  /* Modify low part only */
    
    /* SUBREG pattern through pointer */
    int array[8];
    short *short_view = (short*)array;
    short_view[g_volatile_index] = extracted;  /* SUBREG store */
    
    /* MEM_P with complex address */
    int *ptr = &array[g_volatile_index * 2 % 8];
    *ptr = data.full;
    
    /* Use results */
    use_int(data.full);
    use_short(extracted);
    use_ptr(ptr);
    
    return data.full + extracted + array[0];
}

/* Function 6: Control flow variation with patterns */
__attribute__((noinline))
static int generate_with_control_flow(void) {
    volatile int selector = g_volatile_int % 5;
    int result = 0;
    
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT in loop */
            unsigned int val = g_volatile_int;
            for (int i = 0; i < 4; i++) {
                result += (val >> (i * 8)) & 0xFF;
            }
            break;
        }
        case 1: {
            /* STRICT_LOW_PART with condition */
            int temp = g_volatile_int;
            if (temp > 0) {
                *(char*)&temp = g_volatile_char;  /* Modify low byte */
            }
            result = temp;
            break;
        }
        case 2: {
            /* SUBREG pattern */
            long long big = g_volatile_int * 100LL;
            int small[2];
            small[0] = (int)big;
            small[1] = (int)(big >> 32);
            result = small[0] + small[1];
            break;
        }
        case 3: {
            /* MEM_P with computed address */
            int buffer[10];
            for (int i = 0; i < 10; i++) {
                int idx = (i + g_volatile_index) % 10;
                buffer[idx] = g_volatile_int + i;
            }
            result = buffer[g_volatile_index % 10];
            break;
        }
        default: {
            /* Combined pattern */
            union {
                int i;
                short s[2];
            } u;
            u.i = g_volatile_int;
            u.s[0] = g_volatile_short;  /* STRICT_LOW_PART */
            result = u.i;
            break;
        }
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Exercise each pattern generator */
    checksum += generate_zero_extract();
    
    generate_strict_low_part();
    checksum += g_volatile_int;
    
    generate_subreg();
    checksum += g_volatile_short;
    
    generate_mem_complex();
    checksum += g_volatile_index;
    
    checksum += generate_combined_pattern();
    checksum += generate_with_control_flow();
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions for external functions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
