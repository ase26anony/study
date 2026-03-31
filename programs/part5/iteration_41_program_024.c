/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */
struct BitFieldStruct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function 1: Bit-field extraction from structure */
unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int iterations) {
    unsigned int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* These operations should generate ZERO_EXTRACT RTL */
        sum += s->field1;                    /* Simple bit-field read */
        sum += (s->field2 << 3) & 0x7F;      /* Bit-field with shift and mask */
        sum += (s->field3 == 512) ? 1 : 0;   /* Bit-field comparison */
        sum += (s->field4 & 0x3F) >> 2;      /* Combined mask and shift */
        
        /* Modify structure to prevent optimization */
        s->field1 = (s->field1 + 1) & 0x1F;
        s->field2 = (s->field2 * 3) & 0x7F;
        s->field3 = (s->field3 + i) & 0x3FF;
        s->field4 = (s->field4 ^ i) & 0x3FF;
    }
    return sum;
}

/* Function 2: Explicit bit-field operations on integers */
unsigned int test_zero_extract_integer(unsigned int *arr, int size) {
    unsigned int sum = 0;
    for (int i = 0; i < size; i++) {
        /* These should generate ZERO_EXTRACT for bit-field operations */
        unsigned int val = arr[i];
        
        /* Extract various bit ranges */
        sum += (val >> 8) & 0xFF;           /* Extract byte 1 */
        sum += (val >> 16) & 0x3FF;         /* Extract 10-bit field */
        sum += (val & 0x1F);                /* Extract lower 5 bits */
        sum += ((val >> 20) & 0x7) << 2;    /* Extract and shift */
        
        /* Complex bit-field expression */
        unsigned int masked = val & 0xFF00FF00;
        sum += (masked >> 8) & 0xFF00FF;
        
        /* Prevent dead code elimination */
        arr[i] = val ^ sum;
    }
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with small types */
unsigned int test_strict_low_part(short *short_arr, char *char_arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* These assignments should generate STRICT_LOW_PART RTL */
        short s = short_arr[i];
        char c = char_arr[i];
        
        /* Partial register updates */
        s = (s + i) & 0xFFFF;           /* Keep within 16 bits */
        c = (c * 3) & 0xFF;             /* Keep within 8 bits */
        
        /* Mix with larger types to force partial writes */
        int temp_int = s;               /* Promote to int */
        temp_int = temp_int + c;        /* Arithmetic in int register */
        s = temp_int & 0xFFFF;          /* Write back only low 16 bits - STRICT_LOW_PART */
        
        /* Volatile pointer to force partial store */
        volatile short *vs = &short_arr[i];
        *vs = s;                        /* Should generate STRICT_LOW_PART store */
        
        /* Update char array with partial write */
        char_arr[i] = (temp_int >> 8) & 0xFF;
        
        sum += s + c;
    }
    
    return sum;
}

/* Function 4: Inline assembly for partial register access */
unsigned int test_strict_low_part_asm(int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        unsigned short us;
        unsigned char uc;
        
        /* Inline assembly that operates on partial registers */
        __asm__ volatile (
            "movw %1, %0\n\t"
            : "=r"(us)
            : "r"(i & 0xFFFF)
        );
        
        __asm__ volatile (
            "movb %b1, %0\n\t"
            : "=q"(uc)
            : "r"(i & 0xFF)
            : "cc"
        );
        
        sum += us + uc;
    }
    
    return sum;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union type-punning for SUBREG generation */
unsigned int test_subreg_union(int *data, int size) {
    union PunningUnion {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } u;
    
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        u.full = data[i];
        
        /* Access sub-parts - should generate SUBREG RTL */
        sum += u.halves[0];            /* Low 16 bits */
        sum += u.halves[1];            /* High 16 bits */
        sum += u.bytes[1] << 8;        /* Byte access with shift */
        
        /* Modify and write back */
        u.halves[0] = (u.halves[0] + i) & 0xFFFF;
        u.halves[1] ^= u.bytes[2];
        
        data[i] = u.full;              /* Write back full register */
    }
    
    return sum;
}

/* Function 6: Casting between different integer sizes */
unsigned int test_subreg_casting(long long *ll_arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Casting between different sizes - should generate SUBREG */
        long long ll_val = ll_arr[i];
        
        /* Extract parts through casting */
        int low_part = (int)(ll_val & 0xFFFFFFFF);
        int high_part = (int)((ll_val >> 32) & 0xFFFFFFFF);
        short first_short = (short)(low_part & 0xFFFF);
        short second_short = (short)((low_part >> 16) & 0xFFFF);
        
        /* Operations on sub-parts */
        sum += first_short;
        sum += second_short * 2;
        sum += high_part & 0x7FFF;
        
        /* Modify and reassemble */
        low_part = (low_part ^ sum) & 0xFFFFFFFF;
        ll_val = ((long long)high_part << 32) | (low_part & 0xFFFFFFFFLL);
        ll_arr[i] = ll_val;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */
/* Function 7: Memory references with addressing modes */
unsigned int test_complex_memory(struct BitFieldStruct *struct_arr,
                                 short *short_arr,
                                 int *int_arr,
                                 int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing modes */
        int idx = (i * g_volatile_seed) % size;
        
        /* Memory access with index calculation */
        sum += struct_arr[idx].field2;
        sum += short_arr[idx * 2] + short_arr[idx * 2 + 1];
        
        /* Pointer arithmetic with type punning */
        char *byte_ptr = (char *)&int_arr[idx];
        sum += byte_ptr[0] + (byte_ptr[1] << 8);
        
        /* Update memory locations */
        struct_arr[idx].field1 = (sum >> 3) & 0x1F;
        short_arr[idx] = (short)(sum & 0xFFFF);
        int_arr[idx] ^= sum;
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    /* Use command line or volatile to prevent constant folding */
    int iterations = g_volatile_seed;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int array_size = iterations % 100 + 50;
    
    /* Initialize test data */
    struct BitFieldStruct *bf_array = 
        (struct BitFieldStruct *)malloc(array_size * sizeof(struct BitFieldStruct));
    unsigned int *int_array = (unsigned int *)malloc(array_size * sizeof(unsigned int));
    short *short_array = (short *)malloc(array_size * sizeof(short));
    char *char_array = (char *)malloc(array_size);
    long long *ll_array = (long long *)malloc(array_size * sizeof(long long));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < array_size; i++) {
        bf_array[i].field1 = (i * 3) & 0x1F;
        bf_array[i].field2 = (i * 5) & 0x7F;
        bf_array[i].field3 = (i * 7) & 0x3FF;
        bf_array[i].field4 = (i * 11) & 0x3FF;
        
        int_array[i] = (i * 13) ^ 0x12345678;
        short_array[i] = (short)((i * 17) & 0xFFFF);
        char_array[i] = (char)((i * 19) & 0xFF);
        ll_array[i] = ((long long)i << 32) | (i * 23);
    }
    
    unsigned int total_sum = 0;
    
    /* Execute all test patterns */
    total_sum += test_zero_extract_struct(&bf_array[0], iterations % 10 + 5);
    total_sum += test_zero_extract_integer(int_array, array_size);
    total_sum += test_strict_low_part(short_array, char_array, array_size);
    total_sum += test_strict_low_part_asm(iterations % 20 + 10);
    total_sum += test_subreg_union(int_array, array_size);
    total_sum += test_subreg_casting(ll_array, array_size);
    total_sum += test_complex_memory(bf_array, short_array, int_array, array_size);
    
    /* Clean up */
    free(bf_array);
    free(int_array);
    free(short_array);
    free(char_array);
    free(ll_array);
    
    /* Return checksum to prevent dead code elimination */
    printf("Total checksum: %u\n", total_sum);
    return (int)(total_sum & 0x7FFFFFFF);
}
