/* gengtype_test.c - Test program to trigger all gengtype type categories */

#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// Forward declarations for TYPE_UNDEFINED
struct IncompleteType;  // Never defined - triggers TYPE_UNDEFINED
class NeverDefinedClass;  // Another undefined type
#endif

/* ========== TYPE_UNDEFINED ========== */
// Forward declared but never defined types
struct UndefinedStruct;
typedef struct UndefinedStruct* UndefinedPtr;

#ifdef __cplusplus
extern "C" {
#endif

/* ========== TYPE_SCALAR ========== */
// Global scalar variables
int global_int = 42;
unsigned int global_uint = 4294967295u;
char global_char = 'A';
signed char global_schar = -128;
unsigned char global_uchar = 255;
short global_short = -32768;
unsigned short global_ushort = 65535;
long global_long = -2147483648L;
unsigned long global_ulong = 4294967295UL;
long long global_llong = -9223372036854775807LL;
unsigned long long global_ullong = 18446744073709551615ULL;
float global_float = 3.14159f;
double global_double = 2.718281828459045;
#ifdef __cplusplus
bool global_bool = true;
#endif

/* ========== TYPE_STRING ========== */
// String literals and character arrays
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[100] = "Character array";
const char* const global_const_string = "Constant string pointer";

/* ========== TYPE_STRUCT ========== */
// Basic structure
struct SimpleStruct {
    int x;
    double y;
    char z;
};

// Nested structure
struct OuterStruct {
    struct SimpleStruct inner;
    int outer_field;
};

// Structure with bit-fields
struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
};

// Packed structure
struct PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((packed));

// Structure with flexible array member (C only)
struct FlexArrayStruct {
    int count;
    double data[];  // Flexible array member
};

// Structure containing pointer to undefined type
struct WithUndefinedPtr {
    UndefinedPtr ptr_to_undefined;
    int valid_field;
};

/* ========== TYPE_UNION ========== */
// C-style union
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
    struct SimpleStruct struct_val;
};

// Union within structure
struct UnionContainer {
    int type;
    union {
        int i;
        float f;
        char* s;
    } data;
};

/* ========== TYPE_ARRAY ========== */
// Various arrays
int global_int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double global_double_array[5][3];
struct SimpleStruct global_struct_array[4];
int* global_pointer_array[8];

// Multi-dimensional array
int global_3d_array[2][3][4];

// Array of function pointers
typedef int (*FuncPtr)(int, int);
FuncPtr global_func_ptr_array[5];

/* ========== TYPE_POINTER ========== */
// Various pointers
int* global_int_ptr = &global_int;
const int* global_const_int_ptr = &global_int;
volatile int* global_volatile_int_ptr = &global_int;
int** global_double_ptr = &global_int_ptr;
int* restrict global_restrict_ptr = &global_int;

// Pointers to different types
struct SimpleStruct* global_struct_ptr = 0;
union DataUnion* global_union_ptr = 0;
char** global_string_ptr_ptr = 0;
int (*global_array_ptr)[10] = &global_int_array;

/* ========== TYPE_CALLBACK ========== */
// Function pointer types
typedef int (*BinaryOp)(int, int);
typedef void (*VoidCallback)(void*);
typedef const char* (*StringGenerator)(int);

// Structure with function pointer
struct CallbackContainer {
    BinaryOp operation;
    VoidCallback cleanup;
    StringGenerator generator;
};

// Function using callback
int process_with_callback(int a, int b, BinaryOp op) {
    return op ? op(a, b) : 0;
}

// Actual functions to point to
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void noop_cleanup(void* p) { (void)p; }
const char* int_to_string(int n) { 
    static char buf[20];
    snprintf(buf, sizeof(buf), "%d", n);
    return buf;
}

#ifdef __cplusplus
}  // extern "C"

/* ========== TYPE_USER_STRUCT (C++ Classes) ========== */
// Base class
class BaseClass {
private:
    int private_data;
protected:
    float protected_data;
public:
    BaseClass() : private_data(0), protected_data(0.0f) {}
    virtual ~BaseClass() {}
    virtual void virtual_method() { cout << "BaseClass::virtual_method" << endl; }
    void public_method() { cout << "BaseClass::public_method" << endl; }
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
private:
    double derived_data;
public:
    DerivedClass() : derived_data(0.0) {}
    virtual void virtual_method() override { 
        cout << "DerivedClass::virtual_method" << endl; 
    }
    void derived_only_method() { cout << "DerivedClass::derived_only_method" << endl; }
};

// Multiple inheritance
class Interface1 {
public:
    virtual void interface1_method() = 0;
    virtual ~Interface1() {}
};

class Interface2 {
public:
    virtual void interface2_method() = 0;
    virtual ~Interface2() {}
};

class MultipleInheritanceClass : public BaseClass, public Interface1, public Interface2 {
public:
    virtual void interface1_method() override { 
        cout << "MultipleInheritanceClass::interface1_method" << endl; 
    }
    virtual void interface2_method() override { 
        cout << "MultipleInheritanceClass::interface2_method" << endl; 
    }
    virtual void virtual_method() override { 
        cout << "MultipleInheritanceClass::virtual_method" << endl; 
    }
};

// Template class
template<typename T>
class TemplateClass {
private:
    T data;
public:
    TemplateClass(const T& val) : data(val) {}
    T get_data() const { return data; }
    void set_data(const T& val) { data = val; }
    
    template<typename U>
    U convert() const { return static_cast<U>(data); }
};

// Class with complex members
class ComplexClass {
public:
    ComplexClass() : callback(nullptr) {}
    
    // Method with callback parameter
    void set_callback(function<int(int, int)> cb) {
        callback = cb;
    }
    
    int execute_callback(int a, int b) {
        return callback ? callback(a, b) : 0;
    }
    
private:
    function<int(int, int)> callback;
    TemplateClass<int> template_member{42};
};

/* ========== TYPE_LANG_STRUCT (C++ Language Constructs) ========== */
// Lambda expressions with different captures
auto lambda_no_capture = [](int x) { return x * 2; };
auto lambda_by_value = [global_int](int x) { return x + global_int; };
auto lambda_by_ref = [&global_double](int x) { return x + static_cast<int>(global_double); };
auto lambda_mixed = [global_int, &global_double](int x, int y) { 
    return x * y + global_int + static_cast<int>(global_double); 
};

// Function using initializer_list
int sum_initializer_list(initializer_list<int> lst) {
    int sum = 0;
    for (auto val : lst) {
        sum += val;
    }
    return sum;
}

// Structured bindings (C++17)
pair<int, string> get_pair() {
    return {42, "answer"};
}

// Fold expression (C++17)
template<typename... Args>
auto sum_fold(Args... args) {
    return (args + ...);
}

// Coroutine-related types (C++20)
#ifdef __cpp_coroutines
struct SimpleCoroutine {
    struct promise_type {
        SimpleCoroutine get_return_object() { return {}; }
        suspend_never initial_suspend() { return {}; }
        suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};
#endif

// Template instantiations with complex types
TemplateClass<int> template_int_instance(100);
TemplateClass<double> template_double_instance(3.14159);
TemplateClass<BaseClass*> template_ptr_instance(nullptr);

// Alias template with complex type
template<typename T>
using ComplexAlias = TemplateClass<pair<T, function<void(T)>>>;

#endif  // __cplusplus

/* ========== Main Program ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* TYPE_SCALAR - Local scalars */
    int local_int = 123;
    unsigned int local_uint = 456u;
    char local_char = 'Z';
    float local_float = 1.234f;
    double local_double = 5.678;
#ifdef __cplusplus
    bool local_bool = false;
#endif
    
    /* TYPE_STRUCT - Local structure instances */
    struct SimpleStruct local_struct = {10, 20.5, 'X'};
    struct OuterStruct local_outer = {{1, 2.0, 'A'}, 100};
    struct BitFieldStruct local_bitfield = {1, 7, 15, -128};
    struct PackedStruct local_packed = {'B', 999, 777};
    
    /* TYPE_UNION - Local union instances */
    union DataUnion local_union;
    local_union.int_val = 42;
    
    struct UnionContainer local_union_container;
    local_union_container.type = 1;
    local_union_container.data.i = 100;
    
    /* TYPE_ARRAY - Local arrays */
    int local_array[5] = {1, 2, 3, 4, 5};
    struct SimpleStruct local_struct_array[2] = {{1, 1.0, 'A'}, {2, 2.0, 'B'}};
    
    /* TYPE_POINTER - Take addresses */
    int* local_int_ptr = &local_int;
    struct SimpleStruct* local_struct_ptr = &local_struct;
    union DataUnion* local_union_ptr = &local_union;
    
    /* TYPE_CALLBACK - Use function pointers */
    BinaryOp current_op = (mode == 'A') ? add : multiply;
    int result = process_with_callback(10, 20, current_op);
    
    struct CallbackContainer callbacks = {add, noop_cleanup, int_to_string};
    int callback_result = callbacks.operation(5, 6);
    const char* str_result = callbacks.generator(42);
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT - C++ class instances */
    BaseClass base_instance;
    DerivedClass derived_instance;
    MultipleInheritanceClass multiple_instance;
    
    BaseClass* poly_ptr = (mode == 'A') ? static_cast<BaseClass*>(&derived_instance) : &base_instance;
    poly_ptr->virtual_method();
    
    // Use template classes
    TemplateClass<int> local_template(999);
    int template_val = local_template.get_data();
    
    ComplexClass complex_instance;
    complex_instance.set_callback([](int a, int b) { return a + b; });
    int complex_result = complex_instance.execute_callback(10, 20);
    
    /* TYPE_LANG_STRUCT - C++ language constructs */
    // Use lambdas
    int lambda_result = lambda_no_capture(21);
    lambda_result += lambda_by_value(10);
    lambda_result += lambda_by_ref(5);
    
    // Use initializer_list
    int init_list_sum = sum_initializer_list({1, 2, 3, 4, 5});
    
    // Use structured bindings
    auto [num, text] = get_pair();
    
    // Use fold expression
    int fold_sum = sum_fold(1, 2, 3, 4, 5);
    
    // Use complex alias
    ComplexAlias<int> alias_instance({42, [](int x) { cout << x << endl; }});
#endif
    
    /* TYPE_UNDEFINED - Use pointer to undefined type */
    // This pointer exists but points to undefined type
    UndefinedPtr undefined_pointer = 0;
    
    // Create structure with undefined pointer
    struct WithUndefinedPtr undefined_container = {0, 123};
    
    /* Generate observable output based on all types */
    unsigned long hash = 0;
    
    // Mix sizes and addresses into a hash
    hash ^= (unsigned long)sizeof(local_struct);
    hash ^= (unsigned long)&local_int;
    hash ^= (unsigned long)local_int_ptr;
    hash ^= (unsigned long)result;
    hash ^= (unsigned long)callback_result;
    
#ifdef __cplusplus
    hash ^= (unsigned long)&base_instance;
    hash ^= (unsigned long)template_val;
    hash ^= (unsigned long)complex_result;
    hash ^= (unsigned long)lambda_result;
    hash ^= (unsigned long)init_list_sum;
    hash ^= (unsigned long)fold_sum;
    hash ^= (unsigned long)num;
#endif
    
    hash ^= (unsigned long)&undefined_container;
    
    // Print hash to ensure execution
    printf("Type coverage hash: 0x%lx\n", hash);
    
    // Use mode to select different code paths
    switch (mode) {
        case 'A':
            printf("Mode A: %d + %d = %d\n", 10, 20, result);
            break;
        case 'B':
            printf("Mode B: %d * %d = %d\n", 10, 20, result);
            break;
        case 'C':
            printf("Mode C: Callback result = %d\n", callback_result);
            break;
        default:
            printf("Default mode: String = %s\n", str_result);
            break;
    }
    
    return (int)(hash % 256);
}
