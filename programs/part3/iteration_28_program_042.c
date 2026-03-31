/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed & 1;
}

/* ==============================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================== */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
    __hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_2(int x, int y) __asm__("__hidden_builtin_2");

/* Prototype 3: Used + Hidden + Artificial */
int __attribute__((visibility("hidden"), used, artificial))
    __hidden_builtin_3(void) __asm__("__hidden_builtin_3");

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
    __hidden_builtin_4(long x) __asm__("__hidden_builtin_4");

/* ==============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================== */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Declare x86-specific built-ins with hidden visibility */
extern long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden")))
    __builtin_ia32_mfence(void);

extern unsigned int __attribute__((visibility("hidden"), used))
    __builtin_ia32_crc32qi(unsigned int, unsigned char);

/* x86 SIMD built-in */
extern int __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_addss(int, int);

#endif /* x86/x86_64 */

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

/* ARM CRC built-in */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_crc32b(unsigned int, unsigned char);

/* ARM barrier built-in */
extern void __attribute__((visibility("hidden")))
    __builtin_arm_dmb(unsigned int);

#endif /* ARM */

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PPC altivec built-in */
extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_altivec_vaddubm(int, int);

/* PPC mtfsf built-in */
extern void __attribute__((visibility("hidden")))
    __builtin_set_fpscr_rn(int);

#endif /* PowerPC */

/* ==============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================== */

/* Typedef for function pointers */
typedef int (*func_ptr_int_t)(int);
typedef void (*func_ptr_void_t)(void);
typedef unsigned int (*func_ptr_crc_t)(unsigned int, unsigned char);

/* Volatile function pointers to prevent optimization */
volatile func_ptr_int_t volatile_fp1 = 0;
volatile func_ptr_int_t volatile_fp2 = 0;
volatile func_ptr_void_t volatile_fp3 = 0;
volatile func_ptr_crc_t volatile_fp4 = 0;

/* Array of function pointers for iteration */
static void* func_ptrs[8] = {0};

/* Opaque operation that compiler can't optimize away */
static int perform_opaque_operation(int input) {
    volatile int result = 0;
    for (int i = 0; i < 4; i++) {
        result ^= (input >> i) & 1;
    }
    return result;
}

/* ==============================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================== */

int main(int argc, char *argv[]) {
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Initialize volatile function pointers with built-in addresses */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    /* x86 built-ins */
    volatile_fp1 = (func_ptr_int_t)__builtin_ia32_addss;
    volatile_fp4 = (func_ptr_crc_t)__builtin_ia32_crc32qi;
    
    /* Store in array for iteration */
    func_ptrs[0] = (void*)__builtin_ia32_rdtsc;
    func_ptrs[1] = (void*)__builtin_ia32_mfence;
    func_ptrs[2] = (void*)__builtin_ia32_crc32qi;
    func_ptrs[3] = (void*)__builtin_ia32_addss;
    
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    /* ARM built-ins */
    volatile_fp4 = (func_ptr_crc_t)__builtin_arm_crc32b;
    
    func_ptrs[0] = (void*)__builtin_arm_crc32b;
    func_ptrs[1] = (void*)__builtin_arm_dmb;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC built-ins */
    volatile_fp1 = (func_ptr_int_t)__builtin_altivec_vaddubm;
    
    func_ptrs[0] = (void*)__builtin_altivec_vaddubm;
    func_ptrs[1] = (void*)__builtin_set_fpscr_rn;
#endif
    
    /* Also store our hidden prototype addresses */
    func_ptrs[4] = (void*)__hidden_builtin_1;
    func_ptrs[5] = (void*)__hidden_builtin_2;
    func_ptrs[6] = (void*)__hidden_builtin_3;
    func_ptrs[7] = (void*)__hidden_builtin_4;
    
    /* Runtime-dependent conditional that can't be optimized away */
    int runtime_choice = get_runtime_value();
    
    /* Perform opaque operations on function pointer array */
    int checksum = 0;
    for (int i = 0; i < 8; i++) {
        if (func_ptrs[i]) {
            /* Create opaque dependency on function pointer address */
            checksum ^= ((unsigned long)func_ptrs[i] >> 4) & 0xFF;
            checksum = perform_opaque_operation(checksum);
        }
    }
    
    /* Conditional that depends on runtime value and checksum */
    if ((runtime_choice ^ (checksum & 1)) == 0) {
        /* This path might call through volatile function pointer */
        if (volatile_fp1) {
            /* Create artificial use that compiler must keep */
            int dummy = (int)(unsigned long)volatile_fp1;
            printf("Function pointer value influenced checksum: %d\n", 
                   checksum ^ dummy);
        }
    } else {
        printf("Alternative path with checksum: %d\n", checksum);
    }
    
    /* Additional artificial use of built-in prototypes */
    volatile int artificial_use = 0;
    artificial_use += (int)(unsigned long)__hidden_builtin_1;
    artificial_use += (int)(unsigned long)__hidden_builtin_2;
    artificial_use += (int)(unsigned long)__hidden_builtin_3;
    artificial_use += (int)(unsigned long)__hidden_builtin_4;
    
    if (artificial_use != 0) {
        printf("Built-in prototypes referenced\n");
    }
    
    return checksum & 0x7F;  /* Return non-zero exit code */
}

/* ==============================================
   DUMMY IMPLEMENTATIONS (if not built-in)
   ============================================== */

/* Provide weak implementations in case compiler doesn't provide built-ins */
int __attribute__((weak, visibility("hidden")))
__hidden_builtin_1(int x) {
    return x + 1;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_2(int x, int y) {
    return x + y;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_3(void) {
    return global_seed;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_4(long x) {
    return (int)(x & 0xFFFFFFFF);
}
