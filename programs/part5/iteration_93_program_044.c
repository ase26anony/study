/* auto-inc-dec-test.c
 * 
 * This program is designed to trigger auto-increment/decrement addressing
 * pattern recognition in GCC's RTL optimizer (find_auto_inc).
 * It focuses on post-increment/decrement memory operations that should
 * generate the specific RTL pattern matching the uncovered lines in
 * auto-inc-dec.cc (lines 1352-1358).
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Structure for testing pointer post-increment with field access */
struct Data {
    int value;
    int count;
    char id;
};

/* Function 1: Copy with post-increment pointers (classic strcpy-like) */
void copy_with_postinc(char *dest, const char *src, int n) {
    char *d = dest;
    const char *s = src;
    
    /* Tight loop with post-increment in condition */
    while (n-- > 0 && (*d++ = *s++) != '\0') {
        /* Empty body - all work in condition */
    }
}

/* Function 2: Summation with mixed volatile/non-volatile pointers */
int sum_with_postinc(volatile int *varr, int *arr, int n) {
    volatile int *vp = varr;
    int *p = arr;
    int sum = 0;
    
    /* Loop with post-increment in update statement */
    for (int i = 0; i < n; i++) {
        /* Access volatile, then non-volatile, both with post-increment */
        sum += *vp++;
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_value(volatile int *arr, int size, int target) {
    volatile int *p = arr;
    int found = -1;
    
    /* Multiple basic blocks with post-increment */
    for (int i = 0; i < size; i++) {
        /* Post-increment in conditional expression */
        if (*p++ == target) {
            found = i;
            /* Continue searching in taken path */
            while (i < size && *p++ != target + 1) {
                i++;
            }
            break;
        } else if (i % 2 == 0) {
            /* Another post-increment in not-taken path */
            volatile int temp = *p++;
            (void)temp; /* Use temp to avoid dead store */
        }
    }
    
    return found;
}

/* Function 4: Structure array processing with post-increment */
int process_structs(struct Data *sptr, int count) {
    int total = 0;
    struct Data *current = sptr;
    
    /* Nested loops with post-increment */
    for (int i = 0; i < count; i++) {
        /* Structure field access with pointer post-increment */
        total += current->value;
        current++;
        
        /* Inner loop with different post-increment pattern */
        for (int j = 0; j < current->count; j++) {
            /* Comma expression: access then increment */
            int val = (current->value, current++, val);
            total += val;
        }
    }
    
    return total;
}

/* Function 5: Switch statement with fall-through and post-increment */
int switch_with_postinc(volatile char *buf, int *results) {
    volatile char *ptr = buf;
    int idx = 0;
    int sum = 0;
    
    while (*ptr != '\0') {
        switch (*ptr++) {  /* Post-increment in switch expression */
            case 'A':
            case 'a':
                /* Fall through with post-increment */
                results[idx++] = 1;
                sum += *ptr++;  /* Post-increment in case body */
                break;
                
            case 'B':
            case 'b':
                results[idx++] = 2;
                /* Comma expression sequencing */
                sum += (int)(*ptr, ptr++, *ptr);
                break;
                
            default:
                results[idx++] = 0;
                /* Array access with zero offset equivalent */
                sum += ptr[0];  /* reg1_val = 0 pattern */
                ptr++;
                break;
        }
    }
    
    return sum;
}

/* Function 6: Pointer arithmetic with zero offset pattern */
void zero_offset_patterns(int *arr, volatile int *varr, int n) {
    int *p = arr;
    volatile int *vp = varr;
    
    /* Direct dereference (offset 0) followed by increment */
    for (int i = 0; i < n; i++) {
        *p = *vp;  /* Simple dereference - should match mem_insn.reg1_val = 0 */
        p++;
        vp++;
    }
    
    /* Another zero-offset pattern */
    p = arr;
    int i = 0;
    while (i < n) {
        /* Array access with variable index that's always 0 in this context */
        int idx = 0;
        p[idx] = i;  /* Base address + 0 offset */
        p++;
        i++;
    }
}

int main(void) {
    /* Test data setup */
    char src[100] = "Test string for copy operations with post-increment patterns.";
    char dest[100];
    
    volatile int varray[20];
    int array[20];
    
    struct Data structs[10];
    
    volatile char charbuf[] = "AaBbcCDdefFgG123";
    int results[50];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        varray[i] = i * 2;
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        structs[i].value = i * 10;
        structs[i].count = i % 3 + 1;
        structs[i].id = 'A' + i;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(dest, src, sizeof(dest));
    printf("Copy test: src='%s', dest='%s'\n", src, dest);
    
    /* Test 2: Summation with mixed pointers */
    int sum1 = sum_with_postinc(varray, array, 20);
    printf("Sum test: sum = %d\n", sum1);
    
    /* Test 3: Search with post-increment */
    int found = find_value(varray, 20, 24);  /* Should find 24 at index 12 */
    printf("Search test: found value 24 at index %d\n", found);
    
    /* Test 4: Structure processing */
    int struct_sum = process_structs(structs, 10);
    printf("Structure test: total = %d\n", struct_sum);
    
    /* Test 5: Switch with post-increment */
    int switch_sum = switch_with_postinc(charbuf, results);
    printf("Switch test: sum = %d\n", switch_sum);
    
    /* Test 6: Zero offset patterns */
    zero_offset_patterns(array, varray, 20);
    printf("Zero-offset test: array[10] = %d\n", array[10]);
    
    /* Verify copy worked */
    if (strcmp(src, dest) != 0) {
        printf("ERROR: Copy failed!\n");
        return 1;
    }
    
    /* Verify computations */
    int expected_sum = 0;
    for (int i = 0; i < 20; i++) {
        expected_sum += varray[i] + array[i];
    }
    
    if (sum1 != expected_sum) {
        printf("ERROR: Sum mismatch! Expected %d, got %d\n", expected_sum, sum1);
        return 1;
    }
    
    printf("All tests completed successfully.\n");
    return 0;
}
