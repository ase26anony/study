/* Target patterns for GCC RTL generation to cover resource.cc lines 282-290 */
#include <stddef.h>

/* Force compiler to generate specific RTL patterns */
static volatile int global_counter = 0;

/* Pattern 1: ZERO_EXTRACT + MEM combination */
struct bitfield_struct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 3;
    volatile unsigned int field3 : 8;
    volatile unsigned int field4 : 16;
};

static int __attribute__((noinline)) 
pattern_zero_extract_mem(struct bitfield_struct *s, int idx) {
    volatile int arr[10][10];
    int result = 0;
    
    /* Generate MEM with complex addressing */
    result = arr[idx % 10][(idx + 1) % 10];
    
    /* Generate ZERO_EXTRACT through volatile bit-field assignment */
    s->field1 = (idx & 0x1F);
    s->field2 = ((idx >> 5) & 0x07);
    s->field3 = ((idx >> 8) & 0xFF);
    
    /* More MEM accesses with pointer arithmetic */
    volatile int *ptr = &arr[idx % 10][0];
    for (int i = 0; i < 5; i++) {
        result += *(ptr + i);
    }
    
    return result;
}

/* Pattern 2: STRICT_LOW_PART + SUBREG combination */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile short *ps, volatile char *pc) {
    int combined = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Generate SUBREG through type punning */
    int i = 0x12345678;
    short *short_ptr = (short*)&i;
    *short_ptr = 0xABCD;  /* This should generate SUBREG */
    
    /* Generate STRICT_LOW_PART through inline assembly */
    /* Using byte-sized operation on part of a register */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(temp_char)
        : "0"(temp_char)
        : "cc"
    );
    
    /* More SUBREG patterns with mixed-size accesses */
    long long ll = 0x1122334455667788ULL;
    int *int_ptr = (int*)&ll;
    *int_ptr = 0x88776655;  /* Accessing 64-bit as 32-bit */
    
    /* Another STRICT_LOW_PART pattern */
    asm volatile (
        "orb $0x0F, %0\n\t"
        : "=q"(temp_char)
        : "0"(temp_char)
        : "cc"
    );
    
    /* Cast between different pointer sizes */
    if (ps && pc) {
        *ps = (short)*pc;  /* Implicit conversion with possible SUBREG */
    }
}

/* Pattern 3: Complex expression mixing all patterns */
static int __attribute__((noinline))
pattern_complex_mix(int idx, volatile int *trigger) {
    struct bitfield_struct bs = {0};
    volatile int multi_array[5][5][5];
    int result = 0;
    
    /* Ternary operator selecting different addressing modes */
    volatile int *addr = (idx & 1) ? 
                         (volatile int*)&bs.field1 : 
                         (volatile int*)&multi_array[idx % 5][0][0];
    
    /* Generate MEM with complex index calculation */
    result = addr[(idx * 7) % 16];
    
    /* ZERO_EXTRACT with volatile */
    if (*trigger) {
        bs.field3 = (result & 0xFF);
        bs.field4 = ((result >> 8) & 0xFFFF);
    }
    
    /* SUBREG through pointer casting */
    long value = 0xDEADBEEF;
    int *val_ptr = (int*)&value;
    result ^= *val_ptr;  /* Access 64-bit as 32-bit */
    
    /* Inline assembly that might generate STRICT_LOW_PART */
    char byte_val = result & 0xFF;
    asm volatile (
        "subb $1, %0\n\t"
        : "=q"(byte_val)
        : "0"(byte_val)
        : "cc"
    );
    
    /* Complex MEM addressing in loop */
    for (int i = 0; i < 3; i++) {
        result += multi_array[i][idx % 5][(idx + i) % 5];
    }
    
    return result;
}

/* Helper to force compiler to keep variables alive */
static volatile int sink = 0;

int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? 10 : 5;
    struct bitfield_struct bs = {0};
    volatile short short_var = 0;
    volatile char char_var = 0;
    volatile int array_data[20] = {0};
    
    /* Initialize with some values */
    for (int i = 0; i < 20; i++) {
        array_data[i] = i * 3;
    }
    
    /* Main loop to trigger RTL generation */
    for (volatile int counter = 0; counter < iterations; counter++) {
        int idx = counter + global_counter;
        
        /* Call pattern functions with volatile arguments */
        int r1 = pattern_zero_extract_mem(&bs, idx);
        
        pattern_strict_low_part_subreg(&short_var, &char_var);
        
        int r2 = pattern_complex_mix(idx, &global_counter);
        
        /* Prevent dead code elimination */
        sink += r1 + r2 + short_var + char_var;
        
        /* Additional MEM patterns with pointer arithmetic */
        volatile int *ptr = array_data;
        for (int j = 0; j < 5; j++) {
            sink += *(ptr + (idx + j) % 20);
        }
        
        /* More bit-field operations for ZERO_EXTRACT */
        bs.field1 = (sink & 0x1F);
        bs.field2 = ((sink >> 5) & 0x07);
        
        global_counter++;
    }
    
    /* Final dummy operation */
    volatile int final_result = sink + global_counter;
    
    /* The following would cause UB if run, but is valid for compilation */
    /* Uncomment only for compilation testing, not execution */
    /*
    volatile int *danger = (volatile int*)0x1000;
    *danger = final_result;
    */
    
    return final_result != 0;
}
