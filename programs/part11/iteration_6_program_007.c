/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 5;
    unsigned int f : 12;
} S = {0};

/* Force memory addressing modes with noinline function */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate SET_DEST with ZERO_EXTRACT */
    s->a = 1;
    s->b = 2;
    s->c = 1000;
    s->d = 3;
    s->e = 4;
    s->f = 500;
    
    /* Additional mixed assignments to increase pattern variety */
    s->a = s->b & 0x3;  /* Bit-field operation */
    s->c = s->f >> 2;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_operations(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        int temp = shorts[i];
        temp = temp + (int)chars[i];  /* char to int promotion */
        shorts[i] = (short)(temp & 0xFFFF);  /* int to short truncation - SUBREG */
        
        /* More complex mixed-width operations */
        chars[i] = (char)((temp >> 8) & 0xFF);
    }
}

/* Complex addressing function */
__attribute__((noinline))
int complex_addressing(int arr[100][100], int idx1, int idx2) {
    /* Generate complex memory addressing modes */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices with arithmetic */
            int index1 = (idx1 + i) % 100;
            int index2 = (idx2 + j) % 100;
            
            /* Complex memory access that may generate ZERO_EXTRACT */
            int val = arr[index1][index2];
            
            /* Bitwise operation that could be represented as ZERO_EXTRACT */
            sum += val & 0x00FF00FF;  /* Mask operation */
            
            /* Additional mixed-width access */
            short half_val = (short)(val >> 16);
            sum += half_val;  /* short to int promotion */
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Force register pressure with many local variables */
    volatile int var1 = argc;
    volatile int var2 = argc * 2;
    volatile int var3 = argc + 1;
    volatile int var4 = argc - 1;
    volatile int var5 = argc * 3;
    volatile int var6 = argc / 2;
    volatile int var7 = argc % 5;
    volatile int var8 = argc + 10;
    volatile int var9 = argc - 3;
    volatile int var10 = argc * 4;
    
    /* 1. Trigger bit-field operations */
    modify_bitfields((struct BitFieldStruct*)&S);
    
    /* 2. Mixed-width array operations */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        short_array[i] = (short)(i * 3);
        char_array[i] = (char)(i % 128);
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(short_array, char_array, argc % 50 + 10);
    
    /* 3. Complex 2D array addressing with volatile indices */
    int arr[100][100];
    volatile int volatile_idx_i = argc % 90;
    volatile int volatile_idx_j = (argc * 7) % 90;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices */
    int arr_sum = complex_addressing(arr, volatile_idx_i, volatile_idx_j);
    
    /* 4. Additional mixed-type operations to generate SUBREG */
    volatile int int_var = 0x12345678;
    volatile short short_var;
    volatile char char_var;
    
    /* Generate SUBREG patterns through assignments */
    short_var = (short)int_var;  /* 32-bit to 16-bit - SUBREG */
    char_var = (char)int_var;    /* 32-bit to 8-bit - SUBREG */
    
    /* More complex expression with SUBREG */
    int_var = (int)short_var + (int)char_var * 256;
    
    /* 5. Inline assembly to increase register pressure and force resource tracking */
    asm volatile("" 
                 : 
                 : 
                 : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                   "r8", "r9", "r10", "r11", "r12", "memory");
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    /* Include all modified data in checksum */
    checksum ^= S.a;
    checksum ^= (S.b << 4);
    checksum ^= (S.c << 8);
    checksum ^= (S.d << 28);
    
    for (int i = 0; i < 50; i++) {
        checksum ^= short_array[i];
        checksum ^= (char_array[i] << 16);
    }
    
    checksum ^= arr_sum;
    checksum ^= int_var;
    checksum ^= short_var;
    checksum ^= char_var;
    
    /* Use all volatile variables */
    checksum ^= var1 ^ var2 ^ var3 ^ var4 ^ var5;
    checksum ^= var6 ^ var7 ^ var8 ^ var9 ^ var10;
    
    printf("Checksum: %u\n", checksum);
    
    return (int)checksum % 2;
}
