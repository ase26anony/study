#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global arrays with volatile sections to prevent optimization */
#define ARRAY_SIZE 512
static int32_t global_int_array[ARRAY_SIZE];
static float global_float_array[ARRAY_SIZE];
static volatile int32_t volatile_mask[32]; /* Volatile to prevent constant folding */

/* Vector type definitions using GCC extensions */
typedef int32_t v8si __attribute__((vector_size(32)));      /* 256-bit integer */
typedef int32_t v16si __attribute__((vector_size(64)));     /* 512-bit integer */
typedef float v8sf __attribute__((vector_size(32)));        /* 256-bit float */
typedef float v16sf __attribute__((vector_size(64)));       /* 512-bit float */
typedef double v4df __attribute__((vector_size(32)));       /* 256-bit double */
typedef double v8df __attribute__((vector_size(64)));       /* 512-bit double */

/* Accumulator arrays */
static v8si int_acc_256[4];
static v16si int_acc_512[4];
static v8sf float_acc_256[4];
static v16sf float_acc_512[4];

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int seed) {
    srand(seed);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_int_array[i] = rand() % 1000;
        global_float_array[i] = (rand() % 1000) / 10.0f;
    }
    for (int i = 0; i < 32; i++) {
        volatile_mask[i] = rand() % 16; /* Mask indices 0-15 */
    }
}

/* Function 1: 10-operand shuffle with 512-bit integer vectors */
void shuffle_10_operand_int512(int iter) {
    /* Load data - using volatile to prevent constant propagation */
    volatile int offset = iter * 16;
    v16si a = *(v16si*)&global_int_array[offset];
    v16si b = *(v16si*)&global_int_array[offset + 16];
    
    /* Create control mask from volatile array */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = volatile_mask[(iter + i) % 32] % 32;
    }
    v16si mask = *(v16si*)mask_arr;
    
    /* Complex control flow to prevent optimization */
    if (iter % 3 == 0) {
        /* 10-operand __builtin_shufflevector: 
           Input: a (16 elements), b (16 elements), mask (16 elements)
           Total elements referenced: 48, but the builtin takes 3 vector arguments
           plus mask indices - we need to use shufflevector with explicit indices */
        
        /* This creates a pattern that requires many operands during expansion */
        v16si result = __builtin_shufflevector(a, b, 
            mask_arr[0], mask_arr[1], mask_arr[2], mask_arr[3],
            mask_arr[4], mask_arr[5], mask_arr[6], mask_arr[7],
            mask_arr[8], mask_arr[9], mask_arr[10], mask_arr[11],
            mask_arr[12], mask_arr[13], mask_arr[14], mask_arr[15]);
        
        /* Perform arithmetic to ensure result is used */
        result = result + a;
        int_acc_512[iter % 4] = int_acc_512[iter % 4] + result;
        
    } else if (iter % 3 == 1) {
        /* Alternative shuffle pattern */
        v16si temp = __builtin_shuffle(a, b, mask);
        temp = temp * 2;
        int_acc_512[(iter + 1) % 4] = int_acc_512[(iter + 1) % 4] + temp;
    } else {
        /* Nested shuffle operations */
        v16si shuffled1 = __builtin_shuffle(a, mask);
        v16si shuffled2 = __builtin_shuffle(b, mask);
        v16si combined = shuffled1 + shuffled2;
        int_acc_512[(iter + 2) % 4] = int_acc_512[(iter + 2) % 4] + combined;
    }
}

/* Function 2: 11-operand pattern with mixed float/double vectors */
void shuffle_11_operand_mixed(int iter) {
#ifdef __AVX512F__
    volatile int offset = iter * 8;
    
    /* Load different vector types */
    v8df a_double = *(v8df*)&global_float_array[offset * 2];
    v16sf a_float = *(v16sf*)&global_float_array[offset * 4];
    
    /* Create complex mask */
    int32_t mask_arr[16];
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = (volatile_mask[(iter * 2 + i) % 32] + i) % 16;
    }
    
    /* Switch statement to create complex control flow */
    switch (iter % 4) {
        case 0: {
            /* Multi-step shuffle that may expand to 11 operands */
            v16sf shuffled = __builtin_shuffle(a_float, *(v16si*)mask_arr);
            
            /* Convert and shuffle again */
            v8df converted = __builtin_convertvector(shuffled, v8df);
            v8df mask_double = __builtin_convertvector(*(v8si*)mask_arr, v8df);
            
            /* Complex operation chain */
            v8df result = converted * mask_double;
            float_acc_512[0] = __builtin_convertvector(result, v16sf);
            break;
        }
        case 1: {
            /* Different shuffle pattern */
            v8df shuffled = __builtin_shuffle(a_double, 
                __builtin_convertvector(*(v8si*)&mask_arr[0], v8df));
            shuffled = shuffled + 1.0;
            break;
        }
        case 2: {
            /* Nested shuffles */
            v16sf temp1 = __builtin_shuffle(a_float, *(v16si*)&mask_arr[0]);
            v16sf temp2 = __builtin_shuffle(a_float, *(v16si*)&mask_arr[8]);
            v16sf combined = temp1 + temp2;
            float_acc_512[1] = float_acc_512[1] + combined;
            break;
        }
        default: {
            /* Default case with arithmetic */
            v8df result = a_double * 2.0;
            break;
        }
    }
#endif
}

/* Function 3: Narrowing and expanding shuffles */
void shuffle_narrow_expand(int iter) {
#ifdef __AVX2__
    volatile int offset = iter * 8;
    
    /* Load 256-bit vectors */
    v8si a = *(v8si*)&global_int_array[offset];
    v8si b = *(v8si*)&global_int_array[offset + 8];
    
    /* Create mask from volatile */
    int32_t mask_arr[16]; /* Larger than needed for complexity */
    for (int i = 0; i < 16; i++) {
        mask_arr[i] = volatile_mask[(iter * 3 + i) % 32] % 16;
    }
    
    /* Loop with conditional shuffles */
    for (int j = 0; j < 2; j++) {
        if (j == 0) {
            /* Narrowing shuffle: select 4 elements from each 8-element vector */
            v8si narrowed = __builtin_shufflevector(a, b,
                mask_arr[0] % 8, mask_arr[1] % 8, mask_arr[2] % 8, mask_arr[3] % 8,
                (mask_arr[4] % 8) + 8, (mask_arr[5] % 8) + 8, 
                (mask_arr[6] % 8) + 8, (mask_arr[7] % 8) + 8);
            
            /* Expand back with different pattern */
            v8si expanded = __builtin_shuffle(narrowed, 
                *(v8si*)&mask_arr[8]);
            
            int_acc_256[iter % 4] = int_acc_256[iter % 4] + expanded;
        } else {
            /* Different shuffle pattern */
            v8si shuffled = __builtin_shuffle(a, *(v8si*)&mask_arr[4]);
            shuffled = shuffled * 3;
            int_acc_256[(iter + 1) % 4] = int_acc_256[(iter + 1) % 4] + shuffled;
        }
    }
#endif
}

/* Function 4: SSE2-compatible shuffles */
void shuffle_sse2_pattern(int iter) {
#ifdef __SSE2__
    typedef int32_t v4si __attribute__((vector_size(16)));
    
    volatile int offset = iter * 4;
    v4si a = *(v4si*)&global_int_array[offset];
    v4si b = *(v4si*)&global_int_array[offset + 4];
    
    int32_t mask_arr[8];
    for (int i = 0; i < 8; i++) {
        mask_arr[i] = volatile_mask[(iter + i * 2) % 32] % 8;
    }
    
    /* Multiple shuffle operations in sequence */
    v4si result;
    if (iter % 2 == 0) {
        result = __builtin_shufflevector(a, b,
            mask_arr[0], mask_arr[1], mask_arr[2], mask_arr[3],
            mask_arr[4] + 4, mask_arr[5] + 4, mask_arr[6] + 4, mask_arr[7] + 4);
    } else {
        result = __builtin_shuffle(a, *(v4si*)mask_arr);
    }
    
    /* Store through volatile pointer to prevent elimination */
    volatile v4si* volatile_ptr = (volatile v4si*)&global_int_array[offset];
    *volatile_ptr = result;
#endif
}

/* Compute checksum from accumulator arrays */
int64_t compute_checksum() {
    int64_t checksum = 0;
    
    /* Sum integer accumulators */
    for (int i = 0; i < 4; i++) {
        int32_t* ptr = (int32_t*)&int_acc_256[i];
        for (int j = 0; j < 8; j++) {
            checksum += ptr[j];
        }
        
        ptr = (int32_t*)&int_acc_512[i];
        for (int j = 0; j < 16; j++) {
            checksum += ptr[j];
        }
    }
    
    /* Sum float accumulators (convert to int for checksum) */
    for (int i = 0; i < 4; i++) {
        float* fptr = (float*)&float_acc_256[i];
        for (int j = 0; j < 8; j++) {
            checksum += (int64_t)fptr[j];
        }
        
        fptr = (float*)&float_acc_512[i];
        for (int j = 0; j < 16; j++) {
            checksum += (int64_t)fptr[j];
        }
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    /* Initialize with seed from command line or default */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    init_arrays(seed);
    
    /* Clear accumulators */
    memset(int_acc_256, 0, sizeof(int_acc_256));
    memset(int_acc_512, 0, sizeof(int_acc_512));
    memset(float_acc_256, 0, sizeof(float_acc_256));
    memset(float_acc_512, 0, sizeof(float_acc_512));
    
    /* Main loop with different shuffle patterns */
    for (int i = 0; i < 10; i++) {
        /* Call different shuffle functions based on iteration */
        shuffle_10_operand_int512(i);
        
#ifdef __AVX512F__
        shuffle_11_operand_mixed(i);
#endif
        
#ifdef __AVX2__
        shuffle_narrow_expand(i);
#endif
        
        shuffle_sse2_pattern(i);
        
        /* Additional volatile operation to prevent optimization */
        volatile int dummy = volatile_mask[i % 32];
        (void)dummy;
    }
    
    /* Compute and print checksum */
    int64_t checksum = compute_checksum();
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
