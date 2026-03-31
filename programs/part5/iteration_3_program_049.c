/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, it should trigger
 * all type kind cases in the switch statement.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(x) do { \
    volatile void* _ptr = (void*)&(x); \
    __asm__ __volatile__("" : : "r"(_ptr) : "memory"); \
} while(0)

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t use_types(void* ptr1, void* ptr2, void* ptr3, void* ptr4) {
    volatile size_t result = 0;
    result += (size_t)ptr1;
    result += (size_t)ptr2;
    result += (size_t)ptr3;
    result += (size_t)ptr4;
    return result;
}

/* ========== TYPE DEFINITIONS ========== */

/* Basic scalar types - should trigger TYPE_SCALAR */
GTY(()) struct ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_value;
    short short_value;
    unsigned int unsigned_integer;
    _Bool boolean;
};

/* String type - should trigger TYPE_STRING */
GTY(()) struct StringTypes {
    const char* constant_string;
    char* mutable_string;
    char fixed_string[64];
    wchar_t* wide_string;
};

/* Nested struct - should trigger TYPE_STRUCT */
GTY(()) struct InnerStruct {
    int inner_data;
    double inner_value;
};

/* User struct - should trigger TYPE_USER_STRUCT */
typedef GTY(()) struct UserDefined {
    int user_id;
    char user_name[32];
    struct InnerStruct* nested;
} UserStruct;

/* Union type - should trigger TYPE_UNION */
GTY(()) union DataUnion {
    int as_int;
    double as_double;
    void* as_pointer;
    char as_bytes[8];
    struct {
        int tag;
        union DataUnion* next;
    } recursive;
};

/* Complex pointer types - should trigger TYPE_POINTER */
GTY(()) struct PointerTypes {
    int* int_ptr;
    void* void_ptr;
    struct ScalarTypes* struct_ptr;
    union DataUnion* union_ptr;
    char** double_ptr;
    
    /* Function pointer - might trigger TYPE_CALLBACK */
    int (*compare_func)(const void*, const void*);
    void (*callback_func)(int, char*);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to function */
    void (*(*complex_func_ptr)(int))(void);
};

/* Array types - should trigger TYPE_ARRAY */
GTY(()) struct ArrayTypes {
    int simple_array[10];
    char multi_dim[5][10];
    struct InnerStruct struct_array[4];
    union DataUnion union_array[8];
    
    /* Flexible array member */
    int flexible_array[];
};

/* Callback structure - should trigger TYPE_CALLBACK */
GTY(()) struct CallbackContainer {
    /* Function pointer field */
    size_t (*compute_size)(struct CallbackContainer*);
    void (*process_data)(void*, int);
    
    /* Nested callback */
    int (*(*get_callback)(void))(int, int);
};

/* Language-specific structure - might trigger TYPE_LANG_STRUCT */
/* Simulating a GCC internal language structure */
GTY(()) struct LangStructure {
    /* Tree node-like structure */
    int code;
    union {
        long intval;
        double realval;
        void* ptrval;
        struct LangStructure* chain;
    } u;
    
    /* Location information */
    struct {
        int line;
        int column;
        const char* filename;
    } location;
    
    /* Type information pointer */
    void* type_info;
};

/* Complex nested structure combining all types */
GTY(()) struct MasterStructure {
    /* Scalar members */
    int id;
    double weight;
    
    /* String member */
    const char* name;
    
    /* Struct member */
    struct ScalarTypes scalars;
    
    /* User struct member */
    UserStruct user_data;
    
    /* Union member */
    union DataUnion variant;
    
    /* Pointer members */
    struct PointerTypes* pointers;
    
    /* Array member */
    struct ArrayTypes arrays;
    
    /* Callback member */
    struct CallbackContainer callbacks;
    
    /* Language structure */
    struct LangStructure* lang_data;
    
    /* Self-referential pointer */
    struct MasterStructure* next;
    
    /* Array of pointers */
    void* ptr_array[5];
    
    /* Union with struct */
    union {
        struct {
            int x, y;
        } point;
        struct {
            float r, g, b, a;
        } color;
    } graphical;
};

/* Another union with complex nesting */
GTY(()) union MegaUnion {
    struct MasterStructure as_struct;
    struct {
        struct PointerTypes ptrs;
        struct ArrayTypes arrs;
    } combined;
    void* as_void_ptr;
    long long as_longlong;
};

/* ========== MAIN FUNCTION ========== */

int main(void) {
    size_t total_size = 0;
    size_t checksum = 0;
    
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct InnerStruct inner = {0};
    UserStruct user_struct = {0};
    union DataUnion data_union;
    struct PointerTypes pointers = {0};
    struct ArrayTypes* arrays = NULL;
    struct CallbackContainer callbacks = {0};
    struct LangStructure lang_struct = {0};
    struct MasterStructure master = {0};
    union MegaUnion mega_union;
    
    /* Take addresses to ensure types are considered */
    void* addr1 = &scalars;
    void* addr2 = &strings;
    void* addr3 = &inner;
    void* addr4 = &user_struct;
    void* addr5 = &data_union;
    void* addr6 = &pointers;
    void* addr7 = &callbacks;
    void* addr8 = &lang_struct;
    void* addr9 = &master;
    void* addr10 = &mega_union;
    
    /* Compute sizes of all types */
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct InnerStruct);
    total_size += sizeof(UserStruct);
    total_size += sizeof(union DataUnion);
    total_size += sizeof(struct PointerTypes);
    total_size += sizeof(struct ArrayTypes);
    total_size += sizeof(struct CallbackContainer);
    total_size += sizeof(struct LangStructure);
    total_size += sizeof(struct MasterStructure);
    total_size += sizeof(union MegaUnion);
    
    /* Take addresses of members to ensure full type traversal */
    checksum += (size_t)&scalars.integer;
    checksum += (size_t)&strings.constant_string;
    checksum += (size_t)&inner.inner_data;
    checksum += (size_t)&user_struct.user_name;
    checksum += (size_t)&data_union.as_int;
    checksum += (size_t)&pointers.int_ptr;
    checksum += (size_t)&callbacks.compute_size;
    checksum += (size_t)&lang_struct.code;
    checksum += (size_t)&master.id;
    checksum += (size_t)&mega_union.as_struct;
    
    /* Access array elements */
    if (arrays) {
        checksum += (size_t)&arrays->simple_array[0];
        checksum += (size_t)&arrays->multi_dim[0][0];
    }
    
    /* Use external function to prevent optimization */
    size_t external_result = use_types(addr1, addr3, addr5, addr7);
    checksum += external_result;
    
    /* Prevent dead code elimination */
    KEEP_ALIVE(scalars);
    KEEP_ALIVE(strings);
    KEEP_ALIVE(inner);
    KEEP_ALIVE(user_struct);
    KEEP_ALIVE(data_union);
    KEEP_ALIVE(pointers);
    KEEP_ALIVE(callbacks);
    KEEP_ALIVE(lang_struct);
    KEEP_ALIVE(master);
    KEEP_ALIVE(mega_union);
    
    /* Print results to ensure code isn't optimized away */
    printf("Type analysis coverage test\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: 0x%zx\n", checksum);
    printf("Address samples: %p, %p, %p\n", addr1, addr5, addr9);
    
    return (int)((checksum + total_size) % 256);
}
