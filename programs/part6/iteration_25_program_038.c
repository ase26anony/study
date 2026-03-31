/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*func_ptr_returning_func_ptr)(double))(char);

/* Global variable using complex function pointer type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(float);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*func_array[2])(void);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long l;
        double d;
    } u;
};

/* Global struct with complex initializer */
char global_char = 'A';
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { &global_char, { NULL, NULL } },
    { .d = 3.14 }
};

/* Struct containing array of function pointers */
struct WithFuncPtrs {
    int (*(*func_ptr_array[3])(void))[2];
    struct Nested nested;
};

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int (*callback)(int); } param);

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 5. More global declarations with all delimiter types */
union ComplexUnion {
    struct {
        int (*func)(int, ...);
        char data[4][2];
    } s;
    void *(*getter)(int[][3]);
};

/* Typedef combining all constructs */
typedef struct {
    int (*(*(*complex)[5])(int[][2]))(void);
    union ComplexUnion u;
} UltimateType;

/* Global instance with initializer */
UltimateType global_ultimate = {
    NULL,
    { .s = { NULL, {{'a','b'},{'c','d'},{'e','f'},{'g','h'}} } }
};

/* 6. Simple functions for function pointers */
int simple_double_func(double d) { return (int)(d * 2); }
int char_func(char c) { return (int)c; }
int int_func(int i) { return i * 2; }
int void_func(void) { return 42; }

/* Function returning pointer to array */
int (*func_returning_ptr_to_array(int x))[2] {
    static int arr[3][2] = {{1,2},{3,4},{5,6}};
    return arr[x % 3];
}

/* Implementation of process function */
void process(int (*table[])[5], struct { int x; int (*callback)(int); } param) {
    /* Do nothing, just for declaration */
}

int main(void) {
    int result = 0;
    
    /* 1. Use complex typedef */
    func_returning_func_ptr local_func_ptr = NULL;
    
    /* 2. Access nested array from global struct */
    result += global_nested.a[0][0];  /* Should add 1 */
    result += global_nested.a[1][2];  /* Should add 6 */
    
    /* 3. Use array with designated initializers */
    result += arr[0][1];  /* Should add 2 */
    result += arr[1][0];  /* Should add 4 */
    
    /* 4. Access compound literal through pointer */
    result += ptr_to_array[0][0];  /* Should add 1 */
    result += ptr_to_array[2][1];  /* Should add 6 */
    
    /* 5. Use global_ultimate union data */
    result += global_ultimate.u.s.data[0][0];  /* Should add 'a' (97) */
    result += global_ultimate.u.s.data[3][1];  /* Should add 'h' (104) */
    
    /* 6. Call simple functions */
    result += void_func();  /* Should add 42 */
    result += int_func(5);  /* Should add 10 */
    
    /* Total should be: 1+6+2+4+1+6+97+104+42+10 = 273 */
    
    printf("Result: %d\n", result);
    
    /* Additional complex local declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* Array of pointers to arrays */
    char (*local_strings[2])[20];
    
    /* Nested struct with initializer */
    struct Nested local_nested = {
        .a = {{7,8,9},{10,11,12}},
        .inner = { &global_char, { &void_func, &int_func } },
        .u = { .l = 100 }
    };
    
    result += local_nested.a[0][2];  /* Add 9 */
    printf("Final result: %d\n", result + 9);  /* Should be 282 */
    
    return 0;
}
