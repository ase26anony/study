/* test-coverage.gt - Comprehensive type definitions to cover all TYPE_* cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[][20];

/* TYPE_STRUCT: Plain C struct without GC annotations */
struct plain_struct {
    int field1;
    float field2;
    enum my_enum field3;
};

/* TYPE_USER_STRUCT: GC-aware struct with user tag */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    struct plain_struct d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    callback_type lang_callback;
};

/* Complex nested types to ensure deep processing */

/* GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum status;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* Nested TYPE_STRUCT */
    struct plain_struct nested_struct;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct * GTY((skip)) user_data;
    
    /* Pointer to TYPE_LANG_STRUCT */
    struct lang_specific *lang_ptr;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Another GC-tracked struct with pointer chain */
struct GTY(()) container {
    struct complex_struct * GTY((tag("0"))) first;
    struct complex_struct * GTY((tag("1"))) second;
    
    /* Array of pointers */
    void * GTY((length("count"))) *item_array;
    int count;
    
    /* Callback array */
    callback_type GTY((skip)) callbacks[3];
};

/* Union containing GC pointers */
union GTY((desc("%1.type"))) tagged_union {
    int type;
    struct complex_struct * GTY((tag("1"))) as_complex;
    struct container * GTY((tag("2"))) as_container;
    string_type GTY((tag("3"))) as_string;
};

/* Type with nested arrays */
struct GTY(()) matrix {
    int rows;
    int cols;
    double * GTY((length("rows * cols"))) data;
};

/* Opaque pointer type (triggers TYPE_UNDEFINED when referenced) */
typedef struct opaque_struct *opaque_ptr;

/* Array of callbacks */
typedef callback_type callback_array[];

/* Struct with all basic types */
struct GTY(()) all_types {
    /* SCALAR */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    enum my_enum e;
    
    /* POINTER */
    void *p;
    int *ip;
    
    /* STRING */
    const char *str;
    
    /* ARRAY */
    int arr[10];
    char str_arr[5][20];
    
    /* CALLBACK */
    callback_type cb;
    
    /* Nested UNION */
    union {
        int x;
        float y;
    } nested_union;
    
    /* Bitfields (SCALAR) */
    unsigned int flags:4;
    unsigned int mode:2;
};

/* Forward declaration that will remain TYPE_UNDEFINED */
struct never_defined;

/* Typedef for undefined type */
typedef struct never_defined *undefined_ptr;

/* Empty struct (edge case) */
struct GTY(()) empty_struct {
};

/* Union with only scalars */
union scalar_union {
    int i;
    float f;
    double d;
};

/* Struct with pointer to itself (recursive type) */
struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct * GTY((skip)) next;
};

/* Array of unions */
typedef union my_union union_array[10];

/* Const pointer types */
typedef int * const const_ptr;
typedef const char * const const_string_ptr;

/* Volatile types */
typedef volatile int volatile_int;
typedef volatile void * volatile_ptr;

/* Struct with volatile members */
struct GTY(()) volatile_struct {
    volatile int v_int;
    volatile void *v_ptr;
};

/* Complete the opaque struct definition to avoid warnings,
   but keep it after its first use to ensure TYPE_UNDEFINED is triggered */
struct opaque_struct {
    int defined_later;
    void *some_data;
};

/* Additional language-specific structures with different tags */
struct GTY((tag("GCC"))) gcc_specific {
    int tree_code;
    void *tree_node;
};

struct GTY((tag("C"))) c_specific {
    int c_type;
    const char *c_name;
};

/* Mixed struct with anonymous union */
struct GTY(()) with_anonymous_union {
    int type;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
    } data;
};

/* Function pointer with complex signature */
typedef void (*complex_callback)(
    struct complex_struct *,
    struct container *,
    callback_type,
    int
);

/* Final catch-all structure referencing everything */
struct GTY(()) master_struct {
    struct all_types *all;
    struct complex_struct *complex;
    struct container *container;
    union tagged_union tagged;
    struct matrix *matrix;
    opaque_ptr opaque;
    struct gcc_specific *gcc;
    struct c_specific *c;
    struct recursive_struct *recursive;
    struct volatile_struct *volatile_struct;
    struct with_anonymous_union *with_union;
    complex_callback master_callback;
    
    /* Array of various pointers */
    void * GTY((length("ptr_count"))) *ptr_array;
    int ptr_count;
    
    /* Multi-dimensional array */
    int matrix_2d[3][4];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer returning pointer */
    struct master_struct * (*self_factory)(void);
};
