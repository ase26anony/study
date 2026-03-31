/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern/public TLS variable with tentative definition */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate with weak external on non-Windows */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Provide definition for tls_imported */
__thread int tls_imported = 999;

/* Additional TLS variable with complex type */
typedef struct {
    int a;
    double b;
    char c[4];
} ComplexTLS;
__thread ComplexTLS tls_complex = {1, 3.14, "TLS"};

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void* p1, void* p2, void* p3, void* p4, void* p5, void* p6) {
    /* Dummy writes to prevent optimization */
    volatile int* vp;
    
    if (p1) {
        vp = (volatile int*)p1;
        *vp = *vp + 1;
    }
    if (p2) {
        vp = (volatile int*)p2;
        *vp = *vp + 1;
    }
    /* Just reference the pointers to ensure they're used */
    (void)p3;
    (void)p4;
    (void)p5;
    (void)p6;
}

/* Another helper that returns a value based on TLS variables */
__attribute__((noinline))
static int compute_from_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak % 10;
    result += (int)(tls_hidden % 100);
    result += (int)(tls_protected % 100);
    result += tls_imported % 10;
    result += tls_complex.a;
    
    /* Make result dependent on argc */
    if (argc > 1) {
        tls_static_init = argc;
        tls_extern = 'A' + (argc % 26);
        tls_weak = argc * 10;
        tls_hidden = argc * 100L;
        tls_protected = argc * 200L;
        tls_imported = argc * 5;
        tls_complex.a = argc;
    }
    
    return result;
}

/* Function to ensure TLS variables are referenced in different scopes */
static void modify_tls_values(int val) {
    /* Modify static TLS */
    tls_static_init += val;
    
    /* Modify extern TLS */
    tls_extern = (tls_extern + val) % 128;
    
    /* Modify weak TLS if defined locally */
    if (&tls_weak) {
        tls_weak ^= val;
    }
    
    /* Modify visibility TLS variables */
    tls_hidden |= val;
    tls_protected &= ~val;
    
    /* Modify imported TLS */
    tls_imported *= (val + 1);
    
    /* Modify complex TLS */
    tls_complex.a += val;
    tls_complex.b += val / 100.0;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Initial computation */
    result = compute_from_tls(argc);
    
    /* Force address-taking of all TLS variables */
    use_tls_addresses(
        (void*)&tls_static_init,
        (void*)&tls_extern,
        (void*)&tls_weak,
        (void*)&tls_hidden,
        (void*)&tls_protected,
        (void*)&tls_imported
    );
    
    /* Modify values based on argc */
    modify_tls_values(argc);
    
    /* Second computation with modified values */
    result += compute_from_tls(argc);
    
    /* Use complex TLS member addresses */
    use_tls_addresses(
        (void*)&tls_complex.a,
        (void*)&tls_complex.b,
        (void*)tls_complex.c,
        NULL, NULL, NULL
    );
    
    /* Loop that uses TLS variables to prevent optimization */
    for (int i = 0; i < (argc % 5); i++) {
        tls_static_init++;
        tls_extern--;
        result += tls_weak;
    }
    
    /* Final result depends on all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    result += tls_complex.a;
    
    /* Print to prevent optimization */
    printf("Result: %d (argc=%d)\n", result, argc);
    
    return result % 256;
}

/* Additional file-scope TLS usage */
__thread int file_scope_tls = 1234;

void __attribute__((constructor)) init_tls(void) {
    /* Constructor that uses TLS */
    file_scope_tls = 5678;
}

void __attribute__((destructor)) cleanup_tls(void) {
    /* Destructor that uses TLS */
    file_scope_tls = 0;
}
