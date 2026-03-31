#include <stdio.h>
#include <string.h>

/* Helper function to generate compile-time constant indices */
static inline int const_idx(int x) { return x; }

/* Template-like macro to force constant evaluation */
#define CONST_IDX(x) (__builtin_constant_p(x) ? (x) : (x))

/* Volatile wrapper to prevent early folding */
typedef struct {
    volatile char *ptr;
} volatile_ptr;

/* Different array types to test various TYPE_SIZE scenarios */
static char char_arr[100] = {0};
static short short_arr[100] = {0};
static int int_arr[100] = {0};
static long long ll_arr[100] = {0};

/* Memory target operations (MEM_P path) */
void mem_target_ops(int lo, int hi, int count) {
    volatile_ptr vp;
    
    /* Test count <= 2 with char array (small element size) */
    if (count <= 2) {
        vp.ptr = char_arr;
        /* Memory-to-memory copy of small subrange */
        for (int i = lo; i <= hi; i++) {
            vp.ptr[i] = vp.ptr[i + 10];  /* Simple copy */
        }
    }
    
    /* Test count > 2 with different element sizes */
    if (count > 2) {
        /* With char - TYPE_SIZE * count might be small */
        vp.ptr = char_arr;
        for (int i = lo; i <= hi; i++) {
            vp.ptr[i] = i % 256;
        }
        
        /* With long long - TYPE_SIZE * count might be larger */
        volatile long long *vp_ll = ll_arr;
        for (int i = lo; i <= hi; i++) {
            vp_ll[i] = i;
        }
    }
}

/* Non-memory target operations (!MEM_P path) */
int non_mem_target_ops(int lo, int hi) {
    int sum = 0;
    
    /* Operation where target is a register (arithmetic expression) */
    for (int i = lo; i <= hi; i++) {
        sum += int_arr[i];  /* Target is sum (register), not memory */
    }
    
    /* Another register target example */
    int product = 1;
    for (int i = lo; i <= hi; i++) {
        product *= short_arr[i];
    }
    
    return sum + product;
}

/* Function using __builtin_memcpy with constant size */
void builtin_memcpy_test(int lo, int hi) {
    int count = hi - lo + 1;
    
    /* Small constant size copy */
    if (count <= 2) {
        __builtin_memcpy(&char_arr[lo], &char_arr[lo + 20], count);
    }
    
    /* Larger constant size but small element type */
    if (count > 2 && count <= 8) {
        __builtin_memcpy(&short_arr[lo], &short_arr[lo + 30], count * sizeof(short));
    }
}

int main(int argc, char **argv) {
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        char_arr[i] = i % 100;
        short_arr[i] = i;
        int_arr[i] = i * 2;
        ll_arr[i] = i * 3LL;
    }
    
    /* Use argc to create different control flow paths */
    int test_case = argc > 1 ? argv[1][0] % 4 : 0;
    
    /* Different constant index pairs to test various scenarios */
    const int const_lo = CONST_IDX(5);
    const int const_hi = CONST_IDX(6);  /* count = 2 */
    
    const int const_lo2 = CONST_IDX(10);
    const int const_hi2 = CONST_IDX(15); /* count = 6 */
    
    const int const_lo3 = CONST_IDX(20);
    const int const_hi3 = CONST_IDX(20); /* count = 1 */
    
    /* Force these to be compile-time constants */
    static const int static_lo = 30;
    static const int static_hi = 35;    /* count = 6 */
    
    /* Use volatile to inhibit early constant folding */
    volatile int vol_idx = 40;
    volatile int vol_idx2 = 45;
    
    int result = 0;
    
    switch (test_case) {
        case 0:
            /* Test MEM_P path with count <= 2 */
            mem_target_ops(const_lo, const_hi, const_hi - const_lo + 1);
            result += char_arr[const_lo];
            break;
            
        case 1:
            /* Test MEM_P path with count > 2, small element type */
            mem_target_ops(const_lo2, const_hi2, const_hi2 - const_lo2 + 1);
            result += char_arr[const_lo2];
            break;
            
        case 2:
            /* Test !MEM_P path (register targets) */
            result = non_mem_target_ops(const_lo3, const_hi3);
            break;
            
        case 3:
            /* Test with static constants */
            mem_target_ops(static_lo, static_hi, static_hi - static_lo + 1);
            /* Also test builtin_memcpy */
            builtin_memcpy_test(static_lo, static_hi);
            result += short_arr[static_lo];
            break;
            
        default:
            /* Test with volatile indices (should still work through some paths) */
            mem_target_ops(vol_idx, vol_idx2, vol_idx2 - vol_idx + 1);
            result += int_arr[vol_idx];
            break;
    }
    
    /* Additional test: mixed operations in a loop to ensure analysis */
    for (int i = 0; i < 3; i++) {
        const int loop_lo = CONST_IDX(i * 10);
        const int loop_hi = CONST_IDX(i * 10 + (i + 1));
        
        /* Alternate between memory and non-memory targets */
        if (i % 2 == 0) {
            mem_target_ops(loop_lo, loop_hi, loop_hi - loop_lo + 1);
        } else {
            result += non_mem_target_ops(loop_lo, loop_hi);
        }
    }
    
    /* Use __builtin_constant_p to verify constant propagation */
    if (__builtin_constant_p(const_lo) && __builtin_constant_p(const_hi)) {
        result |= 0x1000;  /* Mark that constants were recognized */
    }
    
    /* Ensure all operations have observable effects */
    printf("Result: %d\n", result);
    for (int i = 0; i < 10; i++) {
        printf("%d ", char_arr[i]);
    }
    printf("\n");
    
    return result != 0;
}
