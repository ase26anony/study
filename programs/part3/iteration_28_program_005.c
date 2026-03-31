/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================================
   PHASE 1: Declare prototypes with various attribute combinations
   to increase chances of hitting the target block
   ============================================================ */

/* Prototype 1: Full attribute combination matching target block */
extern int __attribute__((visibility("hidden"), used, artificial))
__hidden_builtin_proto1(int x) __attribute__((nothrow));

/* Prototype 2: Different order of attributes */
int __attribute__((used, visibility("hidden"), artificial, extern))
__hidden_builtin_proto2(float y) __attribute__((nothrow));

/* Prototype 3: With volatile qualifier simulation */
extern void __attribute__((visibility("hidden")))
__hidden_builtin_proto3(void *ptr) __attribute__((nothrow, artificial, used));

/* Prototype 4: Minimal attributes, let compiler add others */
int __attribute__((visibility("hidden")))
__hidden_builtin_proto4(void);

/* ============================================================
   PHASE 2: Target-specific built-in declarations
   These should trigger TARGET_BUILTIN_DECL hooks
   ============================================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
extern unsigned long long __attribute__((visibility("hidden")))
__builtin_ia32_rdtsc(void) __attribute__((nothrow, used, artificial));

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_sfence(void) __attribute__((nothrow));

extern int __attribute__((visibility("hidden")))
__builtin_ia32_addss(int a, int b) __attribute__((nothrow, used, artificial));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
extern unsigned int __attribute__((visibility("hidden")))
__builtin_arm_rbit(unsigned int val) __attribute__((nothrow, used, artificial));

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int) __attribute__((nothrow));

extern int __attribute__((visibility("hidden")))
__builtin_arm_qadd(int a, int b) __attribute__((nothrow, used, artificial));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
extern unsigned int __attribute__((visibility("hidden")))
__builtin_ppc_mftb(void) __attribute__((nothrow, used, artificial));

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_sync(void) __attribute__((nothrow));

#else
/* Generic fallback - use GCC generic built-ins */
extern void * __attribute__((visibility("hidden"), used, artificial))
__builtin_return_address(unsigned int level) __attribute__((nothrow));

extern int __attribute__((visibility("hidden")))
__builtin_popcount(unsigned int x) __attribute__((nothrow, used, artificial));
#endif

/* ============================================================
   PHASE 3: Function pointer array with volatile qualifiers
   to prevent optimization removal
   ============================================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointers - compiler cannot optimize these away */
volatile func_ptr_t volatile_fp1 = 0;
volatile func_ptr_t volatile_fp2 = 0;
volatile void_func_ptr_t volatile_fp3 = 0;

/* Array of volatile function pointers */
volatile void * volatile_func_ptrs[4] = {0};

/* ============================================================
   PHASE 4: Main function with non-optimizable logic
   ============================================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to create compile-time unknown value */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = atoi(argv[1]) & 1;
    }
    
    /* Initialize global_seed from argv to prevent constant propagation */
    if (argc > 2) {
        global_seed = atoi(argv[2]);
    }
    
    /* ============================================================
       Assign built-in addresses to volatile pointers
       This forces compiler to process the built-in declarations
       ============================================================ */
    
#if defined(__i386__) || defined(__x86_64__)
    volatile_fp1 = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_fp2 = (func_ptr_t)__builtin_ia32_addss;
    volatile_fp3 = (void_func_ptr_t)__builtin_ia32_sfence;
    
    volatile_func_ptrs[0] = (void *)__builtin_ia32_rdtsc;
    volatile_func_ptrs[1] = (void *)__builtin_ia32_addss;
    volatile_func_ptrs[2] = (void *)__builtin_ia32_sfence;
    
#elif defined(__arm__) || defined(__aarch64__)
    volatile_fp1 = (func_ptr_t)__builtin_arm_rbit;
    volatile_fp2 = (func_ptr_t)__builtin_arm_qadd;
    volatile_fp3 = (void_func_ptr_t)__builtin_arm_dmb;
    
    volatile_func_ptrs[0] = (void *)__builtin_arm_rbit;
    volatile_func_ptrs[1] = (void *)__builtin_arm_qadd;
    volatile_func_ptrs[2] = (void *)__builtin_arm_dmb;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_fp1 = (func_ptr_t)__builtin_ppc_mftb;
    volatile_fp3 = (void_func_ptr_t)__builtin_ppc_sync;
    
    volatile_func_ptrs[0] = (void *)__builtin_ppc_mftb;
    volatile_func_ptrs[1] = (void *)__builtin_ppc_sync;
    
#else
    volatile_fp1 = (func_ptr_t)__builtin_popcount;
    volatile_fp3 = (void_func_ptr_t)__builtin_return_address;
    
    volatile_func_ptrs[0] = (void *)__builtin_popcount;
    volatile_func_ptrs[1] = (void *)__builtin_return_address;
#endif
    
    /* Also assign our prototype addresses */
    volatile_func_ptrs[3] = (void *)__hidden_builtin_proto1;
    
    /* ============================================================
       Non-optimizable conditional using volatile pointers
       ============================================================ */
    
    /* Create a value that compiler cannot predict */
    int unpredictable = global_seed + (int)((long)volatile_func_ptrs[0] & 1);
    
    /* Loop through function pointers with opaque operations */
    for (int i = 0; i < 4; i++) {
        if (volatile_func_ptrs[i]) {
            /* Opaque operation - address comparison that can't be optimized */
            unpredictable += (int)((long)volatile_func_ptrs[i] & 3);
        }
    }
    
    /* Conditional that depends on runtime values */
    if (use_builtin && unpredictable > 0) {
        /* This branch may or may not be taken at runtime */
        if (volatile_fp1) {
            /* Call through volatile pointer - compiler must keep the symbol */
            result = volatile_fp1(unpredictable);
        }
        if (volatile_fp3) {
            volatile_fp3();
        }
    } else {
        /* Alternative path that also references built-ins */
        if (volatile_fp2) {
            result = volatile_fp2(result);
        }
    }
    
    /* Additional opaque use of function addresses */
    result += (int)((long)volatile_fp1 ^ (long)volatile_fp2);
    result += (int)((long)volatile_fp3);
    
    /* Mix in prototype addresses */
    result += (int)((long)__hidden_builtin_proto1 & 1);
    result += (int)((long)__hidden_builtin_proto2 & 1);
    result += (int)((long)__hidden_builtin_proto3 & 1);
    result += (int)((long)__hidden_builtin_proto4 & 1);
    
    /* Return value depends on all the above, preventing dead code elimination */
    return result & 0xFF;
}

/* ============================================================
   PHASE 5: Additional functions that reference the built-ins
   to ensure they're processed in different contexts
   ============================================================ */

/* Helper function that takes address of built-ins */
static void __attribute__((used))
take_builtin_addresses(void) {
    void *addrs[8];
    int idx = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    addrs[idx++] = (void *)__builtin_ia32_rdtsc;
    addrs[idx++] = (void *)__builtin_ia32_sfence;
    addrs[idx++] = (void *)__builtin_ia32_addss;
#elif defined(__arm__) || defined(__aarch64__)
    addrs[idx++] = (void *)__builtin_arm_rbit;
    addrs[idx++] = (void *)__builtin_arm_dmb;
    addrs[idx++] = (void *)__builtin_arm_qadd;
#endif
    
    addrs[idx++] = (void *)__hidden_builtin_proto1;
    addrs[idx++] = (void *)__hidden_builtin_proto2;
    addrs[idx++] = (void *)__hidden_builtin_proto3;
    addrs[idx++] = (void *)__hidden_builtin_proto4;
    
    /* Opaque use of addresses */
    volatile int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += (int)((long)addrs[i] & 1);
    }
    global_seed += sum;
}

/* Constructor that runs before main */
static void __attribute__((constructor, used))
init_builtin_refs(void) {
    take_builtin_addresses();
}
