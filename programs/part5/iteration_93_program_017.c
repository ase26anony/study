#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SIZE 256
#define PATTERN_SIZE 16

/* Structure for testing pointer post-increment */
struct Data {
    int values[4];
    char tag;
    int count;
};

/* Function 1: Copy with post-increment - tight loop */
void copy_with_postinc(volatile char *dest, const char *src, size_t n) {
    volatile char *d = dest;
    const char *s = src;
    
    /* Classic K&R style copy with post-increment */
    while (n-- > 0) {
        *d++ = *s++;
    }
}

/* Function 2: Summation with mixed volatile/non-volatile */
int sum_with_postinc(volatile int *arr, int n) {
    int sum = 0;
    volatile int *p = arr;
    
    /* Loop with post-increment in condition */
    while (n-- > 0) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 3: Search with post-increment in complex control flow */
int find_pattern(volatile char *buffer, const char *pattern, int len) {
    volatile char *p = buffer;
    const char *pat = pattern;
    int found = -1;
    int i = 0;
    
    /* Nested loops with post-increment */
    while (i < len) {
        const char *pat_temp = pattern;
        volatile char *p_temp = p;
        int match = 1;
        
        /* Inner loop with post-increment */
        for (int j = 0; j < PATTERN_SIZE; j++) {
            if (*p_temp++ != *pat_temp++) {
                match = 0;
                break;
            }
        }
        
        if (match) {
            found = i;
            /* Post-increment in if branch */
            p += PATTERN_SIZE;
            i += PATTERN_SIZE;
        } else {
            /* Post-increment in else branch */
            p++;
            i++;
        }
    }
    
    return found;
}

/* Function 4: Structure access with pointer post-increment */
void process_structs(struct Data *structs, int count) {
    struct Data *sptr = structs;
    
    /* Loop through structs with post-increment */
    for (int i = 0; i < count; i++) {
        /* Access struct field then increment */
        int val = sptr->count;
        sptr++;
        
        /* Another access with different pattern */
        if (val > 0) {
            /* Comma expression with post-increment */
            int temp = (sptr->values[0], sptr++, temp);
        }
    }
}

/* Function 5: Mixed qualifiers in same expression */
int mixed_qualifier_test(volatile int *vptr, int *regptr, int n) {
    int result = 0;
    
    /* Using both volatile and non-volatile pointers */
    for (int i = 0; i < n; i++) {
        /* Access volatile, then regular, both with post-inc */
        result += *vptr++ + *regptr++;
    }
    
    return result;
}

/* Function 6: Switch case with post-increment */
int switch_with_postinc(volatile int *arr, int op) {
    int result = 0;
    volatile int *ptr = arr;
    
    switch (op) {
        case 0:
            /* Fall-through with post-increment */
            result = *ptr++;
            /* FALLTHROUGH */
        case 1:
            result += *ptr++;
            break;
        case 2:
            /* Multiple post-increments */
            result = *ptr++ + *ptr++;
            break;
        case 3:
            /* Post-increment in loop inside switch */
            for (int i = 0; i < 4; i++) {
                result += *ptr++;
            }
            break;
        default:
            result = *ptr++;
    }
    
    return result;
}

/* Function 7: Zero offset array access */
void zero_offset_test(volatile int *buffer) {
    volatile int *ptr = buffer;
    
    /* Direct dereference with zero offset */
    int first = ptr[0];  /* Should use zero offset */
    ptr++;
    
    /* Another zero offset access */
    int second = *ptr;  /* Direct dereference */
    ptr++;
    
    /* Array access with index 0 */
    int third = ptr[0];  /* Zero offset again */
}

/* Function 8: String operations with post-increment */
int string_length(const char *str) {
    const char *p = str;
    
    /* Classic strlen implementation */
    while (*p++ != '\0')
        ;
    
    return p - str - 1;
}

/* Function 9: Reverse copy with post-decrement */
void reverse_copy(char *dest, const char *src, int len) {
    char *d = dest + len - 1;
    const char *s = src;
    
    /* Post-decrement loop */
    while (len-- > 0) {
        *d-- = *s++;
    }
}

/* Main function with various test cases */
int main() {
    /* Test data - mix volatile and non-volatile */
    volatile char buffer1[SIZE];
    char buffer2[SIZE];
    volatile int numbers[SIZE];
    int regular_numbers[SIZE];
    struct Data struct_array[10];
    
    /* Initialize test data */
    for (int i = 0; i < SIZE; i++) {
        buffer1[i] = (char)(i % 26 + 'A');
        buffer2[i] = (char)(i % 26 + 'a');
        numbers[i] = i * 2;
        regular_numbers[i] = i * 3;
    }
    
    /* Initialize structs */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            struct_array[i].values[j] = i * 10 + j;
        }
        struct_array[i].tag = 'A' + i;
        struct_array[i].count = i * 5;
    }
    
    /* Test 1: Copy with post-increment */
    copy_with_postinc(buffer1, "Test pattern for auto-inc", 25);
    
    /* Test 2: Summation */
    int sum1 = sum_with_postinc(numbers, SIZE);
    printf("Sum of numbers: %d\n", sum1);
    
    /* Test 3: Pattern search */
    const char pattern[PATTERN_SIZE] = "TEST_PATTERN";
    int found = find_pattern(buffer1, pattern, SIZE);
    printf("Pattern found at: %d\n", found);
    
    /* Test 4: Structure processing */
    process_structs(struct_array, 10);
    
    /* Test 5: Mixed qualifiers */
    int mixed_sum = mixed_qualifier_test(numbers, regular_numbers, SIZE / 2);
    printf("Mixed qualifier sum: %d\n", mixed_sum);
    
    /* Test 6: Switch with post-increment */
    int switch_result = switch_with_postinc(numbers, 2);
    printf("Switch result: %d\n", switch_result);
    
    /* Test 7: Zero offset test */
    zero_offset_test(numbers);
    
    /* Test 8: String length */
    const char *test_str = "Hello, auto-increment world!";
    int len = string_length(test_str);
    printf("String length: %d\n", len);
    
    /* Test 9: Reverse copy */
    char source[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char dest[27];
    reverse_copy(dest, source, 26);
    dest[26] = '\0';
    printf("Reversed: %s\n", dest);
    
    /* Additional tight loop tests */
    volatile int *vptr = numbers;
    int *rptr = regular_numbers;
    
    /* Comma expression with post-increment */
    for (int i = 0; i < 10; i++) {
        int val = (*vptr++, *rptr++, vptr[-1] + rptr[-1]);
        (void)val;  /* Prevent unused warning */
    }
    
    /* While loop with post-increment in condition */
    volatile char *cp = (volatile char *)buffer1;
    int count = 0;
    while (*cp++ != 0 && count < SIZE) {
        count++;
    }
    
    printf("Counted %d non-zero bytes\n", count);
    
    return 0;
}
