/* test-coverage.gt - Test file to cover all TYPE_* cases in gengtype-state.cc */

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
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[256];

/* TYPE_STRUCT: Plain C struct without GTY markers */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    callback_type d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various type combinations */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_ARRAY (fixed size) */
    int array_field[5];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* TYPE_CALLBACK */
    callback_type handler;
    
    /* TYPE_POINTER to another struct */
    struct plain_struct *plain_ptr;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* Another GC-tracked struct with nested structures */
struct GTY(()) container_struct {
    /* TYPE_STRUCT (embedded) */
    struct plain_struct embedded;
    
    /* TYPE_USER_STRUCT pointer */
    struct user_struct *user_data;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_specific *lang_data;
    
    /* Array of pointers (TYPE_ARRAY of TYPE_POINTER) */
    void *ptr_array[8];
    
    /* Callback array */
    callback_type callbacks[4];
    
    /* Pointer to opaque type (TYPE_UNDEFINED) */
    struct opaque_struct *opaque;
};

/* Union containing GC-tracked pointers */
union GTY(()) tagged_union {
    struct complex_struct *complex;
    struct container_struct *container;
    string_type str;
    int value;
};

/* Type with nested anonymous struct/union */
struct GTY(()) nested_types {
    union {
        int x;
        float y;
    } anonymous_union;
    
    struct {
        int a;
        int b;
    } anonymous_struct;
    
    /* Pointer to callback */
    callback_type (*get_callback)(void);
};

/* Array of unions */
union my_union union_array[10];

/* Struct with bitfields (scalar types) */
struct bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
};

/* Forward declared struct that gets defined later (testing undefined->defined transition) */
struct forward_declared;

struct forward_declared {
    int data;
    struct forward_declared *next;
};

/* Void pointer type */
typedef void *generic_pointer;

/* Const pointer types */
typedef int * const const_ptr;
typedef const int * ptr_to_const;
typedef const int * const const_ptr_to_const;

/* Multi-dimensional array */
int matrix[3][4];

/* Struct with array of structs */
struct GTY(()) struct_with_array {
    struct plain_struct items[5];
    int count;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
};

/* Union with struct members */
union GTY(()) variant_data {
    struct {
        int type;
        void *data;
    } header;
    struct {
        int x, y;
    } point;
    char buffer[16];
};

/* Typedef for a function type (different from function pointer) */
typedef int func_type(double, const char *);

/* Struct containing all basic scalar types */
struct all_scalars {
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

/* End of test-coverage.gt */
