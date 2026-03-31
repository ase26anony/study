/* Complex type declarations to exercise consume_balanced parsing */

/* 1. Function pointers with deeply nested signatures */
int (*(*complex_callback)(int (*)(float, int[3]), void*))[10];

/* 2. Multi-dimensional arrays with nested initializers */
int matrix[3][4] = {
    {1, 2, {3, 4}},
    {5, {6, 7}, 8},
    {{9}, 10, 11, 12}
};

/* 3. Structure with anonymous nested structs, unions, and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
            int c : (sizeof(int) * 8 - 8);
        };
        long d;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        char* (*nested_func)(int (*)(char**));
    } inner;
};

/* 4. Typedef with complex grouping */
typedef int (*array_of_funcs[5])(char, ...);
typedef int (*(*nested_ptr_func)(double(*)[10]))(int, ...);

/* 5. GCC attributes with nested parentheses */
struct __attribute__((aligned(32), packed)) AlignedStruct {
    int data[8];
} __attribute__((aligned(64)));

int __attribute__((format(printf, 2, 3)))
    (*format_func)(void*, const char*, ...);

/* 6. Macro to generate delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n], int (*)(int[n][n]))

MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);

/* 7. Complex expressions in sizeof and casts */
size_t sizes[] = {
    sizeof(int[10][20]),
    sizeof(struct Outer),
    sizeof(int (*(*)[5])(void))
};

/* 8. Compound literals with nested delimiters */
int (*get_matrix(void))[4] {
    return (int (*)[4])&(int[3][4]){
        {{1}, {2}, {3}, {4}},
        {{5}, {6}, {7}, {8}},
        {{9}, {10}, {11}, {12}}
    };
}

/* 9. Function with complex parameter types */
void process_data(
    int (*(*callback)(int (*)(float)))[10],
    struct Outer (*items[]),
    __attribute__((unused)) int count
) {
    /* Nested block with local compound literal */
    int local = (int){((int[2]){1, 2})[0]};
    (void)local;
}

/* 10. Union with nested array of function pointers */
union ComplexUnion {
    struct {
        int (*(*funcs[3])(int, ...))(char*);
    } nested;
    long long data;
};

/* Minimal main function */
int main(void) {
    /* Declare and use complex types */
    array_of_funcs func_array = {0};
    struct Outer outer_instance = {0};
    union ComplexUnion cu = {0};
    
    /* Use sizeof on complex types */
    size_t s1 = sizeof(int (*(*)[10])(char, ...));
    size_t s2 = sizeof(outer_instance.func_ptr_arr);
    
    /* Assign NULL to complex pointer */
    complex_callback = NULL;
    
    /* Use compound literal */
    int* ptr = (int*)(int[]){1, 2, 3, {4, 5}};
    
    /* Call function with complex return type */
    int (*result)[4] = get_matrix();
    (void)result;
    
    return 0;
}
