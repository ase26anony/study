/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -c this_file.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} S;

/* Force memory addressing modes with noinline function */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;
    s->b = 2;
    s->c = 1000;
    s->d = 3;
    s->e = 50000;
    
    /* Additional operations to create complex patterns */
    s->a = s->b & 0x3;  /* Bit-field operation */
    s->c = s->e >> 2;   /* Another bit-field operation */
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        int temp = (int)shorts[i] * 2;      /* short to int promotion */
        chars[i] = (char)(temp & 0xFF);     /* int to char truncation */
        shorts[i] = (short)(temp >> 8);     /* int to short truncation */
        
        /* More complex mixed-width expressions */
        shorts[i] = (short)((shorts[i] * 3 + chars[i]) & 0xFFFF);
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[100][100], int idx1, int idx2) {
    /* Complex array access with non-constant indices */
    int val = arr[idx1][idx2];
    
    /* Combine with bitwise operation that may generate ZERO_EXTRACT */
    return (val & 0xFFF) | ((val >> 12) & 0xFF);
}

int main(int argc, char *argv[]) {
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct *)&S);
    
    /* 2. Mixed-width operations with local volatile arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 50; i++) {
        short_arr[i] = (short)(i * 3);
        char_arr[i] = (char)(i * 5);
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(short_arr, char_arr, argc > 1 ? atoi(argv[1]) % 50 : 25);
    
    /* 3. Complex 2D array access with volatile indices */
    int arr[100][100];
    volatile int idx_i = 10;
    volatile int idx_j = 20;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices */
    int result = complex_addressing(arr, idx_i, idx_j);
    
    /* 4. Additional mixed-type operations to generate SUBREG */
    volatile int int_var = 0x12345678;
    volatile short short_var;
    volatile char char_var;
    
    /* Generate SUBREG patterns through assignments */
    short_var = (short)int_var;          /* 32-bit to 16-bit */
    char_var = (char)short_var;          /* 16-bit to 8-bit */
    int_var = (int)char_var * 256;       /* 8-bit to 32-bit with scaling */
    
    /* 5. Inline assembly to increase register pressure and force resource tracking */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* 6. More complex expressions combining multiple patterns */
    struct BitFieldStruct local_s;
    local_s.a = (int_var & 0xF);
    local_s.b = (short_var & 0xFF);
    local_s.c = (result & 0xFFFFF);
    
    /* Additional volatile operations to prevent optimization */
    volatile int *volatile_ptr = &int_var;
    *volatile_ptr = (*volatile_ptr << 4) | (local_s.a & 0xF);
    
    /* 7. Compute checksum to prevent dead code elimination */
    unsigned long checksum = 0;
    checksum += S.a + S.b + S.c + S.d + S.e;
    
    for (int i = 0; i < 50; i++) {
        checksum += short_arr[i];
        checksum += char_arr[i];
    }
    
    checksum += result;
    checksum += int_var;
    checksum += short_var;
    checksum += char_var;
    checksum += local_s.a + local_s.b + local_s.c;
    
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
