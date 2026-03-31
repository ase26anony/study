/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations
   that generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex
   MEM expressions in the RTL representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory operations */
volatile unsigned int global_bitfield = 0xABCD1234;
volatile unsigned int global_lowpart = 0xFFFFFFFF;
volatile long long global_ll = 0x1122334455667788LL;
int global_array[256];
struct S { unsigned int f:8; unsigned int g:8; } s;

/* 1. ZERO_EXTRACT patterns */
int extract_bits_ze1(volatile unsigned int *p) {
    /* Bit-field extraction using shift and mask */
    return (*p >> 8) & 0xFF;  /* Should generate ZERO_EXTRACT */
}

int extract_bits_ze2(void) {
    /* Bit-field struct member access */
    struct S local_s;
    local_s.f = 42;
    unsigned int read_f = local_s.f;  /* May generate ZERO_EXTRACT */
    return read_f;
}

int extract_bits_ze3(volatile unsigned int *p) {
    /* Multiple extractions with control flow */
    int result = 0;
    if (v_flag1) {
        result = (*p >> 16) & 0xFFFF;
    } else {
        result = (*p >> 4) & 0xF;
    }
    return result;
}

/* 2. STRICT_LOW_PART patterns */
void set_low_byte_slp1(volatile unsigned int *p, unsigned char v) {
    /* Writing only low byte of integer */
    *p = (*p & ~0xFF) | v;  /* May generate STRICT_LOW_PART */
}

void set_low_byte_slp2(void) {
    /* Cast and assignment to partial register */
    int32_t x = 0x12345678;
    *(int16_t*)&x = 0xABCD;  /* May generate STRICT_LOW_PART */
    global_lowpart = x;
}

void set_low_byte_slp3(volatile unsigned short *p) {
    /* Inline assembly that suggests partial register write */
    asm volatile("" : "+r"(*p) : : "cc");
}

/* 3. SUBREG patterns */
int32_t subreg_access1(void) {
    /* Union for type aliasing */
    union U {
        int32_t i;
        int16_t s[2];
    } u;
    u.i = 0x12345678;
    u.s[0] = 0xABCD;  /* May generate SUBREG */
    return u.i;
}

int32_t subreg_access2(void) {
    /* Pointer cast between different sizes */
    long long ll = global_ll;
    int i = *(int*)&ll;  /* May generate SUBREG */
    return i;
}

int32_t subreg_access3(volatile int64_t *p) {
    /* Mixed-size operations */
    int32_t result;
    if (v_flag2) {
        result = (int32_t)(*p >> 32);
    } else {
        result = (int32_t)*p;
    }
    return result;
}

/* 4. Complex MEM patterns */
int complex_mem1(int *base, int index1, int index2) {
    /* Complex addressing with arithmetic */
    return base[index1 + index2 * 4];  /* May generate complex MEM */
}

int complex_mem2(struct T {
    int arr[100];
    int pad;
} *tp, int i) {
    /* Struct with array access */
    return tp->arr[i];  /* May generate complex MEM */
}

int complex_mem3(void) {
    /* Multiple memory accesses with pointer arithmetic */
    int *ptr = &global_array[0];
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i * 3];  /* Non-linear access pattern */
    }
    return sum;
}

/* 5. Combined function with control flow */
int combined_operations(void) {
    int result = 0;
    
    /* Use volatile flags to create complex control flow */
    if (v_flag1) {
        /* ZERO_EXTRACT pattern */
        result += extract_bits_ze1(&global_bitfield);
        
        /* STRICT_LOW_PART pattern */
        set_low_byte_slp1(&global_lowpart, v_counter & 0xFF);
    }
    
    if (v_counter % 2) {
        /* SUBREG pattern */
        result ^= subreg_access1();
        
        /* Complex MEM pattern */
        result += complex_mem1(global_array, v_counter, v_counter + 1);
    } else {
        /* Alternative path with different patterns */
        result |= extract_bits_ze2();
        result += subreg_access2();
    }
    
    /* Loop with memory operations */
    for (int i = 0; i < 4; i++) {
        if (i & 1) {
            result += complex_mem3();
        } else {
            set_low_byte_slp2();
        }
    }
    
    return result;
}

/* Helper functions to increase pass activity */
void helper1(void) {
    /* Focus on ZERO_EXTRACT and SUBREG */
    int val = extract_bits_ze3(&global_bitfield);
    val += subreg_access3(&global_ll);
    global_array[0] = val;
}

void helper2(void) {
    /* Focus on STRICT_LOW_PART and MEM */
    set_low_byte_slp3((unsigned short*)&global_lowpart);
    
    struct T {
        int arr[100];
        int pad;
    } t;
    global_array[1] = complex_mem2(&t, 10);
}

void helper3(void) {
    /* Mixed patterns in loop */
    for (int i = 0; i < 8; i++) {
        if (i % 3 == 0) {
            extract_bits_ze1(&global_bitfield);
        } else if (i % 3 == 1) {
            set_low_byte_slp1(&global_lowpart, i);
        } else {
            subreg_access1();
        }
    }
}

/* Main function with observable side effects */
int main(void) {
    int final_result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Initialize struct bitfield */
    s.f = 0x42;
    s.g = 0x84;
    
    /* Call helpers multiple times with different volatile conditions */
    for (v_counter = 0; v_counter < 10; v_counter++) {
        v_flag1 = (v_counter % 3) != 0;
        v_flag2 = (v_counter % 5) != 0;
        
        /* Call combined operations */
        final_result += combined_operations();
        
        /* Call individual helpers */
        helper1();
        helper2();
        helper3();
        
        /* Update globals */
        global_bitfield ^= (v_counter << 8);
        global_ll += v_counter;
    }
    
    /* Ensure all operations contribute to observable output */
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
