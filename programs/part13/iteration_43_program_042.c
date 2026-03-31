/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int, const char*);
typedef int (*another_callback)(void);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char* string_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: GTY-marked user-defined structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    const char *d;
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
    int array_field[20];
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR */
    int count;
    enum my_enum enum_field;
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* Nested TYPE_UNION */
    union my_union nested_union;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct *user_ptr;
    
    /* Flexible array member (variable-length array) */
    int flexible_array[];
};

/* Another GTY struct with pointer chain */
struct GTY(()) container {
    /* TYPE_POINTER to another GTY struct */
    struct complex_struct *complex;
    
    /* TYPE_ARRAY of pointers */
    struct opaque_struct *ptr_array[8];
    
    /* TYPE_CALLBACK array */
    another_callback callbacks[3];
    
    /* Nested TYPE_STRUCT */
    struct {
        int x;
        int y;
    } point;
};

/* Union containing GTY pointers */
union GTY(()) gty_union {
    struct complex_struct * GTY((tag("0"))) cs_ptr;
    struct container * GTY((tag("1"))) cont_ptr;
    string_type str;
};

/* TYPE_ARRAY with GTY elements */
typedef struct complex_struct * GTY((length("len"))) varray[];
struct GTY(()) has_varray {
    size_t len;
    varray items;
};

/* Forward declared struct that gets defined later (testing undefined->defined transition) */
struct forward_declared;

/* Now define it */
struct GTY(()) forward_declared {
    int value;
    struct forward_declared *next;  /* Linked list */
};

/* Enumeration type (scalar) used in GTY struct */
enum GTY(()) gty_enum {
    STATE_A,
    STATE_B,
    STATE_C
};

/* Struct using the GTY enum */
struct GTY(()) enum_user {
    enum gty_enum state;
    int data;
};

/* Test TYPE_CALLBACK in struct context */
struct GTY(()) callback_container {
    callback_type handler;
    void * GTY((skip)) user_data;  /* Skip this for GC */
};

/* Array of unions */
union mixed_union mixed_array[10];

/* Opaque pointer type */
typedef void (* GTY((callback)) special_callback)(struct complex_struct *);

/* Another language-specific structure with different tag */
struct GTY((tag("GCC"))) gcc_specific {
    int tree_code;
    void *tree_node;
};

/* Union with GTY markers on alternatives */
union GTY((desc("%1.type"))) typed_union {
    struct GTY((tag("0"))) complex_struct *cs;
    struct GTY((tag("1"))) container *cnt;
    int GTY((tag("2"))) scalar;
    const char * GTY((tag("3"))) str;
};

/* Struct containing the typed union */
struct GTY(()) union_holder {
    int type;
    union typed_union value;
};

/* Test string array */
typedef const char * GTY((length("str_count"))) string_vec[];
struct GTY(()) string_container {
    int str_count;
    string_vec strings;
};

/* Callback that returns a pointer */
typedef struct complex_struct* (*factory_callback)(int);

/* Struct with callback factory */
struct GTY(()) object_factory {
    factory_callback create;
    void (*destroy)(struct complex_struct *);
};

/* Additional scalar types */
typedef unsigned long long uint64;
typedef _Bool bool_t;

/* Simple linked list for testing */
struct GTY(()) simple_list {
    int data;
    struct simple_list *next;
};

/* Binary tree node */
struct GTY(()) tree_node {
    int key;
    struct tree_node *left;
    struct tree_node *right;
};

/* Test various pointer indirections */
typedef struct tree_node **node_handle;

/* Array of callback pointers */
typedef void (*func_array[5])(void);

/* Mixed struct with bitfields (scalar) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int value;
};

/* Complete the opaque struct declaration if needed */
struct opaque_struct {
    int revealed;
    void *data;
};

/* Final check: ensure all basic types are referenced */
struct GTY(()) type_collector {
    /* Reference all scalar types */
    scalar_int int_val;
    scalar_float float_val;
    enum my_enum enum_val;
    
    /* Reference pointer types */
    int_ptr int_pointer;
    opaque_ptr opaque_pointer;
    
    /* Reference array types */
    fixed_array numbers;
    
    /* Reference string type */
    string_type message;
    
    /* Reference callback type */
    callback_type handler;
    
    /* Reference all struct types */
    struct plain_struct plain;
    struct user_struct *user;
    struct lang_specific *lang;
    struct gcc_specific *gcc;
    
    /* Reference union types */
    union my_union data_union;
    union gty_union gty_union;
    
    /* Reference complex types */
    struct complex_struct *complex;
    struct container *container;
    struct forward_declared *list;
    
    /* Reference enum types */
    enum gty_enum gty_enum_val;
    
    /* Reference callback types */
    special_callback spec_callback;
    factory_callback factory;
};
