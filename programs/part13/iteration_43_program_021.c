/* test-coverage.gt - Comprehensive type definitions to cover all gengtype switch cases */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
typedef char char_array[20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
};

/* TYPE_USER_STRUCT: GTY-marked user struct */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    void *b;
    float c;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (flexible array member) */
    char flexible_array[];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int inner_field;
    } inner;
    
    /* TYPE_UNION */
    union my_union union_field;
    
    /* TYPE_CALLBACK */
    callback_type callback_field;
};

/* Another GTY struct with pointer chain */
struct GTY(()) container {
    /* TYPE_POINTER to another GTY type */
    struct complex_struct * GTY((skip)) complex_ptr;
    
    /* TYPE_ARRAY of pointers */
    void * GTY((skip)) ptr_array[5];
    
    /* TYPE_STRING array */
    const char *strings[3];
    
    /* TYPE_SCALAR bitfield */
    unsigned int flags : 4;
    
    /* TYPE_UNION with GTY */
    union GTY((desc ("%1.union_tag"))) tagged_union {
        int as_int;
        void *as_ptr;
        const char *as_string;
    } tagged;
    
    int union_tag;
};

/* TYPE_ARRAY of structs */
struct GTY(()) array_element {
    int value;
    void *data;
};

struct GTY(()) array_container {
    /* Fixed array of structs */
    struct array_element elements[10];
    
    /* Pointer to array */
    struct array_element *element_ptr;
};

/* Linked list structure for pointer traversal */
struct GTY(()) linked_node {
    int data;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
};

/* Union containing various pointer types */
union GTY((desc ("%1.u_type"))) pointer_union {
    int *int_ptr;
    void **void_ptr_ptr;
    struct complex_struct *struct_ptr;
    const char **string_ptr;
    int u_type;
};

/* Structure with callback array */
struct GTY(()) callback_container {
    /* Array of callbacks */
    callback_type callbacks[5];
    
    /* Matrix (2D array) */
    int matrix[3][4];
};

/* Opaque pointer type */
typedef struct opaque_struct * GTY((skip)) opaque_handle;

/* Mixed declarations to ensure all types are processed */
extern string_type global_string;
static callback_type static_callback = 0;
const int global_array[] = {1, 2, 3, 4, 5};

/* A structure with conditional fields */
struct GTY(()) conditional_struct {
    int GTY((condition (false))) unused_field;
    void * GTY((skip)) conditional_ptr;
};

/* Template-like structure (though not C++ templates) */
#define DECLARE_STRUCT(TYPE, NAME) \
struct GTY(()) NAME { \
    TYPE data; \
    struct NAME *next; \
};

/* Instantiate some template-like structs */
DECLARE_STRUCT(int, int_list)
DECLARE_STRUCT(void *, ptr_list)
DECLARE_STRUCT(const char *, string_list)

/* A structure with nested anonymous struct/union */
struct GTY(()) anonymous_member {
    union {
        int x;
        float y;
    } coord;
    
    struct {
        int width;
        int height;
    } size;
};

/* Ensure TYPE_NONE is not triggered (it's for error cases) */
/* gcc_unreachable() should not be reached with valid input */

/* Additional edge cases */

/* Zero-length array */
struct GTY(()) zero_length {
    int count;
    char data[0];  /* TYPE_ARRAY with zero size */
};

/* Multi-dimensional pointer */
typedef int ***triple_ptr;

/* Structure with all basic scalar types */
struct GTY(()) all_scalars {
    char c;
    signed char sc;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
    long double ld;
    _Bool b;
};

/* Complete the opaque struct definition to avoid warnings */
struct opaque_struct {
    int defined_later;
    void *some_data;
};

/* Structure with relative pointer */
struct GTY(()) relative_ptr_struct {
    int offset;
    char data[100];
};

/* Final structure that references most other types */
struct GTY(()) master_struct {
    struct complex_struct *complex;
    struct container *container;
    struct array_container *arrays;
    struct linked_node *list;
    union pointer_union punion;
    struct callback_container *callbacks;
    opaque_handle handle;
    struct conditional_struct conditional;
    struct int_list *int_lists;
    struct anonymous_member anonymous;
    struct zero_length *zero_len;
    struct all_scalars scalars;
    struct relative_ptr_struct relative;
    
    /* TYPE_STRING literal pointer */
    const char *literal;
    
    /* TYPE_CALLBACK that returns a pointer */
    void *(*allocator)(size_t);
    
    /* TYPE_ARRAY of different types */
    int ints[20];
    void *pointers[10];
    callback_type handlers[5];
};
