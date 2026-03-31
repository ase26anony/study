/* Built-in function reference test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away calls */
volatile int global_counter = 0;
volatile int use_builtin = 1;

/* Helper function to create additional scope */
__attribute__((noinline))
static void call_builtin_via_pointer(void) {
    /* Take address of undeclared built-in */
#ifdef __x86_64__
    void (*fp)(void) = (void (*)(void))__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    void (*fp)(void) = (void (*)(void))__builtin_arm_rbit;
#else
    void (*fp)(void) = (void (*)(void))__builtin_expect;
#endif
    
    /* Call through function pointer */
    if (global_counter & 1) {
        fp();
    }
}

/* Another helper with different built-in */
__attribute__((noinline))
static int check_constant_expression(void) {
    /* Reference __builtin_constant_p without declaration */
    int result = 0;
    
#ifdef __GNUC__
    /* Complex expression to force evaluation */
    result = __builtin_constant_p(global_counter + 42);
    
    /* Mix with inline assembly referencing built-in */
    asm volatile("# Builtin reference" : : "i"(__builtin_constant_p));
#endif
    
    return result;
}

int main(void) {
    int checksum = 0;
    volatile int i;
    
    /* Initialize volatile state */
    global_counter = rand() % 100;
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 10; i++) {
        global_counter++;
        
        /* Different paths call different undeclared built-ins */
        if (global_counter % 3 == 0) {
            /* Call __builtin_trap without declaration */
            if (use_builtin) {
                __builtin_trap();
            }
        } 
        else if (global_counter % 3 == 1) {
            /* Call __builtin_unreachable without declaration */
            if (use_builtin > 0) {
                __builtin_unreachable();
            }
        }
        else {
            /* Call __builtin_expect without declaration */
            long res = __builtin_expect(global_counter, 0);
            checksum += (int)res;
        }
        
        /* Architecture-specific built-in references */
#ifdef __x86_64__
        /* Reference x86-specific built-in without declaration */
        unsigned long long tsc = __builtin_ia32_rdtsc();
        checksum ^= (int)(tsc & 0xFFFFFFFF);
        
        /* Inline assembly with built-in reference */
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#elif defined(__arm__) || defined(__aarch64__)
        /* Reference ARM-specific built-in without declaration */
        unsigned int reversed = __builtin_arm_rbit(global_counter);
        checksum += reversed;
        
        asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
        
        /* Generic built-in references */
#ifdef __GNUC__
        /* __builtin_popcount without declaration */
        int popcnt = __builtin_popcount(global_counter);
        checksum += popcnt;
        
        /* __builtin_ffs without declaration */
        int ffs_result = __builtin_ffs(global_counter);
        checksum ^= ffs_result;
#endif
        
        /* Call helper that uses function pointers */
        call_builtin_via_pointer();
        
        /* Use another helper */
        checksum += check_constant_expression();
        
        /* Prevent infinite loops if __builtin_unreachable is optimized */
        if (global_counter > 1000) break;
    }
    
    /* Additional references in different contexts */
    {
        /* Nested block with built-in reference */
        volatile int (*fp2)(int) = (int (*)(int))__builtin_clz;
        if (global_counter & 2) {
            checksum += fp2(global_counter);
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* Force references to additional built-ins */
__attribute__((constructor))
static void init_builtin_refs(void) {
    /* Reference __builtin_return_address without declaration */
    void *ret_addr = __builtin_return_address(0);
    
    /* Reference __builtin_frame_address without declaration */
    void *frame_addr = __builtin_frame_address(0);
    
    /* Use addresses to prevent optimization */
    global_counter += ((long)ret_addr ^ (long)frame_addr) & 1;
}
