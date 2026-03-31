/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing -o coverage_test coverage_test.c */
/* Also try: gcc -O3 -fschedule-insns -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 20;   /* 20-bit field */
    unsigned int d : 1;    /* 1-bit field */
} S;

/* Force memory addressing modes with noinline function */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = x & 0xF;        /* Should generate ZERO_EXTRACT for 4-bit field */
    s->b = y & 0xFF;       /* Should generate ZERO_EXTRACT for 8-bit field */
    s->c = (x + y) & 0xFFFFF; /* 20-bit field */
    s->d = (x ^ y) & 0x1;  /* 1-bit field */
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}

/* Another noinline function to create SUBREG patterns */
__attribute__((noinline, optimize("O0")))
int mixed_width_operations(short *shorts, char *chars, int count) {
    int sum = 0;
    volatile int temp;  /* Prevent optimizations */
    
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns by mixing widths */
        shorts[i] = (short)(i * 37);  /* int to short: potential SUBREG */
        
        /* char to int with sign extension */
        chars[i] = (char)(i * 13);
        temp = chars[i];  /* Load char, should generate SUBREG */
        
        /* Mixed-width arithmetic */
        sum += shorts[i] * temp;  /* short * int */
        
        /* More SUBREG patterns */
        if (i & 1) {
            /* Cast through different types */
            unsigned char uc = (unsigned char)sum;
            sum = uc + shorts[i % count];
        }
    }
    
    return sum;
}

/* Complex addressing with 2D array */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    int result = 0;
    
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access that may combine with bit operations */
    result = arr[i][j] & 0x3FF;  /* Could generate ZERO_EXTRACT */
    
    /* Nested access with different index */
    j = (j + 1) % 100;
    result ^= arr[j][i];
    
    return result;
}

int main(int argc, char **argv) {
    int checksum = 0;
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&S, argc, argc * 2);
    
    /* Use the bit-fields in computation */
    checksum += S.a + S.b * 256 + S.c;
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_array[50];
    volatile char char_array[50];
    
    /* Use argc for non-constant loop bound */
    int loop_count = (argc > 1) ? atoi(argv[1]) % 50 : 10;
    if (loop_count <= 0) loop_count = 10;
    
    checksum += mixed_width_operations((short*)short_array, (char*)char_array, loop_count);
    
    /* 3. Complex 2D array addressing with volatile indices */
    int arr[100][100];
    volatile int idx1 = argc * 17;
    volatile int idx2 = argc * 23;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    checksum += complex_addressing(arr, &idx1, &idx2);
    
    /* 4. Additional register pressure with inline assembly */
    /* Clobber multiple registers to force reload pass */
    asm volatile(
        "# Force register clobber\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        : 
        : "r" (checksum), "r" (loop_count)
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* 5. More bit-field manipulation */
    struct BitFieldStruct local_s;
    
    /* Take address and manipulate */
    struct BitFieldStruct *ptr = &local_s;
    ptr->a = checksum & 0xF;
    ptr->b = (checksum >> 4) & 0xFF;
    ptr->c = checksum & 0xFFFFF;
    
    /* Force memory access pattern */
    checksum ^= ptr->b;
    
    /* 6. Additional SUBREG patterns with pointer casting */
    {
        int x = 0x12345678;
        short *sp = (short*)&x;
        volatile short vs;
        
        /* Generate SUBREG store */
        vs = *sp;  /* Load through short pointer */
        
        /* Generate SUBREG load */
        x = vs;    /* Store short to int */
        
        checksum += x;
    }
    
    /* 7. Create more complex expression with ZERO_EXTRACT */
    {
        volatile unsigned int mask = 0x00FF00FF;
        unsigned int val = checksum * 0x1234;
        
        /* Operation that might generate ZERO_EXTRACT */
        unsigned int extracted = (val & mask) | ((val >> 8) & mask);
        
        /* Store to volatile to force RTL generation */
        volatile unsigned int storage = extracted;
        checksum += storage;
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Use argc to affect control flow and prevent optimization */
    if (argc > 2) {
        /* Additional complex operations in rarely-taken path */
        volatile struct {
            unsigned int f1 : 3;
            unsigned int f2 : 5;
            unsigned int f3 : 24;
        } rare_struct;
        
        rare_struct.f1 = argv[2][0] & 0x7;
        rare_struct.f2 = (rare_struct.f1 * 3) & 0x1F;
        rare_struct.f3 = checksum & 0xFFFFFF;
        
        checksum ^= rare_struct.f2;
    }
    
    return checksum & 0xFF;
}
