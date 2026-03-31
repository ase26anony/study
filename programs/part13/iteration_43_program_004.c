/* test-coverage.gt - Test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;

/* TYPE_ENUM (handled as TYPE_SCALAR) */
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef void* generic_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[][20];

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    double c;
};

/* TYPE_STRUCT: Plain C struct (no GTY) */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int size;
    callback_type callback;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER inside struct */
    struct opaque_struct *opaque_ptr;
    
    /* TYPE_ARRAY inside struct */
    int numbers[5];
    
    /* TYPE_UNION inside struct */
    union my_union data_union;
    
    /* TYPE_STRING inside struct */
    const char *name;
    
    /* TYPE_CALLBACK inside struct */
    callback_type handler;
    
    /* TYPE_SCALAR inside struct */
    enum my_enum status;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Another GTY structure with pointer chain */
struct GTY(()) pointer_chain {
    struct complex_nested *nested;
    struct pointer_chain *next;
    generic_ptr data;
};

/* Union with GTY marker */
union GTY(()) tagged_union {
    struct complex_nested *as_nested;
    struct pointer_chain *as_chain;
    int as_int;
    const char *as_string;
};

/* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
typedef struct complex_nested *ptr_array[8];

/* Callback that returns a pointer */
typedef struct complex_nested* (*factory_callback)(int);

/* Structure containing array of callbacks */
struct GTY(()) callback_container {
    factory_callback factories[4];
    callback_type handlers[];
};

/* Void type usage (contributes to TYPE_UNDEFINED) */
typedef void void_type;

/* Structure with nested anonymous union */
struct GTY(()) with_anon_union {
    int type;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
    } value;
};

/* Structure with bitfields (TYPE_SCALAR) */
struct GTY(()) with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int value : 8;
};

/* Multi-dimensional array */
typedef int matrix[3][3];

/* Pointer to array */
typedef int (*ptr_to_array)[10];

/* Array of function pointers */
typedef void (*func_ptr_array[5])(void);

/* Const pointer types */
typedef int * const const_ptr;
typedef const int * ptr_to_const;

/* Structure with all basic types */
struct GTY(()) all_types {
    /* Scalars */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    _Bool b;
    enum my_enum e;
    
    /* Pointers */
    void *vp;
    int *ip;
    const char *cp;
    
    /* Arrays */
    int arr1[5];
    char arr2[10];
    
    /* Nested structures */
    struct plain_struct ps;
    struct complex_nested *cnp;
    
    /* Union */
    union my_union u;
    
    /* Callback */
    callback_type cb;
};

/* Forward declaration that will be defined later */
struct forward_declared;

/* Structure that references forward declared type */
struct GTY(()) uses_forward {
    struct forward_declared *fd;
    int count;
};

/* Now define the forward declared structure */
struct GTY(()) forward_declared {
    int id;
    struct uses_forward *uf;
};

/* Self-referential structure */
struct GTY(()) self_ref {
    int data;
    struct self_ref *next;
    struct self_ref *prev;
};

/* Structure with variable-length array of pointers */
struct GTY(()) var_len_array {
    int count;
    struct complex_nested *items[];
};

/* Typedef for a function type (not pointer) */
typedef int func_type(int, float);

/* Structure with function pointer field */
struct GTY(()) has_func_ptr {
    func_type *func;
    int (*another_func)(void);
};

/* Complete the opaque struct declaration if needed */
/* Leaving it undefined maintains TYPE_UNDEFINED coverage */

/* Additional edge cases */

/* Empty structure */
struct GTY(()) empty_struct {
    /* No fields */
};

/* Structure with only bitfields */
struct GTY(()) only_bitfields {
    unsigned int : 4;  /* unnamed bitfield */
    unsigned int field1 : 8;
    unsigned int : 0;  /* zero-width bitfield */
    unsigned int field2 : 16;
};

/* Volatile and const qualified pointers */
struct GTY(()) qualified_ptrs {
    volatile int *volatile_ptr;
    const volatile char *const_volatile_ptr;
    int * const const_ptr;
};

/* Structure with alignment attribute */
struct GTY(()) aligned_struct {
    int data;
} __attribute__((aligned(16)));

/* Packed structure */
struct GTY(()) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Structure with array of structures */
struct GTY(()) array_of_structs {
    struct plain_struct items[3];
    struct complex_nested *ptr_items[2];
};

/* Typedef chain */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 my_int3;

/* Opaque pointer typedef */
typedef struct opaque_struct *opaque_ptr_t;

/* Null callback type */
typedef void (*null_callback)(void);

/* Structure with nested structure */
struct GTY(()) outer_struct {
    struct {
        int inner_field1;
        float inner_field2;
    } inner;
    int outer_field;
};

/* Union with nested anonymous structure */
union GTY(()) complex_union {
    struct {
        int type;
        void *data;
    } s;
    long long int ll;
    double dbl;
};

/* Final structure referencing everything */
struct GTY(()) master_struct {
    struct all_types *all;
    union complex_union cu;
    struct array_of_structs aos;
    struct var_len_array *vla;
    opaque_ptr_t opaque;
    null_callback init;
    struct master_struct *next;
};
