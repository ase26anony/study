#include <stdio.h>
#include <stdlib.h>

// Function with pointer-based modification that could alias
void process_with_alias(int *p, int *q) {
    // Test expression uses *p, modification might affect it through aliasing
    if (*p > 0) {
        *q = 0;  // q might alias p
        *p = -1; // Direct modification of test expression
    }
}

// Function with volatile test variable
void process_volatile(volatile int *vp) {
    // Volatile prevents optimization, ensures modification stays
    if (*vp > 10) {
        *vp = 5;
        *vp = *vp + 1;  // Multiple modifications
    }
}

// Function with mixed data types and implicit conversions
void process_mixed_types(int x, float y) {
    // Complex test expression with conversion
    if ((float)x > y) {
        x = (int)(y * 2.0f);  // Modifies variable used in test
        x = x + 1;            // Second modification
    }
}

// Function with array access and potential self-modification
void process_array(int arr[], int n, int idx1, int idx2) {
    // idx1 and idx2 might be equal - creates aliasing possibility
    if (arr[idx1] > 0) {
        arr[idx2] = 0;  // Could modify the same element if idx1 == idx2
        arr[idx1] = -1; // Definitely modifies test expression
    }
}

// Function designed specifically for if-conversion candidate
// Short then-block with multiple non-label instructions
int ifcvt_candidate(int a, int b) {
    int result = a;
    
    // This is the key pattern: test expression 'result' is modified in then-block
    if (result > 0) {
        result = b * 2;     // First modification
        result = result + 1; // Second modification - multiple non-label instructions
        // No function calls, no complex operations
        // Short enough for if-conversion consideration
    }
    
    return result;
}

// Loop-dependent condition with side effects
void process_loop(int data[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        // Condition depends on loop variable
        if (data[i] > threshold) {
            data[i] = 0;      // Modifies test expression
            data[i] += i;     // Second modification
        }
    }
}

// Complex scenario with multiple potential modifications
int complex_scenario(int *ptr1, int *ptr2, volatile int *vptr) {
    int local = *ptr1;
    int sum = 0;
    
    // First if: volatile test
    if (*vptr > 0) {
        *vptr = local;
        sum += *vptr;
    }
    
    // Second if: pointer aliasing concern
    if (*ptr1 > *ptr2) {
        *ptr2 = *ptr1;  // ptr2 might alias ptr1
        local = *ptr2;  // Modifies local used elsewhere
    }
    
    // Third if: candidate for if-conversion
    if (local > 100) {
        local = local / 2;
        local = local * 3;
        sum += local;
    }
    
    return sum;
}

int main() {
    // Initialize test data
    volatile int volatile_var = 15;
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[5] = {20, -5, 30, -10, 40};
    int x = 10, y = 20, z = 30;
    int *ptr_x = &x;
    int *ptr_y = &y;
    
    printf("Starting tests...\n");
    
    // Test 1: Basic if-conversion candidate
    int res1 = ifcvt_candidate(5, 3);
    printf("ifcvt_candidate(5, 3) = %d\n", res1);
    
    // Test 2: Process with potential aliasing
    process_with_alias(&x, &x);  // Same pointer - definite aliasing
    printf("After process_with_alias: x = %d\n", x);
    
    // Test 3: Volatile processing
    process_volatile(&volatile_var);
    printf("After process_volatile: volatile_var = %d\n", volatile_var);
    
    // Test 4: Mixed types
    process_mixed_types(15, 10.5f);
    
    // Test 5: Array processing with potential self-modification
    process_array(array1, 10, 2, 2);  // Same index - will modify test expression
    printf("array1[2] = %d\n", array1[2]);
    
    // Test 6: Loop processing
    process_loop(array2, 5, 10);
    printf("array2 after process_loop: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", array2[i]);
    }
    printf("\n");
    
    // Test 7: Complex scenario
    int sum = complex_scenario(&x, &y, &volatile_var);
    printf("complex_scenario result = %d\n", sum);
    
    // Additional tests to increase coverage
    // Multiple short if-blocks in sequence
    int a = 5, b = 10, c = 15;
    
    if (a > 0) {
        a = b + c;
        a = a * 2;
    }
    
    if (b < 20) {
        b = a - c;
        c = b * 3;
    }
    
    // Nested if with modification
    if (c > 10) {
        if (a > b) {
            a = c;
            b = a + 1;
        }
        c = 0;
    }
    
    printf("Final values: a=%d, b=%d, c=%d\n", a, b, c);
    
    return 0;
}
