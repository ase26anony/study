/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classifier */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct incomplete_struct;           /* TYPE_UNDEFINED */
extern union incomplete_union;             /* TYPE_UNDEFINED */
extern int incomplete_array[];             /* TYPE_UNDEFINED array */

/* Annotated struct for metadata generation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
};

/* Annotated union */
union __attribute__((annotate("gengtype"))) annotated_union {
    int i;
    float f;
    void *p;
};

/* TYPE_SCALAR declarations with qualifiers */
volatile const _Bool volatile_bool __attribute__((unused));                /* TYPE_SCALAR */
const int const_int __attribute__((unused)) = 42;                          /* TYPE_SCALAR */
volatile float volatile_float __attribute__((unused));                     /* TYPE_SCALAR */
static double static_double __attribute__((unused));                       /* TYPE_SCALAR */

/* TYPE_STRING declarations */
char* string_literal __attribute__((unused)) = "Hello, gengtype!";         /* TYPE_STRING */
const char* const const_string __attribute__((unused)) = "Constant";       /* TYPE_STRING */
volatile char* volatile_string_ptr __attribute__((unused));                /* TYPE_STRING via pointer */

/* TYPE_STRUCT declarations */
struct simple_struct {                                                     /* TYPE_STRUCT */
    int a;
    char b;
    float c;
} simple_struct_var __attribute__((unused));

/* TYPE_USER_STRUCT via typedef */
typedef struct {                                                           /* TYPE_USER_STRUCT */
    long id;
    char name[32];
    struct simple_struct nested;
} user_struct_t;

user_struct_t user_struct_var __attribute__((unused));

/* TYPE_UNION declarations */
union simple_union {                                                       /* TYPE_UNION */
    int as_int;
    float as_float;
    char as_char[4];
} union_var __attribute__((unused));

/* TYPE_POINTER declarations - complex qualified pointers */
volatile const int* const volatile_const_ptr __attribute__((unused));      /* TYPE_POINTER */
int* restrict restricted_ptr __attribute__((unused));                      /* TYPE_POINTER */
void (*volatile volatile_func_ptr)(void) __attribute__((unused));          /* TYPE_POINTER */
const struct simple_struct* struct_ptr __attribute__((unused));            /* TYPE_POINTER */

/* TYPE_ARRAY declarations */
int fixed_array[10] __attribute__((unused));                               /* TYPE_ARRAY */
float multi_dim[3][4][5] __attribute__((unused));                         /* TYPE_ARRAY */
extern int extern_array[];                                                 /* TYPE_ARRAY (incomplete) */
char variable_len_array __attribute__((unused)) [];                        /* TYPE_ARRAY (VLA if supported) */

/* TYPE_CALLBACK declarations - function pointers */
typedef int (*comparator_t)(const void*, const void*);                     /* TYPE_CALLBACK */
typedef void (*complex_callback_t)(int, ...) __attribute__((annotate("gengtype"))); /* TYPE_CALLBACK */

/* Complex nested type combining multiple classifications */
struct __attribute__((annotate("gengtype"))) complex_nested {              /* TYPE_STRUCT */
    /* Array of function pointers (TYPE_ARRAY of TYPE_CALLBACK) */
    comparator_t comparators[5];
    
    /* Union containing pointer to struct (TYPE_UNION with TYPE_POINTER) */
    union {
        struct simple_struct* s_ptr;
        user_struct_t* u_ptr;
    } ptr_union;
    
    /* Pointer to array (TYPE_POINTER to TYPE_ARRAY) */
    int (*array_ptr)[10];
    
    /* Nested struct with VLA pointer (if supported) */
    struct {
        size_t len;
        int data[];
    } flexible;
} complex_var __attribute__((unused));

/* Function pointer variables */
comparator_t cmp_func __attribute__((unused));                             /* TYPE_CALLBACK */
complex_callback_t varargs_func __attribute__((unused));                   /* TYPE_CALLBACK */

/* More complex qualified pointers */
int* const* const* triple_ptr __attribute__((unused));                     /* TYPE_POINTER (to pointer to pointer) */
const volatile char* const* volatile_qualified_ptr __attribute__((unused)); /* TYPE_POINTER */

/* Anonymous struct/union */
struct {                                                                   /* TYPE_STRUCT */
    union {                                                                /* TYPE_UNION */
        int x;
        float y;
    } anon_union;
    struct {                                                               /* TYPE_STRUCT */
        char a;
        char b;
    } anon_struct;
} anonymous_agg __attribute__((unused));

/* Use __builtin_types_compatible_p for type comparisons */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* Compare scalar types */
    int scalar_check = __builtin_types_compatible_p(int, float);           /* TYPE_SCALAR vs TYPE_SCALAR */
    
    /* Compare pointer types */
    int ptr_check = __builtin_types_compatible_p(int*, float*);            /* TYPE_POINTER vs TYPE_POINTER */
    
    /* Compare struct vs union */
    int struct_union_check = __builtin_types_compatible_p(
        struct simple_struct, union simple_union);                         /* TYPE_STRUCT vs TYPE_UNION */
    
    /* Compare array types */
    int array_check = __builtin_types_compatible_p(int[10], int[5]);       /* TYPE_ARRAY vs TYPE_ARRAY */
    
    /* Compare function pointers */
    int callback_check = __builtin_types_compatible_p(
        comparator_t, complex_callback_t);                                 /* TYPE_CALLBACK vs TYPE_CALLBACK */
    
    /* Compare qualified vs unqualified */
    int qual_check = __builtin_types_compatible_p(const int, int);         /* TYPE_SCALAR with qualifiers */
    
    /* Compare pointer to scalar vs pointer to struct */
    int mixed_check = __builtin_types_compatible_p(int*, struct simple_struct*); /* TYPE_POINTER different bases */
    
    /* Prevent dead code elimination */
    volatile int dummy = scalar_check + ptr_check + struct_union_check + 
                        array_check + callback_check + qual_check + mixed_check;
    (void)dummy;
}

/* Function using various types */
static void use_types(void) __attribute__((unused));
static void use_types(void) {
    /* Use sizeof on incomplete types (valid in some contexts) */
    size_t sz1 = sizeof(struct incomplete_struct*);    /* TYPE_POINTER to TYPE_UNDEFINED */
    size_t sz2 = sizeof(union incomplete_union*);      /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Use sizeof on arrays */
    size_t sz3 = sizeof(fixed_array);                  /* TYPE_ARRAY */
    size_t sz4 = sizeof(multi_dim);                    /* TYPE_ARRAY */
    
    /* Initialize struct fields */
    simple_struct_var.a = 1;
    simple_struct_var.b = 'A';
    simple_struct_var.c = 3.14f;
    
    /* Use union */
    union_var.as_int = 0xDEADBEEF;
    
    /* Trivial function call via pointer if initialized */
    if (cmp_func) {
        /* Would need actual function to call */
    }
    
    volatile size_t dummy = sz1 + sz2 + sz3 + sz4;
    (void)dummy;
}

/* Main function - minimal runtime logic */
int main(void) {
    /* Ensure types are used to prevent elimination */
    type_comparisons();
    use_types();
    
    /* Trivial operations to keep variables "alive" */
    const_int + 1;                          /* Use scalar */
    string_literal[0];                      /* Use string */
    user_struct_var.id = 100;               /* Use user struct */
    
    /* Return constant */
    return 0;
}

/* Additional external declarations for incomplete types */
struct forward_declared;                    /* TYPE_UNDEFINED */
union forward_declared;                     /* TYPE_UNDEFINED */

/* Use forward declared types in pointers */
struct forward_declared* fwd_ptr __attribute__((unused));
union forward_declared* fwd_uptr __attribute__((unused));

/* Mixed declarations with attributes */
typedef struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} packed_struct_t __attribute__((unused));

/* Aligned types */
typedef struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
} aligned_struct_t __attribute__((unused));

/* Transparent union */
typedef union __attribute__((transparent_union)) trans_union {
    int* ip;
    float* fp;
} trans_union_t __attribute__((unused));

/* Final check: ensure all type categories are represented */
static struct {
    /* TYPE_SCALAR */
    signed char sc;
    unsigned char uc;
    short ss;
    unsigned short us;
    long sl;
    unsigned long ul;
    long long sll;
    unsigned long long ull;
    
    /* TYPE_POINTER to various types */
    _Bool* bp;
    int* ip;
    float* fp;
    double* dp;
    char** spp;  /* pointer to string pointer */
    
    /* TYPE_ARRAY of various types */
    long long ll_array[7];
    unsigned short us_array[3][2];
    
    /* TYPE_CALLBACK variations */
    void (*void_func)(void);
    int (*int_func)(int);
    char* (*str_func)(const char*);
    
    /* TYPE_UNION variations */
    union {
        void* vp;
        int i;
        float f;
    } multi_union;
} comprehensive __attribute__((unused));
