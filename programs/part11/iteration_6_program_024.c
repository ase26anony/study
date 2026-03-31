/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate specific RTL patterns */
#define NOINLINE __attribute__((noinline))
#define NOOPT __attribute__((optimize("O0")))

/* Global volatile structure with bit-fields */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 5;
    unsigned int f : 12;
} g_bitfield = {0};

/* Helper to modify bit-fields - prevents inlining */
NOINLINE void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;           /* Should generate ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;        /* 8-bit field */
    s->c = 0x12345;     /* 20-bit field */
    s->d = 3;           /* 3-bit field */
    s->e = 0x1F;        /* 5-bit field */
    s->f = 0xABC;       /* 12-bit field */
    
    /* Mix assignments to prevent optimization */
    s->a = s->b & 0x7;  /* Complex expression with bit-field */
    s->c = s->f | 0x100;
}

/* Another helper with mixed-width operations */
NOINLINE NOOPT void mixed_width_ops(volatile short *shorts, volatile char *chars, 
                                    volatile int *ints, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that should generate SUBREG patterns */
        shorts[i] = (short)(ints[i] >> 16);          /* SUBREG from int to short */
        chars[i] = (char)(shorts[i] & 0xFF);         /* Further narrowing */
        ints[i] = (int)chars[i] * 2;                 /* Sign extension with SUBREG */
        
        /* Complex expression with memory and SUBREG */
        shorts[i] = (short)((ints[i] + shorts[i]) / 3);
    }
}

/* Function with complex addressing modes */
NOINLINE void complex_addressing(int arr[100][100], volatile int *idx1, 
                                 volatile int *idx2, volatile int *mask) {
    /* Complex array access with volatile indices */
    int val = arr[*idx1][*idx2];
    
    /* Bitwise operation that may generate ZERO_EXTRACT */
    val = val & *mask;
    
    /* Store back with potential STRICT_LOW_PART */
    arr[*idx1][*idx2] = val & 0xFF;  /* Lower 8 bits only */
    
    /* Additional complex pattern */
    arr[*idx1 + 1][*idx2] = (val >> 8) & 0xFF;
    arr[*idx1][*idx2 + 1] = (val >> 16) & 0xFF;
}

/* Main function with register pressure */
int main(int argc, char *argv[]) {
    /* 1. Bit-field operations */
    modify_bitfields((struct BitFieldStruct *)&g_bitfield);
    
    /* 2. Mixed-width operations with local volatile arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    volatile int int_arr[50];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 50; i++) {
        int_arr[i] = argc + i * 3;
    }
    
    /* Call mixed-width operations */
    mixed_width_ops(short_arr, char_arr, int_arr, argc % 30 + 10);
    
    /* 3. Complex addressing with 2D array */
    int matrix[100][100];
    volatile int idx1 = argc % 90;
    volatile int idx2 = (argc * 7) % 90;
    volatile int mask = 0x00FF00FF;
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    complex_addressing(matrix, &idx1, &idx2, &mask);
    
    /* 4. Create register pressure with many local variables */
    volatile int r0 = argc;
    volatile int r1 = argc * 2;
    volatile int r2 = argc * 3;
    volatile int r3 = argc * 4;
    volatile int r4 = argc * 5;
    volatile int r5 = argc * 6;
    volatile int r6 = argc * 7;
    volatile int r7 = argc * 8;
    volatile int r8 = argc * 9;
    volatile int r9 = argc * 10;
    
    /* 5. Inline assembly to clobber registers and force reload */
    asm volatile(
        "# Force register clobbering\n"
        "mov %0, %0\n"
        "mov %1, %1\n"
        : 
        : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), 
          "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9)
        : "memory", "cc"
    );
    
    /* Additional inline assembly with explicit clobbers */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                 "r8", "r9", "r10", "r11", "r12", "memory");
    
    /* 6. More bit-field operations with address taken */
    struct BitFieldStruct local_bitfield = {0};
    volatile struct BitFieldStruct *volatile ptr = &local_bitfield;
    
    /* Complex sequence of bit-field operations */
    ptr->a = (r0 & 0xF);
    ptr->b = (r1 >> 4) & 0xFF;
    ptr->c = (r2 + r3) & 0xFFFFF;
    
    /* Nested bit-field assignment */
    ptr->d = ptr->a + 1;
    ptr->e = ptr->b >> 2;
    ptr->f = ptr->c & 0xFFF;
    
    /* 7. Mixed operations in a loop to increase RTL complexity */
    volatile int temp;
    for (int i = 0; i < (argc % 20 + 5); i++) {
        /* Mix of operations that should generate various RTL patterns */
        temp = short_arr[i % 50];
        char_arr[i % 50] = (temp >> (i % 8)) & 0xFF;
        int_arr[i % 50] = (int)char_arr[i % 50] * int_arr[(i + 1) % 50];
        
        /* Bit-field operation in loop */
        if (i % 3 == 0) {
            ptr->a = (ptr->a + 1) & 0xF;
        }
    }
    
    /* 8. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum ^= g_bitfield.a;
    checksum ^= g_bitfield.b << 4;
    checksum ^= g_bitfield.c << 12;
    checksum ^= local_bitfield.d;
    checksum ^= local_bitfield.e << 3;
    checksum ^= local_bitfield.f << 8;
    
    for (int i = 0; i < 50; i++) {
        checksum ^= short_arr[i];
        checksum ^= char_arr[i] << 8;
        checksum ^= int_arr[i];
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum ^= matrix[i][j];
        }
    }
    
    checksum ^= r0 ^ r1 ^ r2 ^ r3 ^ r4 ^ r5 ^ r6 ^ r7 ^ r8 ^ r9;
    
    printf("Checksum: %u\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
