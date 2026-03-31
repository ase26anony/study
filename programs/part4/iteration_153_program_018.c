/* test_emutls.c
 * 
 * This test program is designed to trigger TLS emulation in GCC's tree-emutls pass
 * and specifically exercise the attribute copying logic for various TLS variable
 * attributes. Compile with options that force TLS emulation (see below).
 */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static TLS with initialization */
static __thread int tls_static_init = 42;

/* Pattern B: Extern TLS with common linkage */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* definition */

/* Pattern C: Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;  /* actual definition */
#else
/* On non-Windows, use weak attribute to simulate similar linkage */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 5000;
#endif

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (p1 != NULL);
    sink = (p2 != NULL);
    sink = (p3 != NULL);
    sink = (p4 != NULL);
    sink = (p5 != NULL);
    sink = (p6 != NULL);
}

/* Another helper to use TLS values in computation */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computation */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    sum += tls_imported % 100;
    
    /* Make result dependent on argc */
    return sum * (argc + 1);
}

int main(int argc, char **argv) {
    int result;
    
    /* Modify TLS variables based on argc to create dynamic behavior */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    tls_imported = argc * 5000;
    
    /* Take addresses of all TLS variables to force emulation code generation */
    use_tls_pointers(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported);
    
    /* Use TLS values in computation with control flow */
    result = compute_checksum(argc);
    
    /* Conditional use to prevent dead code elimination */
    if (result > 1000) {
        printf("TLS result: %d\n", result / 100);
    } else {
        printf("TLS result: %d\n", result);
    }
    
    /* Loop that depends on TLS values */
    for (int i = 0; i < (tls_static_init % 5); i++) {
        tls_weak += i;
    }
    
    return result > 0 ? 0 : 1;
}

/* Additional tentative definition to test DECL_COMMON behavior */
__thread int tls_common;  /* tentative definition */

/* Force reference to tls_common */
__attribute__((constructor))
static void init_tls_common(void) {
    tls_common = 12345;
}
