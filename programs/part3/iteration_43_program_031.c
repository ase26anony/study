/* Pattern-generating code for GCC resource tracking coverage */
#include <stddef.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))

/* Structure with volatile bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 7;
    volatile unsigned int f3 : 12;
    volatile unsigned int padding : 8;
};

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_result = 0;

/* Function A: Focus on ZERO_EXTRACT and MEM patterns */
NOINLINE static void pattern_a(struct bitfield_struct *s, int idx) {
    /* Complex addressing for MEM pattern */
    volatile int *arr[5];
    volatile int data[5][10];
    
    /* Initialize array of pointers with complex addressing */
    for (int i = 0; i < 5; i++) {
        arr[i] = &data[i][0];
    }
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    s->f1 = idx & 0x1F;
    s->f2 = (idx >> 5) & 0x7F;
    s->f3 = (idx >> 12) & 0xFFF;
    
    /* MEM pattern with complex addressing */
    volatile int *ptr = arr[idx % 5] + (idx % 10);
    global_result += *ptr;
    
    /* Additional MEM pattern with pointer arithmetic */
    int offset = idx * 3;
    volatile int *mem_ptr = (volatile int*)((char*)s + (offset % sizeof(*s)));
    global_counter += *mem_ptr;
}

/* Function B: Focus on STRICT_LOW_PART and SUBREG patterns */
NOINLINE static void pattern_b(int val) {
    /* STRICT_LOW_PART pattern using inline assembly */
    unsigned char byte_val = (unsigned char)val;
    unsigned short short_val = (unsigned short)val;
    
    /* Modify only low byte of register */
    asm volatile (
        "addb $1, %0"
        : "=q" (byte_val)
        : "0" (byte_val)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with short */
    asm volatile (
        "subw $2, %0"
        : "=q" (short_val)
        : "0" (short_val)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    int int_val = val;
    short *short_ptr = (short*)&int_val;
    
    /* Access through SUBREG (different sized access) */
    *short_ptr = (short)(val + 1);
    
    /* Another SUBREG pattern with char */
    unsigned char *byte_ptr = (unsigned char*)&int_val;
    byte_ptr[1] = (unsigned char)(val >> 8);
    
    global_result += int_val + byte_val + short_val;
}

/* Function C: Complex expression mixing patterns */
NOINLINE static void pattern_c(struct bitfield_struct *s, int idx, int cond) {
    /* Ternary operator selecting address for MEM pattern */
    volatile int *select_ptr;
    
    if (cond & 1) {
        /* Address of bit-field (potential ZERO_EXTRACT) */
        select_ptr = (volatile int*)&s->f1;
    } else {
        /* Regular integer pointer */
        static volatile int alt_data[10];
        select_ptr = &alt_data[idx % 10];
    }
    
    /* Complex expression with multiple memory accesses */
    int temp = *select_ptr;
    
    /* Mixed-size access causing SUBREG */
    short *temp_short = (short*)&temp;
    temp_short[0] = (short)(temp_short[1] + idx);
    
    /* Assignment that could involve multiple RTL transformations */
    s->f2 = (temp & 0x7F) | ((idx << 3) & 0x7F);
    
    /* Additional MEM with complex index */
    volatile int multi_arr[3][4][5];
    global_counter += multi_arr[idx % 3][(idx >> 2) % 4][(idx >> 4) % 5];
}

/* Helper function with loop to generate repeated patterns */
NOINLINE static void generate_patterns(int iterations) {
    struct bitfield_struct bs = {0};
    volatile int array[100];
    
    /* Initialize array with volatile writes */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 100;
        
        /* Call pattern functions with different arguments */
        pattern_a(&bs, idx);
        pattern_b(array[idx]);
        pattern_c(&bs, idx, i);
        
        /* Additional direct MEM pattern with complex addressing */
        volatile int *ptr1 = &array[idx];
        volatile int *ptr2 = &array[(idx * 7) % 100];
        volatile int *ptr3 = &array[(idx * 13) % 100];
        
        /* Chain of memory accesses */
        int val = *ptr1 + *ptr2;
        *ptr3 = val + *ptr3;
        
        /* Force SUBREG through type punning in loop */
        union {
            int i;
            short s[2];
            char c[4];
        } u;
        
        u.i = idx;
        u.s[0] = (short)(u.s[1] + 1);
        u.c[2] = u.c[0] ^ 0x55;
        
        global_result += u.i;
    }
}

/* Main function with volatile counters and loops */
int main(int argc, char *argv[]) {
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    int iterations = (argc > 1) ? (argv[1][0] % 10) + 1 : 5;
    
    /* Volatile iteration counter */
    volatile int loop_counter = 0;
    
    /* Initialize data structures */
    struct bitfield_struct main_bs = {0};
    volatile int main_array[50];
    
    for (int i = 0; i < 50; i++) {
        main_array[i] = i * 2;
    }
    
    /* Main loop calling pattern generators */
    for (volatile int outer = 0; outer < 3; outer++) {
        generate_patterns(iterations);
        
        /* Additional inline pattern generation */
        for (volatile int inner = 0; inner < 2; inner++) {
            /* STRICT_LOW_PART with different constraints */
            unsigned char c = outer + inner;
            asm volatile (
                "incb %0"
                : "+q" (c)
                :
                : "cc"
            );
            
            /* SUBREG with pointer casting */
            int *int_ptr = (int*)&main_array[inner * 3];
            short *short_ptr = (short*)int_ptr;
            short_ptr[1] = (short)c;
            
            /* ZERO_EXTRACT with volatile struct */
            main_bs.f3 = (main_bs.f3 + c) & 0xFFF;
            
            /* Complex MEM addressing */
            volatile int ***triple_ptr;
            volatile int **double_ptr;
            volatile int *single_ptr;
            
            single_ptr = &main_array[(outer * 17 + inner * 23) % 50];
            double_ptr = &single_ptr;
            triple_ptr = &double_ptr;
            
            /* Commented out - would cause UB if executed but valid for compilation */
            /* global_result += ***triple_ptr; */
        }
        
        loop_counter++;
    }
    
    /* Final dummy operation to prevent dead code elimination */
    volatile int final_sum = global_result + global_counter + loop_counter;
    
    /* The program doesn't need correct semantics, just compilation */
    /* Return 0 to satisfy compiler */
    return 0;
}
