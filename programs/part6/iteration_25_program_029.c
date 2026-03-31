/* Complex type declarations to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function pointers with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix_ptr)[3][4];

/* 3. Nested structures with arrays and function pointers */
struct Inner {
    int (*calc)(int, int);
    char data[2][10];
};

struct Middle {
    struct Inner inner;
    float (*transform)(float);
    int matrix[2][2];
};

struct Outer {
    struct Middle mid;
    void (*processor)(struct Inner *);
    long values[3];
};

/* 4. Union with complex members */
union ComplexUnion {
    int (*func_array[3])(void);
    struct {
        char (*str_table[2])[15];
        double (*math_op)(double);
    } ops;
    long double big_array[4][2];
};

/* 5. Combined construct in single declaration */
int (*combined_var)(int (*)[3], struct Outer (*)[2])[5];

/* 6. Global variables with initializers using nested braces */
struct Outer global_outer = {
    .mid = {
        .inner = {
            .calc = NULL,
            .data = {{'a','b'}, {'c','d'}}
        },
        .transform = NULL,
        .matrix = {{1,2},{3,4}}
    },
    .processor = NULL,
    .values = {100, 200, 300}
};

int nested_array[2][3][4] = {
    [0] = {
        {1,2,3,4},
        {5,6,7,8},
        {9,10,11,12}
    },
    [1] = {
        {13,14,15,16},
        {17,18,19,20},
        {21,22,23,24}
    }
};

/* 7. Compound literal in global context */
int (*global_compound)[2] = (int[][2]){ 
    {1,2}, 
    {3,4}, 
    {5,6} 
};

/* 8. Function prototype with complex parameters */
void process_data(
    int (*table[])[5], 
    struct Outer (*outer_array)[3],
    union ComplexUnion (*unions)[2]
);

/* 9. Typedef with all three delimiters */
typedef struct {
    int (*methods[3])(void);
    char data[2][(4+1)];
} ObjectType;

/* 10. Another complex global with initializer */
ObjectType global_object = {
    .methods = {NULL, NULL, NULL},
    .data = {{'x','y','z'}, {'a','b','c'}}
};

/* Simple functions for function pointers */
int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
float square(float x) { return x * x; }
double cube(double x) { return x * x * x; }

/* Implementation of process_data */
void process_data(
    int (*table[])[5],
    struct Outer (*outer_array)[3],
    union ComplexUnion (*unions)[2]
) {
    /* Function body not important for parser coverage */
    (void)table;
    (void)outer_array;
    (void)unions;
}

int main(void) {
    /* Use the complex types to ensure they're not optimized away */
    
    /* 1. Use function pointers */
    struct Outer local_outer = global_outer;
    local_outer.mid.inner.calc = add;
    
    /* 2. Access nested array elements */
    int sum = 0;
    sum += nested_array[0][1][2];  /* Should be 7 */
    sum += nested_array[1][2][3];  /* Should be 24 */
    
    /* 3. Use global compound literal */
    sum += global_compound[1][0];  /* Should be 3 */
    
    /* 4. Access structure members */
    sum += local_outer.values[0];  /* Should be 100 */
    
    /* 5. Call through function pointer */
    if (local_outer.mid.inner.calc) {
        sum += local_outer.mid.inner.calc(5, 3);  /* Should be 8 */
    }
    
    /* 6. Create and use local complex type */
    int (*(*local_complex)(int (*)(double)))(char);
    (void)local_complex;  /* Prevent unused variable warning */
    
    /* 7. Use array of function pointers */
    union ComplexUnion u;
    u.ops.math_op = cube;
    if (u.ops.math_op) {
        /* Just reference, don't actually call to avoid math.h dependency */
        sum += (int)u.ops.math_op(2.0);  /* Should add 8 */
    }
    
    /* Total should be: 7 + 24 + 3 + 100 + 8 + 8 = 150 */
    printf("Result: %d\n", sum);
    
    return (sum == 150) ? 0 : 1;
}
