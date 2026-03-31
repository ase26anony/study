#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_seed = 42;

/* Function to get unpredictable values */
int get_value(int base) {
    return base + g_seed;
}

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
int test_zero_extract_int(int x) {
    int sum = 0;
    
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    for (int i = 0; i < 4; i++) {
        /* Extract different bit fields */
        int field1 = (x >> (i * 3)) & 0x7;      /* 3-bit field */
        int field2 = (x >> (8 + i * 2)) & 0x3;  /* 2-bit field */
        int field3 = (x >> (16 + i)) & 0x1;     /* 1-bit field */
        
        sum += field1 + field2 + field3;
    }
    
    /* Extract byte from word */
    int byte0 = (x & 0xFF);
    int byte1 = (x & 0xFF00) >> 8;
    int byte2 = (x & 0xFF0000) >> 16;
    int byte3 = (x & 0xFF000000) >> 24;
    
    sum += byte0 + byte1 + byte2 + byte3;
    
    return sum;
}

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int a : 5;
    unsigned int b : 7;
    unsigned int c : 3;
    unsigned int d : 17;
};

int test_zero_extract_struct(struct BitFieldStruct *s) {
    int sum = 0;
    
    /* Bit-field comparisons and assignments */
    if (s->a == 10) sum += 1;
    if (s->b > 20) sum += 2;
    if (s->c != 0) sum += 4;
    
    /* Bit-field arithmetic */
    unsigned int temp = s->a + s->b;
    sum += (temp & 0x1F);  /* Extract lower 5 bits */
    
    /* Nested bit-field extraction */
    unsigned int combined = (s->a << 16) | (s->b << 8) | s->c;
    sum += (combined >> 4) & 0xFFF;  /* Extract middle 12 bits */
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

int test_strict_low_part(void) {
    volatile int result = 0;
    
    /* Partial register updates with different sizes */
    for (int i = 0; i < 100; i++) {
        /* These should generate STRICT_LOW_PART for partial writes */
        short s = (short)(get_value(i) & 0xFFFF);
        char c = (char)(get_value(i * 2) & 0xFF);
        
        /* Force register allocation and partial updates */
        int temp = s * 2;
        s = (short)(temp & 0xFFFF);  /* Partial write back */
        
        temp = c + 1;
        c = (char)(temp & 0xFF);     /* Another partial write */
        
        result += s + c;
    }
    
    /* Pointer to volatile short - may generate partial store */
    volatile short *ps = (volatile short *)&result;
    *ps = (short)(get_value(100) & 0xFFFF);
    
    /* Array of small types */
    char buffer[64];
    for (int i = 0; i < 64; i++) {
        buffer[i] = (char)(get_value(i) & 0xFF);
        result += buffer[i];
    }
    
    return result;
}

/* Inline assembly for byte register access */
void test_strict_low_part_asm(void) {
    unsigned int value = get_value(50);
    unsigned char result;
    
    /* Force byte register operation */
    __asm__ volatile (
        "movb %1, %0\n\t"
        : "=q" (result)
        : "r" ((unsigned char)(value & 0xFF))
        : "cc"
    );
    
    g_seed += result;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union TypePun {
    uint32_t word;
    uint16_t half[2];
    uint8_t byte[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

int test_subreg_union(void) {
    union TypePun u;
    int sum = 0;
    
    for (int i = 0; i < 10; i++) {
        u.word = get_value(i * 100);
        
        /* Access different views of the same register */
        sum += u.half[0];      /* Low 16 bits */
        sum += u.half[1];      /* High 16 bits */
        sum += u.byte[1];      /* Second byte */
        sum += u.parts.low;    /* Another view of low half */
        
        /* Cast between types */
        uint16_t temp16 = (uint16_t)(u.word & 0xFFFF);
        uint8_t temp8 = (uint8_t)(u.word >> 8);
        
        sum += temp16 + temp8;
    }
    
    return sum;
}

/* Packed structure */
struct __attribute__((packed)) PackedStruct {
    uint16_t a;
    uint8_t b;
    uint16_t c;
    uint8_t d;
};

int test_subreg_packed(struct PackedStruct *ps) {
    int sum = 0;
    
    /* Access packed fields - may require SUBREG for misaligned access */
    sum += ps->a;
    sum += ps->b;
    sum += ps->c;
    sum += ps->d;
    
    /* Create SUBREG through pointer arithmetic */
    uint8_t *ptr = (uint8_t *)ps;
    uint16_t combined = (ptr[2] << 8) | ptr[3];  /* May involve SUBREG */
    
    return sum + combined;
}

/* ========== Memory references with complex addresses ========== */

int test_complex_memory(void) {
    int array[256];
    int sum = 0;
    
    /* Initialize array with unpredictable values */
    for (int i = 0; i < 256; i++) {
        array[i] = get_value(i);
    }
    
    /* Complex addressing modes */
    for (int i = 0; i < 100; i++) {
        int idx = get_value(i) & 0xFF;
        
        /* Array access with index calculation */
        sum += array[idx];
        sum += array[idx + 1];
        sum += array[idx * 2 % 256];
        
        /* Pointer arithmetic */
        int *ptr = &array[idx];
        sum += ptr[0] + ptr[1] + ptr[2];
        
        /* Structure pointer with offset */
        struct BitFieldStruct *s = (struct BitFieldStruct *)&array[idx];
        sum += test_zero_extract_struct(s);
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(void) {
    int total = 0;
    
    /* Initialize test data */
    struct BitFieldStruct bfs = {10, 50, 2, 1000};
    struct PackedStruct ps = {100, 50, 200, 25};
    
    int base_value = get_value(0);
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    total += test_zero_extract_int(base_value);
    total += test_zero_extract_struct(&bfs);
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    total += test_strict_low_part();
    test_strict_low_part_asm();
    
    /* Test SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    total += test_subreg_union();
    total += test_subreg_packed(&ps);
    
    /* Test complex memory references */
    printf("Testing complex memory references...\n");
    total += test_complex_memory();
    
    /* Mix all patterns together */
    printf("Mixing all patterns...\n");
    for (int i = 0; i < 10; i++) {
        int val = get_value(i);
        
        /* Combined operations */
        struct BitFieldStruct local_bfs = {
            .a = (val >> 0) & 0x1F,
            .b = (val >> 5) & 0x7F,
            .c = (val >> 12) & 0x7,
            .d = (val >> 15) & 0x1FFFF
        };
        
        total += test_zero_extract_struct(&local_bfs);
        
        /* Partial update in loop */
        short partial = (short)(total & 0xFFFF);
        total = (total & 0xFFFF0000) | partial;
        
        /* Union type-punning */
        union TypePun u;
        u.word = total;
        total += u.half[0] + u.byte[1];
    }
    
    printf("Final checksum: %d\n", total);
    
    /* Return non-zero to indicate success */
    return total != 0 ? 0 : 1;
}
