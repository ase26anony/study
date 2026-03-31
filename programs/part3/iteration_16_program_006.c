#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static int complex_access_loop(volatile int n, volatile int stride, 
                               volatile int offset, volatile int scale) {
    /* Force these to be in memory to prevent constant propagation */
    volatile int v1 = stride;
    volatile int v2 = offset;
    volatile int v3 = scale;
    
    /* Many live variables to consume registers */
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4, var5 = 5;
    int var6 = 6, var7 = 7, var8 = 8, var9 = 9, var10 = 10;
    int var11 = 11, var12 = 12, var13 = 13, var14 = 14, var15 = 15;
    int var16 = 16, var17 = 17, var18 = 18, var19 = 19, var20 = 20;
    int var21 = 21, var22 = 22, var23 = 23, var24 = 24, var25 = 25;
    
    /* Struct with multiple fields to force complex addressing */
    struct Data {
        int x[4];
        int y;
        int z[2];
    };
    
    /* Large array to force memory accesses */
    struct Data arr[1024];
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = i + j;
        }
        arr[i].y = i * 2;
        arr[i].z[0] = i * 3;
        arr[i].z[1] = i * 4;
    }
    
    int sum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n; ++i) {
        /* Use many variables in address calculation to keep them live */
        int temp1 = var1 + var2 + var3 + var4 + var5;
        int temp2 = var6 + var7 + var8 + var9 + var10;
        int temp3 = var11 + var12 + var13 + var14 + var15;
        int temp4 = var16 + var17 + var18 + var19 + var20;
        int temp5 = var21 + var22 + var23 + var24 + var25;
        
        /* Complex index calculation using volatile variables 
           to prevent optimization */
        int idx = (i * v1 + v2) / v3;
        idx = idx + temp1 - temp2 + temp3 - temp4 + temp5;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = 0;
        if (idx >= 1024) idx = 1023;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(idx * 2) % 1024].z[0];
        val += arr[(i * 3) % 1024].z[1];
        
        /* Use variables in computation */
        val += temp1 - temp2 + temp3;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        int store_idx = (i * v2 + v1) / v3;
        store_idx = (store_idx + temp4 - temp5) % 1024;
        
        /* Store with complex addressing */
        arr[store_idx].x[0] = val * v1 + v2;
        arr[store_idx].y = val * v3 + v1;
        
        /* More complex addressing for another store */
        int store_idx2 = (i * v3 + v1 * v2) / (v3 + 1);
        store_idx2 = (store_idx2 + temp1 + temp5) % 1024;
        
        /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        arr[store_idx2].z[i % 2] = val + temp2 + temp4;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[store_idx].x[0]));
        asm volatile("# Complex address 3: %0" : : "m" (arr[store_idx2].z[0]));
        
        /* Update variables to keep them live and prevent optimization */
        var1 += i; var2 += idx; var3 += store_idx;
        var4 += val; var5 += store_idx2;
        var6 -= i; var7 -= idx; var8 -= store_idx;
        var9 -= val; var10 -= store_idx2;
        var11 ^= i; var12 ^= idx; var13 ^= store_idx;
        var14 ^= val; var15 ^= store_idx2;
        var16 |= i; var17 |= idx; var18 |= store_idx;
        var19 |= val; var20 |= store_idx2;
        var21 &= ~i; var22 &= ~idx; var23 &= ~store_idx;
        var24 &= ~val; var25 &= ~store_idx2;
        
        sum += val;
    }
    
    /* Use all variables in final computation */
    sum += var1 + var2 + var3 + var4 + var5;
    sum += var6 + var7 + var8 + var9 + var10;
    sum += var11 + var12 + var13 + var14 + var15;
    sum += var16 + var17 + var18 + var19 + var20;
    sum += var21 + var22 + var23 + var24 + var25;
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
static int complex_address_operand(volatile int* base, volatile int index1, 
                                   volatile int index2, volatile int scale) {
    /* Complex addressing mode as function argument */
    int val = base[(index1 * index2) / scale];
    
    /* Inline assembly with memory operand */
    asm volatile("# Operand address: %0" : : "m" (base[(index1 + index2) * scale]));
    
    return val * 2;
}

int main(void) {
    /* Volatile to prevent constant propagation */
    volatile int n = 500;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Call the complex access function */
    int result = complex_access_loop(n, stride, offset, scale);
    
    /* Create another scenario for different reload types */
    int large_array[4096];
    for (int i = 0; i < 4096; i++) {
        large_array[i] = i * i;
    }
    
    /* Force address reloads through function call */
    int val1 = complex_address_operand(large_array, stride, offset, scale);
    int val2 = complex_address_operand(large_array + 1024, offset, stride, scale + 1);
    
    /* Mix results to prevent dead code elimination */
    result += val1 + val2;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
