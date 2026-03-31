/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -c coverage.c */

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

/* Force memory addressing and prevent optimization */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate complex SET_DEST patterns */
    s->a = 1;      /* Should generate ZERO_EXTRACT or STRICT_LOW_PART */
    s->b = 0x7F;   /* Another bit-field store */
    s->c = 0x12345;
    s->d = 3;
    s->e = 0x1ABCD;
    
    /* Mix with regular memory access */
    volatile int *p = (volatile int*)s;
    *p = *p ^ 0x80000000;
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline))
void mixed_width_ops(volatile short *arr, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that should generate SUBREG RTL */
        volatile int temp = i * 256;
        arr[i] = (short)(temp + i);  /* int to short: SUBREG pattern */
        
        /* More complex expression with SUBREG */
        volatile char c = (char)(arr[i] & 0xFF);
        arr[i] = (short)(c * 2 + i);
    }
}

/* Function with complex addressing modes */
__attribute__((noinline))
int complex_addressing(int idx1, int idx2) {
    static int arr[100][100];
    volatile int *volatile_idx1 = &idx1;
    volatile int *volatile_idx2 = &idx2;
    
    /* Complex memory access with volatile indices */
    int val = arr[*volatile_idx1 % 100][*volatile_idx2 % 100];
    
    /* Bit-field like operation on memory */
    val = (val & 0xFFFF0000) | ((val & 0xFFFF) << 1);
    
    return val;
}

/* Force register pressure */
__attribute__((noinline, optimize("O0")))
void register_pressure() {
    /* Many local variables to force spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile short s1 = 11, s2 = 12, s3 = 13, s4 = 14;
    volatile char c1 = 15, c2 = 16, c3 = 17, c4 = 18;
    
    /* Mixed width operations generating SUBREG */
    v1 = (int)s1 + (int)c1;
    v2 = (v2 << 8) | (int)c2;
    s3 = (short)(v3 & 0xFFFF);
    c4 = (char)(v4 & 0xFF);
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* Use all variables to prevent elimination */
    v10 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    s4 = (short)v10;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* 1. Trigger ZERO_EXTRACT/STRICT_LOW_PART with bit-field operations */
    modify_bitfields((struct BitFieldStruct*)&S);
    
    /* 2. Trigger SUBREG patterns with mixed-width operations */
    volatile short short_array[100];
    mixed_width_ops(short_array, argc > 1 ? atoi(argv[1]) % 50 + 10 : 20);
    
    /* 3. Complex addressing modes */
    volatile int idx1 = argc;
    volatile int idx2 = argc * 2;
    result += complex_addressing(idx1, idx2);
    
    /* 4. Force register pressure and reload */
    register_pressure();
    
    /* 5. More bit-field operations on local volatile struct */
    volatile struct {
        unsigned int f1 : 5;
        unsigned int f2 : 11;
        unsigned int f3 : 16;
    } local_bf;
    
    local_bf.f1 = argc & 0x1F;
    local_bf.f2 = (argc * 3) & 0x7FF;
    local_bf.f3 = (argc * 5) & 0xFFFF;
    
    /* 6. Additional SUBREG patterns through pointer casting */
    volatile int *int_ptr = (volatile int*)&local_bf;
    volatile short *short_ptr = (volatile short*)int_ptr;
    volatile char *char_ptr = (volatile char*)int_ptr;
    
    *short_ptr = (short)(*int_ptr >> 8);  /* int to short: SUBREG */
    *char_ptr = (char)(*short_ptr & 0x7F); /* short to char: SUBREG */
    
    /* 7. Complex expression combining everything */
    for (int i = 0; i < (argc % 10 + 5); i++) {
        /* Bit-field extraction from memory */
        unsigned int temp = *int_ptr;
        unsigned int extracted = (temp >> local_bf.f1) & ((1 << local_bf.f2) - 1);
        
        /* Store with potential ZERO_EXTRACT */
        local_bf.f3 = extracted & 0xFFFF;
        
        /* Mixed-width operation */
        short_array[i % 50] = (short)(extracted + i);
    }
    
    /* Compute checksum to prevent dead code elimination */
    result += S.a + S.b + S.c;
    result += local_bf.f1 + local_bf.f2 + local_bf.f3;
    
    for (int i = 0; i < 20; i++) {
        result += short_array[i];
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
