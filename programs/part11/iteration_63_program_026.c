/* test_resource_patterns.c
 * Designed to generate RTL SET destinations with:
 * - ZERO_EXTRACT
 * - STRICT_LOW_PART  
 * - SUBREG
 * - Complex MEM patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(int);
extern void escape(void*);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed;
static volatile int volatile_index;
static volatile short volatile_short;
static volatile int volatile_int;
static volatile long long volatile_ll;

/* Global variables for memory patterns */
int global_array[100];
struct ComplexStruct {
    int a;
    int b;
    long long c;
    short d;
} global_struct;

/* Prevent inlining and IPA */
__attribute__((noinline, noipa))
void test_zero_extract(void) {
    /* Pattern 1: ZERO_EXTRACT destination via bitfield union */
    union BitfieldUnion {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } parts;
    } u;
    
    u.full = volatile_seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT destination */
    u.parts.mid = volatile_int & 0xFF;
    use(u.full);
    
    /* Alternative: explicit bitfield extraction */
    unsigned int val = volatile_seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (volatile_int & 0xFF) << 8;
    /* May generate: (set (zero_extract ...) ...) */
    val = (val & ~mask) | insert;
    sink(val);
}

__attribute__((noinline, noipa)) 
void test_strict_low_part(void) {
    /* Pattern 2: STRICT_LOW_PART destination */
    int large_val = volatile_seed;
    short small_val = volatile_short;
    
    /* Assignment that only affects low bits */
    *(short*)&large_val = small_val;
    use(large_val);
    
    /* Alternative with arithmetic */
    long long big = volatile_ll;
    int medium = volatile_int;
    /* May generate strict_low_part for the 32-bit assignment */
    *(int*)&big = medium;
    sink((int)big);
    
    /* Using type punning through union */
    union {
        long long ll;
        int i[2];
    } pun;
    pun.ll = volatile_ll;
    pun.i[0] = volatile_int;  /* Only affects low part */
    escape(&pun);
}

__attribute__((noinline, noipa))
void test_subreg(void) {
    /* Pattern 3: SUBREG destinations */
    int array[4] = {0};
    volatile int idx = volatile_index & 3;
    
    /* Access sub-word through pointer cast */
    short *ps = (short*)&array[idx];
    *ps = volatile_short;
    use(array[idx]);
    
    /* Type punning between different sizes */
    long long big_val = volatile_ll;
    int *p_int = (int*)&big_val;
    *p_int = volatile_int;  /* May use SUBREG to access half */
    sink(*p_int);
    
    /* Array of different types */
    char buffer[16];
    int *alias = (int*)&buffer[volatile_index % 12];
    *alias = volatile_int;
    escape(buffer);
}

__attribute__((noinline, noipa))
void test_complex_mem(void) {
    /* Pattern 4: Complex MEM destinations */
    struct ComplexStruct local;
    volatile int offset = volatile_index;
    
    /* Complex addressing mode */
    int *ptr = &local.a + (offset % 4);
    *ptr = volatile_int;
    use(local.a);
    
    /* Global with index */
    int *global_ptr = &global_array[volatile_index % 50];
    *global_ptr = volatile_int * 2;
    sink(*global_ptr);
    
    /* Pointer arithmetic with multiple operations */
    char *base = (char*)&global_struct;
    int *int_ptr = (int*)(base + sizeof(int) + (offset % 8));
    *int_ptr = volatile_int;
    escape(int_ptr);
}

__attribute__((noinline, noipa))
void test_mixed_patterns_in_loop(void) {
    /* Combine patterns in loops with control flow */
    unsigned int checksum = 0;
    volatile int limit = (volatile_seed & 0xF) + 5;
    
    for (int i = 0; i < limit; i++) {
        if (volatile_seed & (1 << (i % 16))) {
            /* ZERO_EXTRACT pattern in loop */
            union {
                unsigned int val;
                struct {
                    unsigned int a: 4;
                    unsigned int b: 4;
                    unsigned int c: 24;
                } bits;
            } u;
            u.val = checksum;
            u.bits.b = (volatile_int + i) & 0xF;
            checksum = u.val;
        } else {
            /* SUBREG pattern in loop */
            long long temp = checksum;
            short *ps = (short*)&temp + (i & 1);
            *ps = (volatile_short + i) & 0xFFFF;
            checksum ^= (unsigned int)temp;
        }
        
        /* Complex MEM in loop */
        global_array[i % 50] = checksum;
    }
    
    sink(checksum);
}

__attribute__((noinline, noipa))
void test_nested_control_flow(void) {
    /* Nested control flow with different patterns */
    int result = 0;
    
    for (int outer = 0; outer < (volatile_seed & 3) + 2; outer++) {
        int temp = volatile_int;
        
        switch (outer % 3) {
            case 0:
                /* STRICT_LOW_PART pattern */
                *(short*)&temp = volatile_short + outer;
                break;
            case 1:
                /* SUBREG pattern */
                {
                    int array[2] = {temp, volatile_int};
                    char *pc = (char*)array + outer;
                    *pc = (volatile_seed >> outer) & 0xFF;
                    temp = array[0];
                }
                break;
            case 2:
                /* Complex MEM pattern */
                {
                    struct {
                        int x;
                        int y;
                    } s;
                    int *p = (outer & 1) ? &s.x : &s.y;
                    *p = temp;
                    temp = s.x + s.y;
                }
                break;
        }
        
        if (volatile_seed & (1 << outer)) {
            /* ZERO_EXTRACT pattern in conditional */
            unsigned int val = temp;
            unsigned int mask = 0xF0F;
            val = (val & ~mask) | ((volatile_int + outer) & mask);
            temp = val;
        }
        
        result ^= temp;
    }
    
    use(result);
}

int main(int argc, char *argv[]) {
    /* Initialize volatile variables */
    volatile_seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    volatile_index = volatile_seed % 100;
    volatile_short = volatile_seed & 0xFFFF;
    volatile_int = volatile_seed;
    volatile_ll = (long long)volatile_seed * volatile_seed;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    global_struct.a = volatile_seed;
    global_struct.b = volatile_seed * 2;
    global_struct.c = volatile_ll;
    global_struct.d = volatile_short;
    
    /* Execute all test patterns */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_mixed_patterns_in_loop();
    test_nested_control_flow();
    
    /* Create a checksum from all globals to prevent elimination */
    int final_checksum = 0;
    for (int i = 0; i < 100; i++) {
        final_checksum ^= global_array[i];
    }
    final_checksum ^= global_struct.a;
    final_checksum ^= global_struct.b;
    final_checksum ^= (int)global_struct.c;
    final_checksum ^= global_struct.d;
    
    printf("Result: %d\n", final_checksum);
    return final_checksum & 0xFF;
}

/* Dummy definitions to satisfy external references */
void use(int x) { volatile int dummy = x; (void)dummy; }
void sink(int x) { volatile int dummy = x; (void)dummy; }
void escape(void* p) { volatile void* dummy = p; (void)dummy; }
