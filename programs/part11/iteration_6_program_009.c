/* Target: resource.cc lines 282-290 - Complex SET_DEST patterns */

/* Global volatile struct with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
    unsigned int e : 24;
} g_bfs;

/* Non-inline function to prevent optimization of bit-field accesses */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFF;
    s->d = (y >> 12) & 0xFFFFF;
    s->e = (x + y) & 0xFFFFFF;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline, optimize("O0")))
int mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    int sum = 0;
    for (int i = 0; i < count; i++) {
        /* SUBREG patterns: mixing int, short, char */
        int temp = shorts[i];          /* short to int - may use SUBREG */
        temp += chars[i];              /* char to int - may use SUBREG */
        shorts[i] = temp & 0xFFFF;     /* int to short - may use SUBREG */
        sum += temp;
    }
    return sum;
}

/* Complex addressing with 2D array */
__attribute__((noinline, optimize("O0")))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access pattern */
    int val = arr[i][j];
    
    /* Bit-field like operation on memory value */
    val = (val & 0xFF00FF00) | ((val & 0x00FF00FF) << 8);
    
    /* Store back with potential ZERO_EXTRACT */
    arr[i][j] = val;
    
    return val;
}

/* Function with register pressure */
__attribute__((noinline, optimize("O0")))
void high_register_pressure(void) {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations creating SUBREG patterns */
    s1 = v1 + v2;           /* int to short */
    s2 = v3 - v4;           /* int to short */
    c1 = v5 & 0xFF;         /* int to char */
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    
    /* More operations after clobber */
    s3 = s1 + s2;           /* short arithmetic */
    c2 = c1 + 1;            /* char arithmetic */
    v1 = s3 + c2;           /* mixed to int */
}

int main(int argc, char **argv) {
    /* 1. Bit-field operations on global struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width array operations */
    volatile short short_arr[50];
    volatile char char_arr[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 50; i++) {
        short_arr[i] = (i * 3) & 0xFFFF;
        char_arr[i] = (i * 5) & 0xFF;
    }
    
    int sum = mixed_width_ops(short_arr, char_arr, argc % 50);
    
    /* 3. Complex 2D array addressing */
    int arr_2d[100][100];
    volatile int idx1 = argc, idx2 = argc * 3;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = (i * 100 + j) ^ 0x12345678;
        }
    }
    
    int complex_val = complex_addressing(arr_2d, &idx1, &idx2);
    
    /* 4. High register pressure section */
    high_register_pressure();
    
    /* 5. Additional volatile bit-field assignment in main */
    /* Direct assignment to force immediate RTL generation */
    volatile struct {
        unsigned int f1 : 3;
        unsigned int f2 : 5;
        unsigned int f3 : 10;
        unsigned int f4 : 14;
    } local_bf;
    
    local_bf.f1 = argc & 0x7;
    local_bf.f2 = (argc >> 3) & 0x1F;
    local_bf.f3 = (argc * 7) & 0x3FF;
    local_bf.f4 = (argc * 13) & 0x3FFF;
    
    /* 6. SUBREG patterns with pointer casting */
    volatile int int_var = 0x12345678;
    volatile short *short_ptr = (volatile short*)&int_var;
    volatile char *char_ptr = (volatile char*)&int_var;
    
    /* Access partial registers - may generate SUBREG */
    short_ptr[0] = argc & 0xFFFF;      /* Store to low half */
    char_ptr[2] = (argc >> 8) & 0xFF;  /* Store to third byte */
    
    /* 7. Compute checksum to prevent dead code elimination */
    int checksum = g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d + g_bfs.e;
    checksum += sum;
    checksum += complex_val;
    checksum += local_bf.f1 + local_bf.f2 + local_bf.f3 + local_bf.f4;
    checksum += int_var;
    checksum += short_arr[0] + char_arr[0];
    
    /* Use checksum to prevent optimization */
    if (checksum > 1000000) {
        return 1;
    }
    
    return 0;
}
