/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static int use_builtins_internal(int selector) {
    volatile int local_result = 0;
    
    /* Take address of undeclared built-ins - forces external decl creation */
    void *builtin_addr;
    
#ifdef __x86_64__
    /* x86-specific built-in without prototype */
    builtin_addr = (void*)__builtin_ia32_rdtsc;
    volatile_func_ptr = builtin_addr;
#endif

#ifdef __arm__
    /* ARM-specific built-in without prototype */
    builtin_addr = (void*)__builtin_arm_rbit;
    volatile_func_ptr = builtin_addr;
#endif

#ifdef __aarch64__
    /* AArch64-specific built-in without prototype */
    builtin_addr = (void*)__builtin_aarch64_ld64b;
    volatile_func_ptr = builtin_addr;
#endif

    /* Generic built-ins without prototypes */
    switch (selector & 0x7) {
        case 0:
            /* Call undeclared built-in directly */
            local_result = __builtin_expect(global_counter > 100, 0);
            break;
        case 1:
            /* Another undeclared built-in */
            local_result = __builtin_constant_p(selector);
            break;
        case 2:
            /* Use built-in in expression */
            local_result = __builtin_popcount(selector);
            break;
        case 3:
            /* Complex built-in usage */
            local_result = __builtin_clz(selector | 1);
            break;
        case 4:
            /* Math built-in */
            local_result = __builtin_ffs(selector);
            break;
        case 5:
            /* Float built-in */
            local_result = __builtin_isnan((double)selector);
            break;
        default:
            /* Memory built-in */
            __builtin_prefetch(&global_counter, 0, 3);
            local_result = selector;
    }
    
    return local_result;
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline, hot))
static void stress_builtin_addresses(void) {
    /* Create function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    int (*expect_ptr)(long, int) = __builtin_expect;
    
    /* Use inline asm to reference built-in names directly */
#ifdef __GNUC__
    /* These asm statements create additional references to the symbols */
    asm volatile("" : : "i"(__builtin_trap));
    asm volatile("" : : "i"(__builtin_unreachable));
    asm volatile("" : : "i"(__builtin_expect));
    
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
#endif
    
    /* Store pointers to prevent optimization */
    volatile_func_ptr = trap_ptr;
    if (global_counter & 0x100) {
        volatile_func_ptr = unreachable_ptr;
    }
}

int main(void) {
    volatile int i, result = 0;
    volatile int use_trap_path = 0;
    volatile int use_unreachable_path = 0;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Loop with complex control flow around built-in calls */
    for (i = 0; i < 1000; i++) {
        global_counter++;
        
        /* Determine which path to take based on volatile state */
        use_trap_path = (global_counter % 37) == 0;
        use_unreachable_path = (global_counter % 41) == 0;
        
        if (use_trap_path) {
            /* Call undeclared built-in on one path */
            __builtin_trap();  /* This should trigger external decl creation */
        } 
        else if (use_unreachable_path && (global_counter > 500)) {
            /* Call another undeclared built-in on different path */
            __builtin_unreachable();  /* Should also trigger decl creation */
        }
        else {
            /* Normal path with various built-in usages */
            result += use_builtins_internal(global_counter);
            
            /* Periodically stress built-in address taking */
            if ((global_counter % 73) == 0) {
                stress_builtin_addresses();
            }
            
            /* Call through function pointer to undeclared built-in */
            if ((global_counter % 19) == 0) {
#ifdef __x86_64__
                /* Create and call through pointer to x86 built-in */
                unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
                if (rdtsc_ptr) {
                    result += (int)(rdtsc_ptr() & 0xFF);
                }
#endif
            }
        }
        
        /* Use __builtin_expect without prototype */
        if (__builtin_expect((global_counter & 0xFF) == 0x7F, 0)) {
            result += 2;
        }
        
        /* Use __builtin_constant_p without prototype */
        if (!__builtin_constant_p(global_counter)) {
            result -= 1;
        }
    }
    
    /* Final built-in usage to ensure all paths are covered */
    printf("Result checksum: %d\n", result + __builtin_popcount(global_counter));
    
    /* One more address-taking operation */
    void (*final_trap_ptr)(void) = __builtin_trap;
    volatile_func_ptr = final_trap_ptr;
    
    /* Use the function pointer to create a potential call site */
    if (result > 1000000) {  /* Never true, but compiler doesn't know */
        ((void (*)(void))volatile_func_ptr)();
    }
    
    return result & 0xFF;
}

/* Additional global scope references to built-ins */
#ifdef __GNUC__
/* These declarations are intentionally missing - compiler must create them */
/* The following lines create implicit declarations that should trigger
   default_builtin_extdecl when the compiler sees references to them */
extern void __builtin_trap(void);
extern void __builtin_unreachable(void);
extern long __builtin_expect(long, int);
extern int __builtin_constant_p(int);
extern int __builtin_popcount(unsigned int);
extern int __builtin_clz(unsigned int);
extern int __builtin_ffs(int);
extern int __builtin_isnan(double);
extern void __builtin_prefetch(const void *, int, int);

#ifdef __x86_64__
extern unsigned long long __builtin_ia32_rdtsc(void);
#endif

#ifdef __arm__
extern unsigned int __builtin_arm_rbit(unsigned int);
#endif
#endif
