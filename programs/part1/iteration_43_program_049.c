/*
 * Test program to trigger delay slot filling logic in GCC's reorg.cc
 * Specifically targets lines 2135-2149 in fill_eager_delay_slots()
 */

#include <stdio.h>
#include <stdlib.h>

/* Architecture detection */
#if defined(__mips__) || defined(__sparc__) || defined(__MIPS__) || defined(__sparc)
#define HAS_DELAY_SLOTS 1
#else
#define HAS_DELAY_SLOTS 0
#endif

/* Barrier to prevent unwanted optimizations */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Test 1: Simple conditional jump with arithmetic at target */
#if HAS_DELAY_SLOTS
static int test_simple_jump_arithmetic(int a, int b)
{
    int result = a;
    
    /* Create a simple conditional jump */
    if (a > b) {
        COMPILER_BARRIER();
        goto target1;
    }
    
    /* Alternative path */
    result = b - a;
    return result;
    
target1:
    /* Candidate instruction for delay slot: simple arithmetic */
    /* Uses local variable not live across the jump */
    int temp = result + 1;
    result = temp * 2;
    return result;
}
#else
static int test_simple_jump_arithmetic(int a, int b)
{
    /* Portable version */
    return (a > b) ? ((a + 1) * 2) : (b - a);
}
#endif

/* Test 2: Unconditional jump via goto with bitwise operations */
#if HAS_DELAY_SLOTS
static int test_unconditional_jump_bitwise(int x)
{
    int val = x;
    
    if (val != 0) {
        COMPILER_BARRIER();
        goto target2;
    }
    
    return 0;
    
target2:
    /* Candidate: bitwise operations on local variable */
    /* Should not conflict with jump resources */
    val = val ^ 0xFF;
    val = val & 0x7F;
    return val;
}
#else
static int test_unconditional_jump_bitwise(int x)
{
    return (x != 0) ? ((x ^ 0xFF) & 0x7F) : 0;
}
#endif

/* Test 3: Nested conditional with safe register usage */
#if HAS_DELAY_SLOTS
static int test_nested_conditional(int a, int b, int c)
{
    int t1 = a;
    int t2 = b;
    
    /* Create more complex condition to ensure jump generation */
    if (a > 0 && b < 100) {
        if (c == 0) {
            COMPILER_BARRIER();
            goto target3;
        }
        t1 = b + c;
    }
    
    return t1 + t2;
    
target3:
    /* Candidate: arithmetic on temporaries not used before jump */
    int local1 = t1 + 5;
    int local2 = t2 * 2;
    return local1 + local2;
}
#else
static int test_nested_conditional(int a, int b, int c)
{
    if (a > 0 && b < 100 && c == 0) {
        return (a + 5) + (b * 2);
    }
    return (a > 0 && b < 100) ? (b + c) + b : a + b;
}
#endif

/* Test 4: Jump with multiple candidate instructions at target */
#if HAS_DELAY_SLOTS
static int test_multiple_candidates(int x)
{
    int result = x;
    
    switch (x & 3) {
        case 0:
            COMPILER_BARRIER();
            goto target4a;
        case 1:
            COMPILER_BARRIER();
            goto target4b;
        default:
            return x * 2;
    }
    
target4a:
    /* First candidate type: simple increment */
    result = result + 1;
    return result;
    
target4b:
    /* Second candidate type: shift operation */
    result = result << 2;
    return result;
}
#else
static int test_multiple_candidates(int x)
{
    switch (x & 3) {
        case 0: return x + 1;
        case 1: return x << 2;
        default: return x * 2;
    }
}
#endif

/* Test 5: Avoid using special registers (like $ra on MIPS) */
#if HAS_DELAY_SLOTS
static int test_safe_register_usage(int a, int b)
{
    /* Use only argument registers or fresh locals */
    int local_var1 = a;
    int local_var2 = b;
    
    if (local_var1 < local_var2) {
        COMPILER_BARRIER();
        goto target5;
    }
    
    return local_var1 - local_var2;
    
target5:
    /* Safe: only uses local variables defined before jump */
    int sum = local_var1 + local_var2;
    int diff = local_var1 - local_var2;
    return sum ^ diff;
}
#else
static int test_safe_register_usage(int a, int b)
{
    if (a < b) {
        int sum = a + b;
        int diff = a - b;
        return sum ^ diff;
    }
    return a - b;
}
#endif

/* Test 6: Function with inline asm to guide instruction selection */
#if HAS_DELAY_SLOTS && defined(__mips__)
static int test_mips_specific(int x)
{
    int result = x;
    
    /* Force a simple jump pattern with asm */
    if (result > 100) {
        /* asm barrier that doesn't use delay slot */
        __asm__ volatile(
            ".set noreorder\n\t"
            ".set nomacro\n\t"
            "b 1f\n\t"
            "nop\n\t"  /* This nop might be replaced */
            ".set macro\n\t"
            ".set reorder\n\t"
            : : : "memory"
        );
        
        /* The actual C goto for the compiler to see */
        goto mips_target;
    }
    
    return result * 3;
    
mips_target:
    /* Very simple instruction that should be delay-slot eligible */
    result = result & 0xFFFF;
    return result;
}
#elif HAS_DELAY_SLOTS && defined(__sparc__)
static int test_sparc_specific(int x)
{
    int result = x;
    
    if (result != 0) {
        /* SPARC-specific control flow */
        __asm__ volatile(
            "cmp %0, 0\n\t"
            "be 1f\n\t"
            "nop\n\t"
            : : "r"(result) : "cc", "memory"
        );
        
        goto sparc_target;
    }
    
    return 0;
    
sparc_target:
    result = result | 0x1000;
    return result;
}
#else
static int test_arch_specific(int x)
{
    /* Portable fallback */
    return (x > 100) ? (x & 0xFFFF) : (x * 3);
}
#endif

/* Main driver that calls all tests */
int main(void)
{
    int checksum = 0;
    int i;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 10;
        
        checksum += test_simple_jump_arithmetic(a, b);
        checksum += test_unconditional_jump_bitwise(a);
        checksum += test_nested_conditional(a, b, c);
        checksum += test_multiple_candidates(a);
        checksum += test_safe_register_usage(a, b);
        
#if HAS_DELAY_SLOTS
        #if defined(__mips__) || defined(__sparc__)
        checksum += test_arch_specific(a);
        #endif
#endif
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Delay slot architecture: %s\n", 
           HAS_DELAY_SLOTS ? "YES" : "NO (using portable fallbacks)");
    
    return 0;
}
