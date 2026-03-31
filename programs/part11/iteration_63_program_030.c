/* test_resource_patterns.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* External functions to prevent optimization */
extern void use(int);
extern void sink(void*);
extern int volatile_input(void);

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Global variables for memory patterns */
int glob_array[100];
volatile int glob_seed;

/* Pattern 1: ZERO_EXTRACT destination via bitfield operations */
NOINLINE void test_zero_extract(volatile int seed) {
    /* Method 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low: 8;
            unsigned int mid: 8;
            unsigned int high: 16;
        } bits;
    } u;
    
    u.full = seed;
    /* This assignment to bitfield may generate ZERO_EXTRACT */
    u.bits.mid = volatile_input() & 0xFF;
    use(u.full);
    
    /* Method 2: Explicit bitwise operations */
    unsigned int val = seed;
    unsigned int mask = 0xFF00;
    unsigned int insert = (volatile_input() & 0xFF) << 8;
    /* May generate: (set (zero_extract:SI (reg:SI X) (const_int 8) (const_int 8))
     *                 (and:SI (reg:SI Y) (const_int 255))) */
    val = (val & ~mask) | insert;
    use(val);
    
    /* Method 3: Nested bitfields in loop */
    for (int i = 0; i < (seed & 3); i++) {
        union {
            uint32_t dword;
            struct {
                uint16_t low;
                uint16_t high;
            } words;
        } v;
        v.dword = seed + i;
        v.words.high = volatile_input() & 0xFFFF;
        use(v.dword);
    }
}

/* Pattern 2: STRICT_LOW_PART destination */
NOINLINE void test_strict_low_part(volatile int seed) {
    /* Method 1: Short to int assignment */
    int large = seed * 1000;
    short small = volatile_input() & 0x7FFF;
    
    /* May generate: (set (strict_low_part (reg:SI X)) (reg:HI Y)) */
    large = (large & ~0xFFFF) | (small & 0xFFFF);
    use(large);
    
    /* Method 2: Pointer casting */
    int val = seed;
    short *ps = (short*)&val;
    *ps = volatile_input() & 0xFFFF;
    use(val);
    
    /* Method 3: In loop with condition */
    for (int i = 0; i < (seed & 7); i++) {
        int temp = seed + i * 100;
        if (volatile_input() & 1) {
            short s = volatile_input() & 0xFF;
            temp = (temp & ~0xFF) | s;
        }
        use(temp);
    }
    
    /* Method 4: Through function argument */
    int data = seed;
    short update = volatile_input() & 0x7FFF;
    /* Force computation that keeps low part only */
    data = data ^ 0x12345678;
    data = (data & ~0xFFFF) | update;
    use(data);
}

/* Pattern 3: SUBREG destination */
NOINLINE void test_subreg(volatile int seed) {
    /* Method 1: Type punning with different sizes */
    long long big = (long long)seed * 1000000LL;
    int *p32 = (int*)&big;
    
    /* May generate: (set (subreg:SI (reg:DI X) 0) (reg:SI Y)) */
    *p32 = volatile_input();
    use((int)big);
    
    /* Method 2: Array with short accesses */
    int array[4] = {seed, seed+1, seed+2, seed+3};
    volatile int idx = seed & 3;
    short *ps = (short*)&array[idx];
    
    for (int i = 0; i < (seed & 3); i++) {
        *ps = volatile_input() & 0xFFFF;
        ps += 1;  /* Move to next short */
        use(array[i]);
    }
    
    /* Method 3: Structure with mixed types */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.i = seed;
    short *sptr = &m.s;
    *sptr = volatile_input() & 0xFFFF;
    use(m.i);
    
    /* Method 4: Complex pointer arithmetic */
    char buffer[100];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i;
    }
    
    int *alias = (int*)&buffer[seed % (sizeof(buffer) - 3)];
    *alias = volatile_input();
    use(buffer[0]);
}

/* Pattern 4: Complex MEM destinations */
NOINLINE void test_complex_mem(volatile int seed) {
    /* Method 1: Structure with volatile offset */
    struct point {
        int x;
        int y;
        int z;
    } pt = {seed, seed+1, seed+2};
    
    volatile int off = seed & 1;
    int *ptr = &pt.x + off;
    *ptr = volatile_input();
    use(pt.x + pt.y);
    
    /* Method 2: Global array with index */
    volatile int idx = seed % 50;
    glob_array[idx] = volatile_input();
    glob_array[idx + 1] = glob_array[idx] * 2;
    use(glob_array[0]);
    
    /* Method 3: Pointer to pointer */
    int val1 = seed;
    int val2 = seed * 2;
    int *p1 = &val1;
    int *p2 = &val2;
    
    volatile int choice = seed & 1;
    int **pp = choice ? &p1 : &p2;
    **pp = volatile_input();
    use(val1 + val2);
    
    /* Method 4: Computed address with scaling */
    int base[10];
    for (int i = 0; i < 10; i++) {
        base[i] = i;
    }
    
    volatile int scale = (seed & 3) + 1;
    int *addr = &base[0] + (seed % 8) * scale;
    *addr = volatile_input();
    
    /* Use all elements to keep them alive */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += base[i];
    }
    use(sum);
    
    /* Method 5: Memory through frame pointer relative */
    int local = seed;
    int *dynamic = &local + (seed & 1);
    *dynamic = volatile_input();
    use(local);
}

/* Pattern 5: Combined patterns in complex control flow */
NOINLINE void test_combined(volatile int seed) {
    int result = 0;
    
    for (int i = 0; i < (seed & 15); i++) {
        volatile int mode = (seed + i) & 3;
        
        switch (mode) {
            case 0: {
                /* ZERO_EXTRACT pattern */
                union {
                    uint32_t val;
                    struct {
                        uint16_t a;
                        uint16_t b;
                    } parts;
                } u;
                u.val = seed + i;
                u.parts.a = volatile_input() & 0xFFFF;
                result += u.val;
                break;
            }
            
            case 1: {
                /* STRICT_LOW_PART pattern */
                int temp = seed * i;
                short s = volatile_input() & 0x7FFF;
                temp = (temp & ~0xFFFF) | s;
                result += temp;
                break;
            }
            
            case 2: {
                /* SUBREG pattern */
                long long big = seed * 1000LL;
                int *p = (int*)&big + (i & 1);
                *p = volatile_input();
                result += (int)big;
                break;
            }
            
            case 3: {
                /* MEM pattern */
                int array[4] = {0};
                volatile int idx = i & 3;
                int *ptr = &array[0] + idx;
                *ptr = volatile_input();
                result += array[idx];
                break;
            }
        }
    }
    
    use(result);
}

/* Main driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seed */
    volatile int seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = time(NULL);
    }
    
    glob_seed = seed;
    
    /* Call all test patterns */
    test_zero_extract(seed);
    test_strict_low_part(seed + 1);
    test_subreg(seed + 2);
    test_complex_mem(seed + 3);
    test_combined(seed + 4);
    
    /* Create a checksum from global array */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= glob_array[i];
    }
    
    printf("Result: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy external references */
void use(int x) {
    glob_array[0] ^= x;
}

int volatile_input(void) {
    static int counter = 0;
    return glob_seed + (counter++);
}
