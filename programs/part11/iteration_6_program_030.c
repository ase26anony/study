/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -c this_file.c */
/* For debugging: gcc -O0 -fdump-rtl-expand -c this_file.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    int a : 4;   /* 4-bit field */
    int b : 8;   /* 8-bit field */
    int c : 20;  /* 20-bit field */
    int d : 1;   /* 1-bit field */
} S;

/* Force memory addressing modes and prevent optimization */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to same memory location */
    s->a = 1;      /* Likely generates ZERO_EXTRACT or STRICT_LOW_PART */
    s->b = 0x7F;   /* Another bit-field store */
    s->c = 0x12345;
    s->d = 1;
    
    /* Mix with regular memory access */
    volatile int *p = (volatile int*)s;
    *p = *p & 0xFFF;  /* Complex memory operation */
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(volatile short *arr, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that force SUBREG patterns */
        int val = i * 256 + i;
        arr[i] = (short)val;  /* 32-bit to 16-bit store - SUBREG likely */
        
        /* More complex expression with SUBREG */
        char c = (char)(val & 0xFF);
        arr[i] = arr[i] + (short)c;  /* Mixed-width operation */
    }
}

/* Complex addressing mode generator */
__attribute__((noinline))
int complex_addressing(int idx1, int idx2) {
    static int arr[100][100];
    volatile int *volatile_idx1 = &idx1;  /* Prevent constant propagation */
    volatile int *volatile_idx2 = &idx2;
    
    /* Complex array access with non-constant indices */
    int val = arr[*volatile_idx1 % 100][*volatile_idx2 % 100];
    
    /* Bitwise operation that might generate ZERO_EXTRACT */
    return (val & 0x00FF00FF);  /* Masking specific bytes */
}

int main(int argc, char **argv) {
    /* 1. Trigger bit-field patterns */
    modify_bitfields((struct BitFieldStruct*)&S);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 3;
        char_arr[i] = (char)(i & 0xFF);
    }
    
    /* Mixed type operations in loop */
    for (int i = 0; i < argc * 10; i++) {  /* Non-constant bound */
        int idx = i % 100;
        /* Multiple SUBREG-inducing operations */
        short_arr[idx] = (short)int_arr[idx];  /* 32-bit to 16-bit */
        char_arr[idx] = (char)(short_arr[idx] & 0xFF);  /* 16-bit to 8-bit */
        
        /* Sign extension operations */
        int signed_val = (signed char)char_arr[idx];  /* SUBREG for sign extend */
        int_arr[idx] = int_arr[idx] + signed_val;
    }
    
    mixed_width_ops(short_arr, argc * 5);
    
    /* 3. Complex addressing modes */
    volatile int v1 = argc;
    volatile int v2 = argc * 2;
    int result = complex_addressing(v1, v2);
    
    /* 4. Create register pressure with inline assembly */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory");
    
    /* 5. More bit-field operations on local volatile struct */
    volatile struct {
        int x : 10;
        int y : 22;
    } local_bf;
    
    local_bf.x = argc & 0x3FF;
    local_bf.y = (argc * argc) & 0x3FFFFF;
    
    /* 6. Additional SUBREG patterns through pointer casting */
    volatile int *int_ptr = &int_arr[0];
    volatile short *short_ptr = (volatile short*)int_ptr;
    volatile char *char_ptr = (volatile char*)int_ptr;
    
    /* Cross-type stores that generate SUBREG */
    *short_ptr = (short)(*int_ptr >> 16);  /* Store high half */
    char_ptr[2] = (char)(*int_ptr & 0xFF); /* Store byte */
    
    /* 7. Nested complex expressions */
    int complex_result = 0;
    for (int i = 0; i < argc; i++) {
        /* Combine bit-field access with array access */
        complex_result += ((int_arr[i] & 0xF) << 4) | (local_bf.x & 0xF);
        
        /* Memory access with addressing computation */
        complex_result += short_arr[(i * 7) % 100] * 2;
    }
    
    /* 8. Final checksum to prevent dead code elimination */
    int checksum = S.a + S.b + S.c + S.d;
    checksum += result + complex_result;
    
    for (int i = 0; i < 100; i++) {
        checksum += int_arr[i] + short_arr[i] + char_arr[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
