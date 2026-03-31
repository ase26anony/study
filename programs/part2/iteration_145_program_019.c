#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification that could alias */
void process_with_alias(int *p, int *q) {
    /* Test expression uses *p, modification might affect it through aliasing */
    if (*p > 100) {
        *q = 50;  /* q might alias p */
        *p = *p - 10;  /* Direct modification of test expression */
    }
}

/* Function with volatile variable to prevent optimization */
void process_volatile(volatile int *vp) {
    if (*vp > 0) {
        *vp = *vp * 2;  /* Modification of volatile test expression */
        *vp = *vp + 1;  /* Second modification - ensures multiple instructions */
    }
}

/* Function with mixed data types and implicit conversions */
float process_mixed_types(int x, float y) {
    float result = y;
    /* Test expression involves float comparison, modification in int */
    if ((float)x > y) {
        x = x / 2;  /* First modification */
        x = x + 1;  /* Second modification */
        result = (float)x;  /* Third operation */
    }
    return result;
}

/* Function with array access and potential self-modification */
void process_array(int arr[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        /* Condition uses arr[i], then block modifies it */
        if (arr[i] > threshold) {
            arr[i] = threshold;  /* First modification */
            arr[i] = arr[i] - 1;  /* Second modification - ensures multiple insns */
        }
    }
}

/* Function with multiple modifications in then block */
int process_multiple_mods(int a, int b) {
    int temp = a;
    if (temp > b) {
        temp = temp - b;  /* First modification of test variable */
        temp = temp * 2;  /* Second modification */
        temp = temp + 1;  /* Third modification - 3 non-label instructions */
    }
    return temp;
}

/* Function with pointer arithmetic and aliasing */
void process_pointer_arithmetic(int *base, int offset1, int offset2) {
    int *ptr1 = base + offset1;
    int *ptr2 = base + offset2;
    
    /* offset1 and offset2 might be equal, causing aliasing */
    if (*ptr1 > 0) {
        *ptr2 = 0;  /* Might modify same location as ptr1 */
        *ptr1 = *ptr1 * 2;  /* Direct modification */
    }
}

int main() {
    volatile int v1 = 10;
    volatile int v2 = 20;
    int arr[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int arr2[5] = {100, 200, 300, 400, 500};
    int x = 30, y = 40, z = 50;
    int result = 0;
    
    /* Test 1: Volatile variable modification */
    process_volatile(&v1);
    
    /* Test 2: Array processing with loop-dependent condition */
    process_array(arr, 10, 50);
    
    /* Test 3: Mixed types with implicit conversions */
    float fresult = process_mixed_types(x, 25.5f);
    
    /* Test 4: Multiple modifications in then block */
    result += process_multiple_mods(x, y);
    
    /* Test 5: Pointer aliasing scenario */
    process_with_alias(&arr[2], &arr[2]);  /* Same location - definite aliasing */
    
    /* Test 6: Another aliasing scenario with different offsets */
    process_with_alias(&arr[3], &arr[4]);  /* Different locations - possible aliasing */
    
    /* Test 7: Pointer arithmetic with potential aliasing */
    process_pointer_arithmetic(arr2, 1, 1);  /* Same offset - aliasing */
    
    /* Test 8: Direct modification in short then block */
    if (z > 20) {
        z = z - 10;  /* First modification */
        z = z * 2;   /* Second modification */
    }
    
    /* Test 9: Nested conditions with modifications */
    for (int i = 0; i < 5; i++) {
        if (arr2[i] > 250) {
            arr2[i] = arr2[i] / 2;  /* First modification */
            arr2[i] = arr2[i] + i;  /* Second modification */
        }
    }
    
    /* Test 10: Complex test expression with modification */
    int *ptr = &x;
    if ((*ptr + y) > 50) {
        *ptr = 0;      /* Modifies x which is part of test expression */
        y = y - 10;    /* Also modifies y which is part of test expression */
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: v1=%d, v2=%d, fresult=%.2f, result=%d, z=%d\n", 
           v1, v2, fresult, result, z);
    
    /* Print array contents to ensure modifications happened */
    printf("Array1: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\nArray2: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr2[i]);
    }
    printf("\n");
    
    return 0;
}
