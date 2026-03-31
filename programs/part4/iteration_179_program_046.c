/* test_cache_kernel.c - Cache detection stress test */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force CPUID usage through builtins */
__attribute__((noinline)) 
int detect_cpu_features(void) {
    int features = 0;
    
    /* Test various CPUID checks that require cache initialization */
    if (__builtin_cpu_supports("sse")) features |= 1;
    if (__builtin_cpu_supports("sse2")) features |= 2;
    if (__builtin_cpu_supports("sse3")) features |= 4;
    if (__builtin_cpu_supports("ssse3")) features |= 8;
    if (__builtin_cpu_supports("sse4.1")) features |= 16;
    if (__builtin_cpu_supports("sse4.2")) features |= 32;
    if (__builtin_cpu_supports("avx")) features |= 64;
    if (__builtin_cpu_supports("avx2")) features |= 128;
    
    /* CPU vendor checks */
    if (__builtin_cpu_is("intel")) features |= 256;
    if (__builtin_cpu_is("amd")) features |= 512;
    
    return features;
}

/* Cache-sensitive computation */
__attribute__((noinline))
double cache_sensitive_work(int size) {
    volatile double *array = malloc(size * sizeof(double));
    double sum = 0.0;
    
    if (!array) return 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        array[i] = (i % 7) * 0.1;
    }
    
    /* Multiple passes to stress cache */
    for (int pass = 0; pass < 100; pass++) {
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    free((void*)array);
    return sum;
}

/* Architecture-specific function variants */
#ifdef ARCH_NEHALEM
__attribute__((target("arch=nehalem")))
void arch_specific_nehalem(void) {
    printf("Nehalem-specific code path\n");
}
#endif

#ifdef ARCH_SANDYBRIDGE
__attribute__((target("arch=sandybridge")))
void arch_specific_sandybridge(void) {
    printf("Sandy Bridge-specific code path\n");
}
#endif

#ifdef ARCH_HASWELL
__attribute__((target("arch=haswell")))
void arch_specific_haswell(void) {
    printf("Haswell-specific code path\n");
}
#endif

#ifdef ARCH_SKYLAKE
__attribute__((target("arch=skylake")))
void arch_specific_skylake(void) {
    printf("Skylake-specific code path\n");
}
#endif

#ifdef ARCH_ZEN
__attribute__((target("arch=znver1")))
void arch_specific_zen(void) {
    printf("Zen-specific code path\n");
}
#endif

int main(int argc, char **argv) {
    int size = 10000;
    if (argc > 1) size = atoi(argv[1]);
    
    printf("CPU Features: 0x%x\n", detect_cpu_features());
    
    /* Call architecture-specific functions if compiled */
#ifdef ARCH_NEHALEM
    arch_specific_nehalem();
#endif
#ifdef ARCH_SANDYBRIDGE
    arch_specific_sandybridge();
#endif
#ifdef ARCH_HASWELL
    arch_specific_haswell();
#endif
#ifdef ARCH_SKYLAKE
    arch_specific_skylake();
#endif
#ifdef ARCH_ZEN
    arch_specific_zen();
#endif
    
    double result = cache_sensitive_work(size);
    printf("Result: %f\n", result);
    
    return 0;
}
