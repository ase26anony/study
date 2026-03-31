/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate ZERO_EXTRACT/STRICT_LOW_PART RTL patterns */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bitfield;

/* Force memory addressing modes */
volatile int g_memory_pressure[256];

/* Complex addressing array */
int g_complex_array[100][100];

/* Non-inline function to prevent optimization */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely generates ZERO_EXTRACT */
    s->b = 0xFF;   /* Another bit-field store */
    s->c = 0x7FF;  /* More bit-field operations */
    s->d = 0x55;   /* Additional bit-field assignment */
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that may generate SUBREG RTL */
        int temp = shorts[i];          /* Load short, may use SUBREG */
        temp = temp * 2;               /* Arithmetic */
        chars[i] = (char)(temp & 0xFF); /* Store char, may use SUBREG */
        
        /* More mixed-width operations */
        shorts[i] = (short)(temp >> 8); /* int to short, may generate SUBREG */
    }
}

/* Function to create complex addressing */
__attribute__((noinline))
int complex_addressing(volatile int idx1, volatile int idx2) {
    /* Complex array access with volatile indices */
    int result = g_complex_array[idx1 % 100][idx2 % 100];
    
    /* Combine with bitwise operation that might be ZERO_EXTRACT */
    result &= 0x00FF00FF;  /* Mask operation */
    
    /* More operations to increase RTL complexity */
    result |= (idx1 << 16) | (idx2 & 0xFFFF);
    
    return result;
}

int main(int argc, char **argv) {
    /* Step 1: Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct *)&g_bitfield);
    
    /* Step 2: Mixed-width operations to generate SUBREG patterns */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        short_array[i] = (short)(i * 3);
        char_array[i] = (char)(i * 5);
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(short_array, char_array, argc > 1 ? atoi(argv[1]) % 50 + 50 : 50);
    
    /* Step 3: Complex addressing with volatile indices */
    volatile int idx1 = argc * 7;
    volatile int idx2 = argc * 13;
    int complex_result = complex_addressing(idx1, idx2);
    
    /* Step 4: More memory pressure and register pressure */
    volatile int local_pressure[50];
    for (int i = 0; i < 50; i++) {
        /* Mix types to encourage SUBREG */
        local_pressure[i] = (int)((short)(i * 2) * (char)(i + 1));
        
        /* Access global memory to force MEM_P(x) path */
        g_memory_pressure[i % 256] = local_pressure[i] ^ complex_result;
    }
    
    /* Step 5: Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "/* Clobber multiple registers to force reload */\n\t"
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        : 
        : "r" (complex_result), "r" (local_pressure[0])
        : "r0", "r1", "r2", "r3", "memory"
    );
    
    /* Additional inline assembly for more pressure */
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        : 
        : 
        : "cc", "memory"
    );
    
    /* Step 6: Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum ^= g_bitfield.a;
    checksum ^= g_bitfield.b << 4;
    checksum ^= g_bitfield.c << 8;
    checksum ^= g_bitfield.d << 16;
    
    for (int i = 0; i < 50; i++) {
        checksum ^= local_pressure[i];
        checksum ^= short_array[i % 100];
        checksum ^= char_array[i % 100] << 8;
    }
    
    checksum ^= complex_result;
    
    /* Use argc to make indices truly non-constant */
    checksum ^= g_complex_array[argc % 50][(argc * 2) % 50];
    
    printf("Checksum: %u\n", checksum);
    
    return (int)checksum % 256;
}

/* Additional global to increase complexity */
volatile int g_extra_pressure[10][10][10];

/* Function never called but affects compilation */
__attribute__((noinline, optimize("O0")))
void unused_but_affects_compilation(void) {
    /* Complex nested operations */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                /* Mixed-width access */
                short temp = (short)g_extra_pressure[i][j][k];
                g_extra_pressure[i][j][k] = (int)(temp * 3) & 0xFF;
            }
        }
    }
}
