/* test-caller-save.c
 * A test program designed to trigger specific uncovered lines in GCC's
 * caller-save.cc optimization pass, particularly the instruction insertion
 * logic that updates BB_END when inserting save/restore instructions.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointers to prevent optimization */
typedef void (*volatile func_ptr)(void);

/* Global sink to prevent dead code elimination */
volatile int global_sink;

/* Simple non-inline functions that get called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Function call at the end of a basic block before return
 * This creates a scenario where BB_END needs to be updated
 */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d) {
    /* Create many live values that must survive across the call */
    int live1 = a * 2 + 1;
    int live2 = b ^ 0x5555;
    int live3 = c & 0xAAAA;
    int live4 = d | 0x1234;
    int live5 = a + b + c + d;
    int live6 = (a << 3) | (b >> 2);
    int live7 = c * d - a;
    int live8 = (d ^ a) & (b | c);
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = func1;
    
    /* Complex condition to create basic block structure */
    if (a > b) {
        /* More computations to increase register pressure */
        live1 = live1 * 3;
        live2 = live2 + live3;
        live4 = live4 ^ live5;
        live6 = live6 & live7;
        live8 = live8 | live1;
        
        /* Call at the end of basic block before return */
        fp();
        
        /* This return makes the call the last instruction in the BB */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path with different computations */
        live1 = live1 / 2;
        live2 = live2 - live3;
        live4 = live4 | live5;
        live6 = live6 ^ live7;
        live8 = live8 & live1;
        
        /* Another call at BB end */
        void (*volatile fp2)(void) = func2;
        fp2();
        
        return live1 * live2 * live3 * live4;
    }
}

/* Test 2: Function call in switch case with break
 * Creates basic blocks that end with calls
 */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * y;
    int val2 = y + z;
    int val3 = x ^ z;
    int val4 = (x << 4) | (y >> 2);
    int val5 = z * 3;
    int val6 = x & y & z;
    int val7 = (x + y) * (z - x);
    int val8 = y | z;
    
    switch (x % 4) {
        case 0:
            /* Many operations before call */
            val1 = val1 + val2;
            val3 = val3 * val4;
            val5 = val5 ^ val6;
            val7 = val7 & val8;
            
            /* Call at end of case before break */
            func1();
            break;
            
        case 1:
            val2 = val2 - val3;
            val4 = val4 | val5;
            val6 = val6 + val7;
            val8 = val8 * val1;
            
            /* Different function call */
            void (*volatile fp)(void) = func2;
            fp();
            break;
            
        case 2:
            /* Even more register pressure */
            int extra1 = val1 * val2;
            int extra2 = val3 ^ val4;
            int extra3 = val5 & val6;
            int extra4 = val7 | val8;
            
            extra1 = extra1 + extra2;
            extra3 = extra3 - extra4;
            
            func3();
            break;
            
        default:
            /* Complex computation chain */
            for (int i = 0; i < 3; i++) {
                val1 = val1 + i;
                val2 = val2 ^ i;
                val3 = val3 * (i + 1);
            }
            
            func4();
            break;
    }
    
    /* Use all values after switch to ensure liveness across calls */
    result = val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
    
    /* Volatile store to prevent optimization */
    global_sink = result;
    return result;
}

/* Test 3: Complex loop before call, then use values after call
 * Creates high register pressure
 */
NOINLINE int test_call_between_complex_ops(int seed) {
    /* Array of values that will be computed in loop */
    int values[12];
    
    /* Initialize with seed */
    for (int i = 0; i < 12; i++) {
        values[i] = seed + i;
    }
    
    /* Complex loop creating many live values */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    
    for (int i = 0; i < 100; i++) {
        /* Many independent computations to create register pressure */
        tmp1 = values[i % 12] * i;
        tmp2 = values[(i + 1) % 12] ^ i;
        tmp3 = values[(i + 2) % 12] & 0xFF;
        tmp4 = values[(i + 3) % 12] | i;
        tmp5 = values[(i + 4) % 12] << (i % 4);
        tmp6 = values[(i + 5) % 12] >> (i % 3);
        
        acc1 = acc1 + tmp1;
        acc2 = acc2 ^ tmp2;
        acc3 = acc3 | tmp3;
        acc4 = acc4 & tmp4;
        
        /* More intermediate values */
        values[i % 12] = (tmp5 + tmp6) * acc1;
    }
    
    /* Call with all accumulators live */
    void (*volatile fp)(void) = func1;
    fp();
    
    /* Use all computed values after call */
    int result = acc1 + acc2 + acc3 + acc4;
    for (int i = 0; i < 12; i++) {
        result = result ^ values[i];
    }
    
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int x = a, y = b, z = c;
    
    /* Layer 1 */
    if (a > 0) {
        x = x * 2;
        y = y + 3;
        z = z ^ 0x99;
        
        /* Layer 2 */
        if (b > 0) {
            int t1 = x * y;
            int t2 = y * z;
            int t3 = z * x;
            int t4 = t1 + t2 + t3;
            
            /* Layer 3 */
            if (c > 0) {
                int u1 = t4 & 0xF0F0;
                int u2 = t4 | 0x0F0F;
                int u3 = u1 ^ u2;
                int u4 = u3 << 2;
                
                /* Call at end of deeply nested block */
                func2();
                
                return u4;
            } else {
                int u1 = t4 >> 4;
                int u2 = t4 << 4;
                int u3 = u1 | u2;
                
                func3();
                
                return u3;
            }
        } else {
            int t1 = x + y + z;
            int t2 = x ^ y ^ z;
            int t3 = t1 * t2;
            
            func4();
            
            return t3;
        }
    } else {
        x = x / 2;
        y = y - 3;
        z = z | 0x66;
        
        void (*volatile fp)(void) = func1;
        fp();
        
        return x + y + z;
    }
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running caller-save insertion tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i*2, i*3, i*4);
    }
    
    /* Run test 2 */
    for (int i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, i+1, i+2);
    }
    
    /* Run test 3 */
    for (int i = 1; i <= 5; i++) {
        total += test_call_between_complex_ops(i * 100);
    }
    
    /* Run test 4 */
    for (int i = -5; i <= 5; i++) {
        total += test_nested_conditionals(i, i+1, i+2);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
