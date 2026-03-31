/* test-tls-emutls-attr-copy.c
 * 
 * This program is designed to trigger TLS emulation scenarios that require
 * copying declaration attributes between TLS variables, specifically targeting
 * the uncovered lines in tree-emutls.cc (lines 295-304).
 *
 * Compile with: gcc -O2 -femulated-tls -fvisibility=hidden -fPIC -fdump-tree-all -o test test.c
 * Or for cross-compilation testing: <target>-gcc -O2 -femulated-tls ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque function declarations to prevent optimization */
extern void use_ptr(void *p);
extern int opaque_int(void);
extern void *opaque_ptr(void);

/* Define these as empty functions when linking */
#ifdef LINK_TEST
void use_ptr(void *p) { (void)p; }
int opaque_int(void) { return 42; }
void *opaque_ptr(void) { return NULL; }
#endif

/* ================= TLS VARIABLES WITH DIVERSE ATTRIBUTES ================= */

/* Public TLS with default visibility */
__thread int tls_public = 10;
__thread int tls_public_uninit;

/* Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 20;

/* Hidden visibility TLS */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 30;

/* Internal visibility TLS */
__thread int tls_internal __attribute__((visibility("internal"))) = 40;

/* Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 50;

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with attribute if supported */
#if __has_attribute(dllimport)
__thread int tls_dllimport __attribute__((dllimport));
#else
__thread int tls_dllimport = 60;
#endif
#endif

/* Common TLS symbol (tentative definition) */
__thread int tls_common;

/* Static TLS (not public) with complex initialization */
static __thread int tls_static = 70;
static __thread int tls_static_dyn = 0;

/* TLS variable whose address escapes - should preserve DECL_PRESERVE_P */
__thread int tls_preserve __attribute__((used)) = 80;

/* External TLS declarations (simulating another compilation unit) */
extern __thread int tls_extern;
extern __thread int tls_extern_hidden __attribute__((visibility("hidden")));

/* TLS with alignment requirement that might affect emulation */
__thread int tls_aligned __attribute__((aligned(64))) = 90;

/* Volatile TLS to prevent optimization */
volatile __thread int tls_volatile = 100;

/* ================= FUNCTION DECLARATIONS ================= */

static inline int inline_tls_access(int idx) {
    /* Inline function accessing TLS - may trigger declaration copying */
    static __thread int tls_inline_counter = 0;
    tls_inline_counter++;
    
    switch (idx % 4) {
        case 0: return tls_public + tls_inline_counter;
        case 1: return tls_hidden - tls_inline_counter;
        case 2: return tls_static * tls_inline_counter;
        case 3: return tls_preserve / (tls_inline_counter ? tls_inline_counter : 1);
        default: return tls_inline_counter;
    }
}

/* Function that takes address of TLS and uses it in ways that might
 * force emulation structure creation */
static void manipulate_tls_addresses(int seed) {
    void *tls_addresses[12];
    volatile void *volatile_tls_ptr;
    
    /* Take addresses of various TLS variables */
    tls_addresses[0] = &tls_public;
    tls_addresses[1] = &tls_weak;
    tls_addresses[2] = &tls_hidden;
    tls_addresses[3] = &tls_internal;
    tls_addresses[4] = &tls_protected;
    tls_addresses[5] = &tls_static;
    tls_addresses[6] = &tls_preserve;
    tls_addresses[7] = &tls_aligned;
    tls_addresses[8] = (void*)&tls_volatile;
    tls_addresses[9] = &tls_common;
    tls_addresses[10] = &tls_static_dyn;
    
    /* Force address into volatile pointer */
    vol_tls_ptr = tls_addresses[seed % 11];
    
    /* Pass addresses to opaque function to prevent optimization */
    for (int i = 0; i < 11; i++) {
        use_ptr(tls_addresses[i]);
    }
    
    /* Use inline function that accesses TLS */
    int inline_result = inline_tls_access(seed);
    tls_static_dyn = inline_result;
    
    /* Complex expression with TLS address arithmetic */
    if (seed & 1) {
        int *ptr = (int*)tls_addresses[seed % 5];
        *ptr += opaque_int();
    }
}

/* Function with loop that accesses TLS variables */
static int process_tls_variables(int iterations, int seed) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix TLS and non-TLS accesses in loop */
        int val = 0;
        
        /* Access different TLS variables based on seed and loop counter */
        switch ((i + seed) % 8) {
            case 0:
                val = tls_public++;
                tls_common += val;
                break;
            case 1:
                val = tls_hidden--;
                tls_static ^= val;
                break;
            case 2:
                val = tls_internal;
                tls_internal = val * 2;
                break;
            case 3:
                val = tls_protected;
                tls_protected = val / 2;
                break;
            case 4:
                val = tls_weak;
                tls_weak = val + i;
                break;
            case 5:
                val = tls_preserve;
                tls_preserve = val - seed;
                break;
            case 6:
                val = tls_aligned;
                tls_aligned = val | 0xFF;
                break;
            case 7:
                val = tls_volatile;
                tls_static_dyn = val & 0x7F;
                break;
        }
        
        sum += val;
        sum += inline_tls_access(i);
    }
    
    return sum;
}

/* Initialize some TLS variables dynamically */
static void init_dynamic_tls(void) {
    tls_static_dyn = opaque_int();
    tls_common = tls_public + tls_hidden;
    
    /* Force address taking of TLS with asm to ensure DECL_PRESERVE_P */
    __asm__ volatile ("# TLS preserve marker %0" : : "r"(&tls_preserve));
}

/* ================= MAIN FUNCTION ================= */

int main(int argc, char **argv) {
    int seed = 0;
    
    /* Use argv for unpredictable control flow */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = (int)opaque_ptr();
    }
    
    /* Initialize TLS variables */
    init_dynamic_tls();
    
    /* Define the extern TLS variables (simulating linkage from another file) */
    __thread int tls_extern = 200;
    __thread int tls_extern_hidden __attribute__((visibility("hidden"))) = 210;
    
    /* Manipulate TLS addresses to force emulation structures */
    manipulate_tls_addresses(seed);
    
    /* Process TLS variables with loops and conditionals */
    int iterations = (seed % 20) + 5;
    int checksum = process_tls_variables(iterations, seed);
    
    /* Additional complex TLS usage pattern */
    {
        int *ptr_array[4];
        ptr_array[0] = &tls_public;
        ptr_array[1] = &tls_hidden;
        ptr_array[2] = &tls_static;
        ptr_array[3] = &tls_preserve;
        
        for (int i = 0; i < 4; i++) {
            *ptr_array[i] += checksum % 17;
            checksum += *ptr_array[i];
        }
    }
    
    /* Final checksum computation using all TLS variables */
    checksum += tls_public;
    checksum += tls_weak;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_dllimport;
    checksum += tls_common;
    checksum += tls_static;
    checksum += tls_static_dyn;
    checksum += tls_preserve;
    checksum += tls_extern;
    checksum += tls_extern_hidden;
    checksum += tls_aligned;
    checksum += tls_volatile;
    
    printf("TLS checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* ================= SEPARATE COMPILATION UNIT SIMULATION ================= */

/* To fully test external TLS linkage, compile this separately:
 * 
 * // tls-extern.c
 * __thread int tls_extern = 200;
 * __thread int tls_extern_hidden __attribute__((visibility("hidden"))) = 210;
 * 
 * And link with: gcc -O2 -femulated-tls -fPIC test.c tls-extern.c
 */
