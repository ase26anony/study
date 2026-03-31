/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c */
/* This program generates RTL patterns that trigger mark_referenced_resources */
/* for SET_DEST with ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM patterns */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile struct with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    int a:4;    /* 4-bit field */
    int b:8;    /* 8-bit field */
    int c:20;   /* 20-bit field */
    int padding:32;
} g_bfs = {0};

/* 2D array for complex addressing modes */
int g_arr[100][100];

/* Helper to prevent optimization */
int __attribute__((noinline)) use_value(int x) {
    volatile int sink = x;
    return sink;
}

/* Function to modify bit-fields with noinline to prevent inlining */
void __attribute__((noinline, optimize("O0"))) 
modify_bitfields(struct BitFieldStruct *s, int val1, int val2, int val3) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = val1 & 0xF;      /* Will generate ZERO_EXTRACT for 4-bit field */
    s->b = val2 & 0xFF;     /* Will generate ZERO_EXTRACT for 8-bit field */
    s->c = val3 & 0xFFFFF;  /* Will generate ZERO_EXTRACT for 20-bit field */
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}

/* Function with mixed-width operations to generate SUBREG patterns */
void __attribute__((noinline, optimize("O0")))
mixed_width_operations(short *sptr, char *cptr, int *iptr, int count) {
    volatile int i;
    
    for (i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width assignments */
        short temp_short = (short)(*iptr + i);  /* int to short conversion */
        *sptr = temp_short;                     /* SUBREG store */
        
        /* More mixed-width operations */
        char temp_char = (char)(temp_short & 0xFF);
        *cptr = temp_char;                      /* Another SUBREG store */
        
        /* Complex expression with SUBREG */
        int temp_int = (int)temp_char * 2;      /* char to int promotion */
        *iptr = temp_int + (*sptr);             /* Mixed-width operation */
        
        /* Pointer arithmetic with different widths */
        sptr++;
        cptr++;
        iptr++;
    }
}

/* Function with complex array addressing */
int __attribute__((noinline, optimize("O0")))
complex_array_access(int idx1, int idx2, int idx3) {
    volatile int vi = idx1;
    volatile int vj = idx2;
    volatile int vk = idx3;
    
    /* Complex addressing with volatile indices */
    int *ptr = &g_arr[vi % 50][vj % 50];
    
    /* Bit-field like operation on array element */
    *ptr = (*ptr & 0xFFFF0000) | (vk & 0xFFFF);
    
    /* More complex addressing with pointer arithmetic */
    int val = g_arr[(vi + vj) % 50][(vj + vk) % 50];
    
    /* Operation that might generate ZERO_EXTRACT */
    return (val & 0xFF) | ((val >> 8) & 0xFF00);
}

int main(int argc, char **argv) {
    /* Force argc to be used to prevent constant propagation */
    volatile int arg = argc;
    
    /* 1. Bit-field operations on global struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, arg, arg * 2, arg * 3);
    
    /* 2. Mixed-width local variables */
    volatile short vs;
    volatile char vc;
    volatile int vi;
    
    /* Mixed-width assignments to generate SUBREG */
    vs = (short)arg;           /* int to short - SUBREG */
    vc = (char)(arg & 0xFF);   /* int to char - SUBREG */
    vi = (int)vc * 10;         /* char to int promotion */
    
    /* 3. Array with mixed types for SUBREG generation */
    short s_arr[50];
    char c_arr[50];
    int i_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50 && i < arg; i++) {
        s_arr[i] = (short)(i * 2);
        c_arr[i] = (char)(i * 3);
        i_arr[i] = i * 5;
    }
    
    /* Perform mixed-width operations */
    mixed_width_operations(s_arr, c_arr, i_arr, (arg % 20) + 1);
    
    /* 4. Complex array addressing with bit operations */
    int result = complex_array_access(arg, arg + 1, arg + 2);
    
    /* 5. Create register pressure with inline assembly */
    asm volatile(
        "# Force register pressure\n"
        "mov r0, %0\n"
        "mov r1, %1\n"
        "add r2, r0, r1\n"
        : 
        : "r" (result), "r" (arg)
        : "r0", "r1", "r2", "memory", "cc"
    );
    
    /* 6. More bit-field operations with different patterns */
    struct BitFieldStruct local_bfs;
    
    /* Direct bit-field assignments */
    local_bfs.a = (arg & 0xF);
    local_bfs.b = ((arg >> 4) & 0xFF);
    local_bfs.c = ((arg >> 12) & 0xFFFFF);
    
    /* Pointer to bit-field struct */
    struct BitFieldStruct *bfs_ptr = &local_bfs;
    
    /* Additional bit-field stores through pointer */
    bfs_ptr->a = (result & 0xF);
    bfs_ptr->b = ((result >> 4) & 0xFF);
    
    /* 7. Complex expression combining everything */
    int final_result = 
        (g_bfs.a & 0xF) |
        ((g_bfs.b & 0xFF) << 4) |
        ((g_bfs.c & 0xFFFFF) << 12) |
        (result & 0xFF000000);
    
    /* Use all computed values to prevent dead code elimination */
    final_result += vs + vc + vi;
    
    for (int i = 0; i < 10 && i < arg; i++) {
        final_result += s_arr[i] + c_arr[i] + i_arr[i];
    }
    
    /* Force memory accesses */
    asm volatile("" ::: "memory");
    
    printf("Result: %d\n", final_result % 1000);
    return final_result % 256;
}
