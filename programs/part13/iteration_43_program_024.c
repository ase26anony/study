/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef char string_array[][20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC structure */
struct GTY((user)) user_struct {
    void * GTY((skip)) data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void * GTY((skip)) c;
    const char * GTY((skip)) d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void * GTY((skip)) lang_data;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER within struct */
    struct plain_struct * GTY((skip)) plain_ptr;
    
    /* TYPE_ARRAY within struct */
    int numbers[5];
    
    /* TYPE_UNION within struct */
    union my_union values;
    
    /* TYPE_STRING within struct */
    const char * GTY((skip)) name;
    
    /* TYPE_CALLBACK within struct */
    callback_type handler;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int inner_field;
        float inner_float;
    } inner;
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another GTY-marked structure with various field types */
struct GTY(()) another_gty_struct {
    /* TYPE_SCALAR fields */
    scalar_int count;
    enum my_enum state;
    
    /* TYPE_POINTER to TYPE_USER_STRUCT */
    struct user_struct * GTY((skip)) user_data;
    
    /* TYPE_ARRAY of pointers */
    void * GTY((skip)) *ptr_array[8];
    
    /* TYPE_STRING array */
    const char * GTY((skip)) strings[4];
    
    /* TYPE_CALLBACK array */
    callback_type callbacks[3];
    
    /* Nested TYPE_LANG_STRUCT */
    struct lang_specific lang_item;
};

/* Union with GTY marker */
union GTY(()) gty_union {
    struct complex_nested * GTY((skip)) nested;
    struct another_gty_struct * GTY((skip)) another;
    int scalar_value;
};

/* Typedef for a pointer to callback */
typedef callback_type *callback_ptr;

/* Structure containing all major types */
struct GTY(()) master_struct {
    /* TYPE_STRUCT */
    struct plain_struct plain;
    
    /* TYPE_USER_STRUCT */
    struct user_struct user;
    
    /* TYPE_UNION */
    union my_union uni;
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific lang;
    
    /* TYPE_POINTER */
    void * GTY((skip)) generic_ptr;
    
    /* TYPE_ARRAY */
    int matrix[3][3];
    
    /* TYPE_SCALAR */
    scalar_int code;
    scalar_float weight;
    enum my_enum choice;
    
    /* TYPE_STRING */
    const char * GTY((skip)) message;
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_UNDEFINED pointer */
    struct opaque_struct * GTY((skip)) opaque;
    
    /* Nested complex type */
    struct complex_nested complex;
    
    /* Another GTY structure */
    struct another_gty_struct another;
    
    /* GTY union */
    union gty_union gty_uni;
};

/* Global variable declarations for completeness */
struct master_struct GTY((root)) * GTY((skip)) global_master;
int GTY((skip)) global_counter;
const char * GTY((skip)) global_name = "test";
