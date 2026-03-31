/* Compile with: gcc -O2 -flto -fuse-linker-plugin target_func.c main.c -o test */

/* Force generation of multi-versioned function with internal compiler declarations */
__attribute__((target_clones("default", "avx2", "avx512f", "sse4.2")))
__attribute__((noinline, noclone, used, visibility("hidden")))
static int multi_version_func(int x, int y) {
    /* Complex enough to not be optimized away, but simple for demonstration */
    volatile int result = 0;
    
    /* Use operations that might benefit from different instruction sets */
    for (int i = 0; i < 16; i++) {
        result += x * y + i;
    }
    
    /* Atomic operation that might require helper functions */
    __atomic_add_fetch(&result, 1, __ATOMIC_SEQ_CST);
    
    return result;
}

/* Another approach: function with explicit target attribute */
__attribute__((target("avx512f"), noinline, used, visibility("hidden")))
extern int avx512_specific(int x) {
    /* Use __builtin_cpu_supports to potentially trigger internal helpers */
    if (__builtin_cpu_supports("avx512f")) {
        return x * x + 123;
    }
    return x + 456;
}

/* Force declaration with unusual linkage combination */
__attribute__((target("arch=armv8-a+crc"), noinline, used))
static int crc_checksum(const char *data, int len) {
    /* This might trigger creation of CRC helper functions */
    unsigned int crc = 0;
    for (int i = 0; i < len; i++) {
        crc = __builtin_ia32_crc32qi(crc, data[i]);
    }
    return crc;
}

/* Interface function that will be called from main */
int get_computation(int mode, int value) {
    switch (mode) {
        case 0:
            return multi_version_func(value, value + 1);
        case 1:
            return avx512_specific(value);
        case 2:
            return crc_checksum((const char*)&value, sizeof(value));
        default:
            return value;
    }
}
