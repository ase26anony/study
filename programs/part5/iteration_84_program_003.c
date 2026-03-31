/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (sizeof(struct { int a; }) / 4))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ( { [ ( { } ) ] } )
#else
#define EXTRA_NESTING ( { } )
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Outer {
    /* Function pointer with nested parentheses and struct */
    int (*complex_fp)(int (*callback)(int[2][3]), 
                      struct { 
                          int a; 
                          union { 
                              char c; 
                              double d; 
                          } u; 
                      } s);
    
    /* Array of function pointers */
    void (*fp_array[3])(int, ...);
};

/* Requirement 4: Multi-dimensional array with complex dimensions */
int multi_dim[(2+3)*4][5][sizeof(struct { char c; int i; double d; }) / 8];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    char data[16];
    int __attribute__((deprecated)) old_field;
} __attribute__((aligned(32)));

/* Union with nested anonymous struct */
union Container {
    struct {
        int (*nested_fp)(int (*)(int), float);
        char data[10];
    } inner;
    long value;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int initialized = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } nested;
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2, {3}} }, 
    [1] = { .x = 4, .y = {5, 6}, .nested = { .z = (7 + (8)) } }
};

/* Typedef with complex array type */
typedef int (*(*complex_typedef)[5])(int, ...);

/* Requirement 2 usage */
ARRAY_MACRO;

/* Enum with attribute */
enum __attribute__((flag_enum)) Flags {
    FLAG_A = 1 << 0,
    FLAG_B = 1 << 1,
    FLAG_C = 1 << 2
};

/* Function declaration with nested parameter */
void process(struct Outer (*handler)(int, 
                                     struct { 
                                         int (*method)(int[][5]); 
                                     })) 
    __attribute__((nonnull(1)));

/* Requirement 4: Array with macro in dimension */
int dynamic_size[COMPLEX_PARENS(10)];

/* Nested anonymous struct in parameter */
int calculate(int, struct { int coeff[3]; double factors[]; } *);

/* Requirement 6: Conditional type definition */
#ifdef USE_EXTRA
struct ConditionalType {
    int field[EXTRA_NESTING];
    union {
        struct {
            char *ptr;
        } s;
        long l;
    } u;
};
#endif

/* Main function to avoid dead code elimination */
int main(void) {
    struct Outer o = {0};
    struct PackedStruct ps = {{0}};
    
    /* Use some variables to prevent optimization */
    int result = initialized + multi_dim[0][0][0] + dynamic_size[0];
    (void)result;
    (void)o;
    (void)ps;
    
    return 0;
}

/* Additional complex cases */

/* Function pointer returning pointer to array */
int (*(*signal(int sig, void (*func)(int)))[5])(int);

/* Struct containing all delimiter types deeply nested */
struct AllDelimiters {
    int (*fp1)(int (*)(int[({2})]), struct { int a[3]; });  /* Mixed ()[]{} */
    int arr1[sizeof(struct { int x; char y[(2+3)*4]; })];
    struct {
        union {
            int i;
            char c[5];
        } u;
    } nested;
};

/* Macro that expands to contain all delimiters */
#define ULTIMATE_NEST { { ( [ { ( { [ ] } ) } ] ) } }

/* Variable using the ultimate nest macro */
int ultimate_var[] = ULTIMATE_NEST;

/* Final typedef with everything */
typedef struct {
    int (*(*member1))(int, ...);
    int member2[((2*3)+4)];
    struct __attribute__((packed)) {
        char data[8];
    } member3;
} UltimateType;
