/* Main file with various TLS declarations and attribute manipulations */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force emulated TLS */
#pragma GCC tls_model emulated

/* TLS with different attributes to be copied */
/* 1. Static TLS with used attribute */
static __thread int tls_static_used __attribute__((used)) = 42;

/* 2. TLS with weak linkage */
__thread long tls_weak __attribute__((weak)) = 100;

/* 3. TLS with hidden visibility */
__thread double tls_hidden __attribute__((visibility("hidden"))) = 3.14;

/* 4. TLS with default visibility specified */
__thread char tls_visible __attribute__((visibility("default"))) = 'A';

/* 5. External TLS declaration (defined in another file) */
extern __thread int tls_extern;

/* 6. Common TLS (no initializer, might become DECL_COMMON) */
__thread int tls_common;

/* 7. Public TLS with DLL import simulation */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* Simulate with weak and used */
__thread int tls_dllimport __attribute__((weak, used));
#endif

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    srand(time(NULL));
    tls_static_used = rand() % 1000;
    tls_weak = rand() % 1000;
    tls_hidden = (double)rand() / RAND_MAX;
    tls_visible = 'A' + (rand() % 26);
    tls_common = rand() % 1000;
}

/* Opaque function that takes TLS addresses - forces address taking */
static __attribute__((noinline, used))
void manipulate_tls_addresses(int *p1, long *p2, double *p3, char *p4) {
    /* Use inline assembly to ensure addresses are used */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    
    /* Modify values through pointers */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 1.0;
    if (p4) *p4 += 1;
}

/* Another function that creates complex context for TLS */
static __attribute__((noinline))
int use_tls_in_statement_expr(void) {
    /* GNU statement expression with TLS address use */
    int result = ({
        int local_tls __thread = 99;  /* Local TLS declaration */
        int *addr = &local_tls;
        
        /* Take address in assembly to prevent optimization */
        asm volatile("" : "+r"(addr) : : "memory");
        
        /* Use external function call */
        printf("TLS address in statement expr: %p\n", (void*)addr);
        
        /* Modify and return */
        local_tls *= 2;
        local_tls;
    });
    
    return result;
}

/* Function that forces TLS declaration duplication through multiple uses */
static void force_tls_duplication(void) {
    /* Take addresses of various TLS variables */
    int *addr1 = &tls_static_used;
    long *addr2 = &tls_weak;
    double *addr3 = &tls_hidden;
    char *addr4 = &tls_visible;
    int *addr5 = &tls_extern;
    int *addr6 = &tls_common;
    int *addr7 = &tls_dllimport;
    
    /* Pass addresses to opaque function - may cause duplication */
    manipulate_tls_addresses(addr1, addr2, addr3, addr4);
    
    /* Use statement expression with TLS */
    int from_stmt = use_tls_in_statement_expr();
    
    /* Complex expression using multiple TLS variables */
    int checksum = tls_static_used + tls_weak + (int)tls_hidden + 
                   tls_visible + tls_extern + tls_common + from_stmt;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(checksum) : "memory");
    
    printf("TLS checksum: %d\n", checksum);
}

int main(void) {
    /* Local TLS variable in main */
    __thread int local_main_tls = 123;
    
    /* Force taking address in a way that might trigger duplication */
    int *local_addr = &local_main_tls;
    
    /* Use inline assembly with the address */
    asm volatile("" : "+r"(local_addr) : : "memory");
    
    /* Call function that manipulates TLS addresses */
    force_tls_duplication();
    
    /* Additional complex use of TLS addresses */
    void *addresses[] = {
        &tls_static_used,
        &tls_weak,
        &tls_hidden,
        &tls_visible,
        &tls_extern,
        &tls_common,
        &local_main_tls
    };
    
    /* Opaque use of addresses array */
    asm volatile("" : : "r"(addresses) : "memory");
    
    return 0;
}
