/* test_complex_types.c - Target for gengtype-parse.cc coverage */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func)(int);
typedef int (*(*func_returning_func_ptr)(double))(char);

/* Function pointer with nested parentheses in parameter */
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3])[(4+1)];

/* Array with parenthesized size expression */
double (*data_ptr_array[ (sizeof(int) > 2) ? 5 : 3 ])[7];

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

/* Global struct with initializer using nested braces */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, {NULL, NULL} },
    { .d = 3.14 }
};

/* Array with designated initializers and nested braces */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine constructs in single declarations */
/* Variable with initializer using compound literal containing array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Complex struct with function pointer array */
struct Container {
    int (*(*func_table[3])(struct Nested *))[2];
    struct {
        int x;
        char (*str)[10];
    } inner;
};

/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; double y; } param);

/* 5. Global scope declarations */
int (*(*global_var)(void))[3] = NULL;

struct Data { 
    int (*func)(int); 
    int (*array_func[2])(int, int);
} global_data = { NULL, {NULL, NULL} };

/* Union with nested struct */
union ComplexUnion {
    struct {
        int (*(*func_ptr)(int))[5];
        char data[4][3];
    } s;
    long long ll;
};

/* 6. Helper functions for execution */
int simple_int_func(void) { return 42; }
int func_taking_int(int x) { return x * 2; }
int (*func_returning_int_array(double d))[3] { 
    static int arr[3] = {1,2,3};
    return &arr; 
}
int sum_matrix(int m[][3]) { return m[0][0] + m[1][2]; }

/* Implementation of process function */
void process(int (*table[])[5], struct { int x; double y; } param) {
    /* Do nothing - just for declaration */
    (void)table;
    (void)param;
}

/* Main function to use all constructs */
int main(void) {
    int result = 0;
    
    /* 1. Use complex typedef */
    func_returning_func frf = NULL;
    simple_func sf = simple_int_func;
    
    /* 2. Access nested array from global struct */
    result += global_nested.a[0][0];      /* 1 */
    result += global_nested.a[1][2];      /* 6 */
    result += arr[0][2];                  /* 3 */
    result += arr[1][1];                  /* 5 */
    
    /* 3. Use array of pointers to arrays */
    if (ptr_to_array) {
        result += (*ptr_to_array)[0];     /* 1 */
        ptr_to_array++;
        result += (*ptr_to_array)[1];     /* 4 */
    }
    
    /* 4. Assign to function pointer */
    complex_func_ptr = (int (*(*)(int (*)(double)))(char))func_returning_int_array;
    
    /* 5. Use designated initializer result */
    result += global_nested.u.d > 3.0 ? 1 : 0;  /* 1 */
    
    /* 6. Create and use local complex type */
    struct Container local_container = {
        .func_table = {NULL, NULL, NULL},
        .inner = { .x = 7, .str = NULL }
    };
    result += local_container.inner.x;    /* 7 */
    
    /* 7. Call process function */
    int (*table_array[2])[5];
    struct { int x; double y; } param = { .x = 10, .y = 20.0 };
    process(table_array, param);
    
    /* Total should be: 1+6+3+5+1+4+1+7 = 28 */
    printf("Result: %d\n", result);
    
    return 0;
}
