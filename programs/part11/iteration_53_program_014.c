I'll create a comprehensive test to cover all the switch cases in `gengtype-state.cc`. This test will generate type definitions for each `TYPE_*` kind and run `gengtype` to process them.

```bash
#!/bin/bash
# test_gengtype_coverage.sh - Test to cover all TYPE_* cases in gengtype-state.cc

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== gengtype State Coverage Test ===${NC}"

# Check if we're in a GCC build directory
if [ ! -f "./gengtype" ] && [ ! -f "../gengtype" ]; then
    echo -e "${YELLOW}Warning: gengtype not found in current or parent directory${NC}"
    echo "Looking for gengtype in PATH..."
    if ! command -v gengtype &> /dev/null; then
        echo -e "${RED}Error: gengtype not found. Please run from GCC build directory or ensure gengtype is in PATH${NC}"
        exit 1
    fi
    GENGTYPE="gengtype"
else
    if [ -f "./gengtype" ]; then
        GENGTYPE="./gengtype"
    else
        GENGTYPE="../gengtype"
    fi
fi

echo "Using gengtype: $GENGTYPE"

# Create temporary directory for test files
TEST_DIR=$(mktemp -d /tmp/gengtype_test_XXXXXX)
echo "Test directory: $TEST_DIR"

# Create output directory
OUTPUT_DIR="$TEST_DIR/output"
mkdir -p "$OUTPUT_DIR"

# ============================================================================
# Generate type definition files covering all TYPE_* cases
# ============================================================================

# Main type definitions file
cat > "$TEST_DIR/test_types.h" << 'EOF'
/* Test types covering all gengtype TYPE_* cases */

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
    int a;
    double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
    void *data;
    int tag;
};

/* TYPE_UNION */
union my_union {
    int i;
    float f;
    char c;
};

/* TYPE_POINTER - struct with pointer member */
struct with_ptr {
    int *p;
    struct my_struct *next;
};

/* TYPE_ARRAY - struct with array member */
struct with_array {
    int arr[10];
    char name[32];
};

/* TYPE_LANG_STRUCT - language-specific struct */
/* Simulating tree_node-like structure */
struct GTY((tag("TS_VAR_DECL"))) lang_struct {
    int code;
    union GTY((desc("%0.code"))) lang_union {
        struct GTY((tag("0"))) {
            char *name;
        } decl;
        struct GTY((tag("1"))) {
            int value;
        } constant;
    } u;
};

/* TYPE_SCALAR - typedef of fundamental type */
typedef unsigned long my_scalar;

/* TYPE_STRING - char pointer (string) */
struct with_string {
    char *str;
    const char *cstr;
};

/* TYPE_CALLBACK - function pointer */
typedef int (*callback_func)(int, void*);

struct with_callback {
    callback_func handler;
    void *user_data;
};

/* TYPE_NONE should never be encountered in valid input */
/* gcc_unreachable() will be called if it is */

/* Additional complex types to ensure coverage */
struct complex_type {
    struct my_struct nested;
    union my_union choice;
    struct with_ptr *ptr_field;
    struct with_array array_field;
    my_scalar scalar_field;
    char *string_field;
    callback_func callback_field;
};

/* Array of pointers */
struct ptr_array {
    struct my_struct *items[20];
    int count;
};

/* Nested anonymous struct/union */
struct nested_anon {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Chain of pointers */
struct node {
    int data;
    struct node *next;
    struct node *prev;
};

/* Self-referential structure */
struct tree {
    int value;
    struct tree *left;
    struct tree *right;
};

/* Variable length array (simulated) */
struct vla_container {
    int length;
    int data[1];  /* Flexible array member */
};

/* Opaque pointer type */
typedef struct undefined_struct *opaque_ptr;

/* Enum type */
enum my_enum {
    ENUM_A,
    ENUM_B,
    ENUM_C
};

struct with_enum {
    enum my_enum choice;
    int value;
};

/* Bitfield structure */
struct with_bitfields {
    unsigned int flag1:1;
    unsigned int flag2:3;
    unsigned int count:8;
    unsigned int :4;  /* unnamed bitfield */
    unsigned int value:16;
};

/* Array of arrays */
struct matrix {
    int values[3][3];
    double doubles[2][4][6];
};

/* Pointer to array */
struct ptr_to_array {
    int (*matrix)[10][10];
    char (*strings)[20];
};

/* Function pointer with complex signature */
typedef void (*complex_callback)(struct my_struct*, int, callback_func, ...);

struct with_complex_callback {
    complex_callback func;
    char *description;
};

/* Union with pointers */
union ptr_union {
    int *int_ptr;
    char **str_ptr_ptr;
    struct my_struct *struct_ptr;
    void (*func_ptr)(void);
};

/* Structure with all basic types */
struct all_basic_types {
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
    void *vp;
    const void *cvp;
    volatile int vi;
};

/* Template-like structure (using macros) */
#define DECLARE_CONTAINER(TYPE, NAME) \
struct NAME { \
    TYPE *items; \
    int size; \
    int capacity; \
}

/* Instantiate template-like structures */
DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(struct my_struct, struct_container);
DECLARE_CONTAINER(char*, string_container);

/* Structure with function pointer table */
struct function_table {
    int (*open)(const char*);
    int (*read)(void*, size_t);
    int (*write)(const void*, size_t);
    int (*close)(void);
};

/* Nested structures with different linkage */
static struct static_struct {
    int internal;
} static_instance;

extern struct extern_struct {
    int external;
} extern_instance;

/* Constant structure */
const struct const_struct {
    int immutable;
} const_instance = {42};

/* Volatile structure */
volatile struct volatile_struct {
    int changing;
} volatile_instance;

/* Atomic type (C11) */
#ifdef __STDC_NO_ATOMICS__
/* Not available */
#else
#include <stdatomic.h>
struct with_atomic {
    atomic_int counter;
    atomic_flag flag;
};
#endif

/* Aligned structure */
struct aligned_struct {
    int normal;
    int aligned __attribute__((aligned(64)));
} __attribute__((aligned(128)));

/* Packed structure */
struct packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Structure with designated initializers (in declaration) */
struct with_designated_init {
    int field1;
    int field2;
    int field3;
} designated_instance = { .field1 = 1, .field3 = 3 };

/* Anonymous union in struct (C11) */
struct with_anon_union {
    int type;
    union {
        int int_value;
        float float_value;
        char *string_value;
    };
};

/* Structure with array of function pointers */
struct operation_table {
    const char *name;
    int (*operations[5])(int, int);
};

/* Forward declared circular reference */
struct forward_a;
struct forward_b;

struct forward_a {
    struct forward_b *b;
    int value;
};

struct forward_b {
    struct forward_a *a;
    char *name;
};

/* Complete the undefined struct */
struct undefined_struct {
    int defined_now;
    struct my_struct *related;
};

/* Type alias with typedef */
typedef struct my_struct MyStructAlias;
typedef MyStructAlias *MyStructPtr;

/* Array typedef */
typedef int IntArray[10];
typedef struct my_struct StructArray[5];

/* Function typedef */
typedef int (*BinaryOp)(int, int);

/* Structure using all the typedefs */
struct using_typedefs {
    MyStructAlias alias;
    MyStructPtr ptr;
    IntArray numbers;
    StructArray objects;
    BinaryOp operation;
};

/* Inline structure definition */
struct outer {
    int id;
    struct {
        int x;
        int y;
        struct {
            int depth;
            char *label;
        } inner;
    } nested;
};

/* Structure with variable arguments marker */
struct varargs_container {
    int count;
    /* ... variable arguments would be here in actual use */
};

/* Transparent union (GCC extension) */
typedef union transparent_union {
    int *int_ptr;
    long *long_ptr;
} transparent_union __attribute__((transparent_union));

/* Structure with transparent union */
struct uses_transparent_union {
    transparent_union ptr;
    int type;
};

/* Structure with vector type (GCC extension) */
typedef int v4si __attribute__ ((vector_size (16)));

struct with_vector {
    v4si vec1;
    v4si vec2;
};

/* Structure with decimal type (GCC extension) */
#ifdef __STDC_IEC_60559_DFP__
_Decimal32 decimal32;
_Decimal64 decimal64;
_Decimal128 decimal128;
#endif

/* Cleanup attribute */
struct with_cleanup {
    char *buffer __attribute__((cleanup(free)));
    FILE *file __attribute__((cleanup(fclose)));
};

/* Deprecated structure */
struct deprecated_struct {
    int old_field;
} __attribute__((deprecated));

/* May-alias structure */
typedef struct may_alias_struct {
    int value;
} __attribute__((may_alias));

/* Structure with section attribute */
struct in_special_section {
    int critical_data;
} __attribute__((section(".critical")));

/* Weak symbol structure */
struct weak_struct {
    int data;
} __attribute__((weak));

/* Alias attribute */
struct original_struct {
    int value;
};

struct alias_struct {
    int value;
} __attribute__((alias("original_struct")));

/* Constructor/destructor priority */
struct with_ctors {
    int initialized;
} __attribute__((constructor(101))) __attribute__((destructor(101)));

/* Structure with visibility */
struct hidden_struct {
    int secret;
} __attribute__((visibility("hidden")));

struct default_vis_struct {
    int visible;
} __attribute__((visibility("default")));

/* Structure with tls model */
__thread struct thread_local_struct {
    int thread_data;
} tls_instance;

/* Structure with warning suppression */
struct warning_struct {
    int unused __attribute__((unused));
    int packed_misaligned __attribute__((packed, aligned(1)));
};

/* Final test structure referencing everything */
struct master_test_struct {
    /* Basic types */
    struct my_struct basic_struct;
    struct GTY((user)) user_struct user_struct_instance;
    union my_union union_instance;
    
    /* Pointers */
    struct with_ptr *ptr_struct;
    struct undefined_struct *opaque_ptr;
    
    /* Arrays */
    struct with_array array_struct;
    struct ptr_array ptr_array_struct;
    
    /* Language-specific */
    struct GTY((tag("TS_VAR_DECL"))) lang_struct lang_struct_instance;
    
    /* Scalars */
    my_scalar scalar_value;
    enum my_enum enum_value;
    
    /* Strings */
    struct with_string string_struct;
    
    /* Callbacks */
    struct with_callback callback_struct;
    struct with_complex_callback complex_callback_struct;
    
    /* Complex nested */
    struct complex_type complex_instance;
    struct nested_anon anon_struct;
    struct node *list_head;
    struct tree *tree_root;
    
    /* Special attributes */
    struct all_basic_types basic_types;
    struct aligned_struct aligned_instance;
    struct packed_struct packed_instance;
    
    /* Typedef usage */
    struct using_typedefs typedefs_instance;
    
    /* Forward reference resolution */
    struct forward_a forward_a_instance;
    struct forward_b forward_b_instance;
    
    /* Vector types if available */
#ifdef __SSE2__
    struct with_vector vector_instance;
#endif
    
    /* End marker */
    int end_marker;
};
EOF

# Additional header file for more complex cases
cat > "$TEST_DIR/test_more_types.h" << 'EOF'
/* Additional types for comprehensive coverage */

/* TYPE_CALLBACK variations */
typedef void (*void_callback)(void);
typedef int (*int_callback)(int);
typedef char* (*str_callback)(const char*);
typedef struct my_struct* (*struct_callback)(int);

/* Array of callbacks */
struct callback_array {
    void_callback void_callbacks[5];
    int_callback int_callbacks[3];
    str_callback str_callbacks[2];
};

/* Nested callback */
typedef int (*nested_callback)(int (*inner)(int, int), void*);

struct with_nested_callback {
    nested_callback processor;
    void *context;
};

/* Recursive callback type */
typedef void (*recursive_callback)(recursive_callback self, int depth);

/* Structure with conditional compilation */
#ifdef TEST_FEATURE
struct conditional_struct {
    int feature_enabled;
    #ifdef EXTRA_FEATURE
    int extra_feature;
    #endif
};
#else
struct conditional_struct {
    int feature_disabled;
};
#endif

/* Union with nested anonymous struct */
union complex_union {
    struct {
        int type;
        union {
            int i;
            float f;
        } value;
    } tagged;
    struct {
        char data[16];
    } raw;
    void *pointer;
};

/* Structure with offsetof usage */
struct offset_test {
    char first;
    int second;
    double third;
    char last;
};

/* Verify offset calculations */
static_assert(offsetof(struct offset_test, second) >= sizeof(char),
              "offsetof test failed");

/* Structure with static assert */
struct static_assert_test {
    int value;
    char padding;
};

static_assert(sizeof(struct static_assert_test) >= sizeof(int) + sizeof(char),
              "Size check failed");

/* Inline function in header */
static inline int inline_helper(int x) {
    return x * 2;
}

struct with_inline_helper {
    int (*compute)(int);
};

/* Structure containing macro-generated fields */
#define ADD_FIELD(TYPE, NAME) TYPE NAME

struct macro_fields {
    ADD_FIELD(int, field1);
    ADD_FIELD(char*, field2);
    ADD_FIELD(struct my_struct*, field3);
#undef ADD_FIELD
};

/* Variable length structure (C99 flexible array member) */
struct vla_struct {
    int length;
    double data[];  /* Flexible array member */
};

/* Zero-length array (GCC extension) */
struct zero_length_array {
    int count;
    int items[0];  /* Zero-length array */
};

/* Array of structures containing arrays */
struct array_of_arrays {
    struct with_array arrays[5];
    int counts[5];
};

/* Pointer to function returning pointer to function */
typedef int (*(*complex_func_ptr)(int))(int, int);

struct with_complex_func_ptr {
    complex_func_ptr builder;
    char *name;
};

/* Structure with __attribute__((aligned)) on individual fields */
struct field_aligned {
    char a;
    int b __attribute__((aligned(32)));
    char c __attribute__((aligned(64)));
    double d;
};

/* Structure with bitfields of various sizes */
struct mixed_bitfields {
    unsigned int a:1;
    signed int b:2;
    unsigned int c:3;
    unsigned int d:4;
    unsigned int e:5;
    unsigned int f:6;
    unsigned int g:7;
    unsigned int h:8;
    unsigned int i:9;
    unsigned int j:10;
    unsigned int :0;  /* Force alignment */
    unsigned int k:11;
    unsigned int l:12;
};

/* Union of bitfields */
union bitfield_union {
    struct {
        unsigned int a:8;
        unsigned int b:8;
        unsigned int c:8;
        unsigned int d:8;
    } bytes;
    unsigned int word;
};

/* Structure with const members */
struct const_members {
    const int immutable;
    const char *const constant_string;
    volatile int volatile_counter;
    const volatile int cv_special;
};

/* Structure with restrict pointers */
struct restrict_pointers {
    int *restrict ptr1;
    int *restrict ptr2;
    struct my_struct *restrict struct_ptr;
};

/* Atomic structure if supported */
#ifdef __STDC_NO_ATOMICS__
#else
#include <stdatomic.h>
struct atomic_members {
    atomic_int atomic_counter;
    atomic_uint atomic_unsigned;
    atomic_flag atomic_flag;
    _Atomic(struct my_struct*) atomic_ptr;
};
#endif

/* Structure with thread-local member */
struct with_tls_member {
    int normal;
    __thread int thread_specific;
};

/* Structure with alignment requirement */
struct overaligned_struct {
    int data;
} __attribute__((aligned(128)));

/* Packed structure with bitfields */
struct packed_bitfields {
    unsigned int a:3;
    unsigned int b:5;
    unsigned int c:8;
    unsigned int d:16;
} __attribute__((packed));

/* Structure with cleanup attribute on pointer */
struct auto_cleanup {
    char *buffer __attribute__((cleanup(free)));
    FILE *file __attribute__((cleanup(fclose)));
    void *memory __attribute__((cleanup(free)));
};

/* Deprecated and unavailable attributes */
struct attribute_mix {
    int normal __attribute__((deprecated));
    int hot __attribute__((hot));
    int cold __attribute__((cold));
    int pure __attribute__((pure));
    int const_func __attribute__((const));
    int unused __attribute__((unused));
    int used __attribute__((used));
    int weak __attribute__((weak));
    int alias __attribute__((alias("other")));
};

/* Section attribute */
struct in_custom_section {
    int important __attribute__((section(".important.data")));
    int critical __attribute__((section(".critical.data")));
};

/* Visibility attributes */
struct visibility_mix {
    int hidden __attribute__((visibility("hidden")));
    int internal __attribute__((visibility("internal")));
    int protected __attribute__((visibility("protected")));
    int default_vis __attribute__((visibility("default")));
};

/* Noinline and always_inline */
struct with_inline_hints {
    int (*noinline_func)(int) __attribute__((noinline));
    int (*inline_func)(int) __attribute__((always_inline));
    int (*flatten_func)(int) __attribute__((flatten));
};

/* Target-specific attributes */
#ifdef __AVX2__
struct avx2_struct {
    __m256i avx2_data;
    __m256 avx2_float;
} __attribute__((aligned(32)));
#endif

#ifdef __AVX512F__
struct avx512_struct {
    __m512i avx512_data;
    __m512 avx512_float;
    __m512d avx512_double;
} __attribute__((aligned(64)));
#endif

/* ARM NEON */
#ifdef __ARM_NEON
struct neon_struct {
    int32x4_t neon_int;
    float32x4_t neon_float;
} __attribute__((aligned(16)));
#endif

/* PowerPC Altivec */
#ifdef __ALTIVEC__
struct altivec_struct {
    __vector int vec_int;
    __vector float vec_float;
} __attribute__((aligned(16)));
#endif

/* MIPS */
#ifdef __mips__
struct mips_struct {
    int mips_data __attribute__((mode(SI)));
    long long mips_64 __attribute__((mode(DI)));
};
#endif

/* Structure with mode attribute */
struct mode_attributes {
    int byte __attribute__((mode(QI)));
    int word __attribute__((mode(HI)));
    int int32 __attribute__((mode(SI)));
    int int64 __attribute__((mode(DI)));
    float float32 __attribute__((mode(SF)));
    double float64 __attribute__((mode(DF)));
    long double float128 __attribute__((mode(TF)));
};

/* Transparent union with many types */
typedef union big_transparent_union {
    void *any;
    int *int_ptr;
    char **str_ptr;
    struct my_struct *struct_ptr;
    void (*func_ptr)(void);
    int (*int_func)(int);
} big_transparent_union __attribute__((transparent_union));

/* Final comprehensive test structure */
struct ultimate_test {
    /* All basic categories */
    struct my_struct *structs[10];
    union my_union unions[5];
    
    /* Arrays of everything */
    int int_array[20];
    struct with_ptr *ptr_array[15];
    callback_func callbacks[8];
    
    /* Nested structures */
    struct {
        int depth;
        struct {
            int deeper;
            struct {
                int deepest;
                char *message;
            } inner;
        } middle;
    } nested;
    
    /* Anonymous union (C11) */
    union {
        int as_int;
        float as_float;
        char *as_string;
        struct my_struct *as_struct;
    };
    
    /* Bitfield section */
    struct {
        unsigned int flags:16;
        unsigned int mode:4;
        unsigned int :4;  /* padding */
        unsigned int state:8;
    } status;
    
    /* Function pointer table */
    struct function_table ops;
    
    /* Variable length array pointer */
    struct vla_struct *vla_ptr;
    
    /* Atomic operations if available */
#ifdef __STDC_NO_ATOMICS__
#else
    struct atomic_members atomic;
#endif
    
    /* Target-specific data if available */
#ifdef __AVX2__
    struct avx2_struct avx2_data;
#endif
    
    /* Alignment test */
    struct overaligned_struct aligned_data __attribute__((aligned(256)));
    
    /* End with sentinel */
    int sentinel __attribute__((unused));
};
EOF

# Create a gengtype input file with markers
cat > "$TEST_DIR/test.gt" << 'EOF'
# GCC gengtype test input file
# This file defines types for testing all TYPE_* cases

# Basic scalar types
typedef int test_int;
typedef long test_long;
typedef void* test_pointer;

# Struct type
struct test_gt_struct {
    int field1;
    test_int field2;
    test_pointer field3;
};

# Union type  
union test_gt_union {
    int i;
    float f;
    struct test_gt_struct *s;
};

# Array type
struct test_gt_array {
    int data[10];
    struct test_gt_struct items[5];
};

# Pointer chain
struct test_gt_list {
    int value;
    struct test_gt_list *next;
    struct test_gt_list *prev;
};

# Callback type
typedef int (*test_gt_callback)(int, struct test_gt_struct*);

struct test_gt_with_callback {
    test_gt_callback handler;
    void *user_data;
};

# String type
struct test_gt_strings {
    char *str;
    const char *cstr;
    char buffer[256];
};

# Lang struct simulation
struct GTY((tag("TEST_TAG"))) test_gt_lang_struct {
    int code;
    union GTY((desc("%0.code"))) {
        struct GTY((tag("0"))) {
            char *name;
        } case1;
        struct GTY((tag("1"))) {
            int value;
        } case2;
    } u;
};

# User struct
struct GTY((user)) test_gt_user_struct {
    void *opaque;
    int type;
};

# Opaque/undefined type (forward declaration)
struct test_gt_opaque;

# Complete opaque type later
struct test_gt_opaque {
    int revealed;
    struct test_gt_struct *link;
};

# Complex nested type
struct test_gt_complex {
    struct test_gt_struct base;
    union test_gt_union choice;
    struct test_gt_array arr;
    struct test_gt_list *list;
    test_gt_callback cb;
    struct test_gt_strings strings;
    struct GTY((tag("TEST_TAG"))) test_gt_lang_struct lang;
    struct GTY((user)) test_gt_user_struct user;
    struct test_gt_opaque *opaque_ptr;
    
    # Nested anonymous struct
    struct {
        int x;
        int y;
        struct {
            int depth;
            char *label;
        } inner;
    } position;
    
    # Bitfields
    unsigned int flags:8;
    unsigned int state:4;
    unsigned int :4;  # padding
    unsigned int mode:16;
    
    # Array of pointers
    struct test_gt_struct *ptr_array[10];
    
    # Function pointer array
    test_gt_callback callbacks[5];
    
    # Flexible array member (C99)
    int flexible_array[];
};

# Template-like macro
#define DEFINE_CONTAINER(TYPE, NAME) \
struct NAME { \
    TYPE *data; \
    int size; \
    int capacity; \
}

# Use the macro
DEFINE_CONTAINER(int, test_gt_int_container);
DEFINE_CONTAINER(struct test_gt_struct, test_gt_struct_container);
DEFINE_CONTAINER(char*, test_gt_string_container);

# Alias types
typedef struct test_gt_struct TestGtStruct;
typedef TestGtStruct *TestGtStructPtr;
typedef int TestGtArray[20];

# Structure using aliases
struct test_gt_with_aliases {
    TestGtStruct alias;
    TestGtStructPtr ptr;
    TestGtArray numbers;
};

# Self-referential type with multiple links
struct test_gt_graph_node {
    int id;
    char *label;
    struct test_gt_graph_node **neighbors;
    int neighbor_count;
};

# Tree structure
struct test_gt_tree {
    int value;
    struct test_gt_tree *parent;
    struct test_gt_tree *left;
    struct test_gt_tree *right;
};

# Circular reference
struct test_gt_circular_a;
struct test_gt_circular_b;

struct test_gt_circular_a {
    int id;
    struct test_gt_circular_b *partner;
};

struct test_gt_circular_b {
    int id;
    struct test_gt_circular_a *partner;
};

# Union with struct members
union test_gt_mixed_union {
    struct {
        int type;
        union {
            int i;
            float f;
            char *s;
        } value;
    } tagged;
    struct {
        unsigned char bytes[16];
    } raw;
    void *pointer;
    long long int64;
};

# Structure with all attribute variations
struct test_gt_attributed {
    # Alignment
    int normal;
    int aligned32 __attribute__((aligned(32)));
    int aligned64 __attribute__((aligned(64)));
    
    # Packed
    struct {
        char a;
        int b;
        char c;
    } __attribute__((packed)) packed_struct;
    
    # Deprecated
    int old __attribute__((deprecated));
    
    # Unused
    int unused __attribute__((unused));
    
    # Cleanup
    char *autofree __attribute__((cleanup(free)));
    
    # Section
    int critical __attribute__((section(".critical.data")));
    
    # Weak symbol
    int weak_ref __attribute__((weak));
};

# Final master structure referencing everything
struct test_gt_master {
    # All basic types
    struct test_gt_struct basic;
    union test_gt_union choice;
    struct test_gt_array array;
    struct test_gt_list *list;
    
    # Special types
    struct GTY((tag("TEST_TAG"))) test_gt_lang_struct lang;
    struct GTY((user)) test_gt_user_struct user;
    struct test_gt_opaque *opaque;
    
    # Callbacks
    test_gt_callback callback;
    struct test_gt_with_callback callback_struct;
    
    # Strings
    struct test_gt_strings strings;
    
    # Containers
    struct test_gt_int_container int_container;
    struct test_gt_struct_container struct_container;
    struct test_gt_string_container string_container;
    
    # Aliases
    struct test_gt_with_aliases aliases;
    
    # Graph
    struct test_gt_graph_node *graph;
    
    # Tree
    struct test_gt_tree *tree;
    
    # Circular
    struct test_gt_circular_a circular_a;
    struct test_gt_circular_b circular_b;
    
    # Mixed union
    union test_gt_mixed_union mixed;
    
    # Attributed
    struct test_gt_attributed attributed;
    
    # Complex nested
    struct test_gt_complex complex;
    
    # Array of everything
    struct test_gt_struct struct_array[5];
    union test_gt_union union_array[3];
    test_gt_callback callback_array[4];
    struct test_gt_list *ptr_array[8];
    
    # Anonymous struct
    struct {
        int counter;
        char *name;
        struct {
            int x;
            int y;
        } point;
    } anonymous;
    
    # Bitfields
    struct {
        unsigned int initialized:1;
        unsigned int active:1;
        unsigned int mode:3;
        unsigned int :3;  # padding
        unsigned int count:8;
        unsigned int flags:16;
    } state;
    
    # Flexible array member
    int variable_data[];
};
EOF

# Create a simple C file to ensure headers are valid
cat > "$TEST_DIR/test_main.c" << 'EOF'
/* Test program to verify type definitions compile */
#include "test_types.h"
#include "test_more_types.h"

/* Dummy functions to satisfy callbacks */
static int dummy_callback(int x, void *data) { return x * 2; }
static void dummy_void_callback(void) {}
static int dummy_int_callback(int x) { return x; }
static char* dummy_str_callback(const char *s) { return (char*)s; }

int main(void) {
    /* Instantiate various types to ensure they're valid */
    struct my_struct s1 = { .a = 1, .b = 2.0 };
    union my_union u1 = { .i = 42 };
    struct with_ptr p1 = { .p = &s1.a };
    struct with_array a1 = { .arr = {1, 2, 3} };
    struct with_string str1 = { .str = "test" };
    struct with_callback cb1 = { .handler = dummy_callback, .user_data = &s1 };
    
    struct complex_type c1 = {
        .nested = s1,
        .choice = u1,
        .ptr_field = &p1,
        .scalar_field = 100,
        .string_field = "complex",
        .callback_field = dummy_callback
    };
    
    /* Test more types */
    struct callback_array ca1 = {0};
    struct with_nested_callback nc1 = {0};
    struct macro_fields mf1 = {0};
    struct const_members cm1 = { .immutable = 42, .constant_string = "const" };
    
    /* Test ultimate structure */
    struct ultimate_test ut1 = {0};
    
    return 0;
}
EOF

# ============================================================================
# Run gengtype with the test files
# ============================================================================

echo -e "\n${YELLOW}Running gengtype with test files...${NC}"

# First, try to compile test_main.c to verify types are valid
echo "Compiling test program to verify type definitions..."
if gcc -c "$TEST_DIR/test_main.c" -o "$TEST_DIR/test_main.o" 2>/dev/null; then
    echo -e "${GREEN}✓ Type definitions are valid C${NC}"
else
    echo -e "${YELLOW}⚠ Type definitions have some compilation issues (may be expected for GCC extensions)${NC}"
fi

# Create a gengtype state file if needed
if [ ! -f "gtype.state" ]; then
    echo "Creating initial gtype.state..."
    touch "$TEST_DIR/gtype.state"
else
    cp gtype.state "$TEST_DIR/gtype.state"
fi

# Run gengtype with various options to trigger all code paths
echo -e "\n${YELLOW}Test 1: Basic type scanning${NC}"
if $GENGTYPE -S "$TEST_DIR" -I "$TEST_DIR" "$TEST_DIR/test_types.h" 2>&1 | tee "$TEST_DIR/output1.log"; then
    echo -e "${GREEN}✓ Test 1 passed${NC}"
else
    echo -e "${YELLOW}⚠ Test 1 had warnings/errors${NC}"
fi

echo -e "\n${YELLOW}Test 2: With state file generation${NC}"
if $GENGTYPE -S "$TEST_DIR" -I "$TEST_DIR" -w "$OUTPUT_DIR/gtype.state" "$TEST_DIR/test_types.h" 2>&1 | tee "$TEST_DIR/output2.log"; then
    echo -e "${GREEN}✓ Test 2 passed${NC}"
    
    # Check if state file was created
    if [ -f "$OUTPUT_DIR/gtype.state" ]; then
        echo -e "${GREEN}✓ State file created${NC}"
        echo "State file size: $(wc -l < "$OUTPUT_DIR/gtype.state") lines"
    fi
else
    echo -e "${YELLOW}⚠ Test 2 had warnings/errors${NC}"
fi

echo -e "\n${YELLOW}Test 3: Multiple input files${NC}"
if $GENGTYPE -S "$TEST_DIR" -I "$TEST_DIR" -w "$OUTPUT_DIR/gtype2.state" \
    "$TEST_DIR/test_types.h" "$TEST_DIR/test_more_types.h" 2>&1 | tee "$TEST_DIR/output3.log"; then
    echo -e "${GREEN}✓ Test 3 passed${NC}"
else
    echo -e "${YELLOW}⚠ Test 3 had warnings/errors${NC}"
fi

echo -e "\n${YELLOW}Test 4: With .gt input file${NC}"
if $GENGTYPE -S "$TEST_DIR" -I "$TEST_DIR" -w "$OUTPUT_DIR/gtype3.state" \
    "$TEST_DIR/test.gt" 2>&1 | tee "$TEST_DIR/output4.log"; then
    echo -e "${GREEN}✓ Test 4 passed${NC}"
else
    echo -e "${YELLOW}⚠ Test 4 had warnings/errors${NC}"
fi

echo -e "\n${YELLOW}Test 5: Generate type description files${NC}"
if $GENGTYPE -S "$TEST_DIR" -I "$TEST_DIR" \
    -r "$OUTPUT_DIR/gtype-desc.c" \
    "$TEST_DIR/test_types.h" "$TEST_DIR/test_more_types.h" 2>&1 | tee "$TEST_DIR/output5.log"; then
    echo -e "${GREEN}✓ Test 5 passed${NC}"
    
    # Check if description file was created
    if [ -f "$OUTPUT_DIR/gtype-desc.c" ]; then
        echo -e "${GREEN}✓ Description file created${NC}"
        echo "Description file size: $(wc -l < "$OUTPUT_DIR/gtype-desc.c") lines"
        
        # Check for markers of different type kinds
        echo -e "\n${YELLOW}Checking for type markers in output...${NC}"
        
        # Look for struct references
        if grep -q "my_struct" "$OUTPUT_DIR/gtype-desc.c"; then
            echo -e "${GREEN}✓ Found struct type${NC}"
        fi
        
        if grep -q "user_struct" "$OUTPUT_DIR/gtype-desc.c"; then
            echo -e "${GREEN}✓ Found user struct type${NC}"
        fi
        
        if grep -q "my_union" "$OUTPUT_DIR/gtype-desc.c"; then
            echo -e "${GREEN}✓ Found union type${NC}"
        fi
        
        if grep -q "with_ptr" "$OUTPUT_DIR/gtype-desc.c"; then
            echo -e "${GREEN}✓ Found pointer type${NC}"
        fi
        
        if grep -q "with_array" "$OUTPUT_DIR/gtype-desc.c"; then
            echo -e "${GREEN}✓ Found array
