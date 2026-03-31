/* gengtype_test.c - Comprehensive type coverage test for gengtype */
#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
using namespace std;

// For TYPE_LANG_STRUCT triggers
template<typename... Ts>
auto sum_fold(Ts... args) {
    return (args + ...);  // Fold expression (C++17)
}

// Coroutine test (C++20)
#ifdef __cpp_coroutines
struct generator {
    struct promise_type;
    using handle = coroutine_handle<promise_type>;
    
    struct promise_type {
        int current_value;
        
        generator get_return_object() { return generator{handle::from_promise(*this)}; }
        suspend_always initial_suspend() { return {}; }
        suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }
        void unhandled_exception() {}
    };
    
    bool move_next() {
        if (coro.done()) return false;
        coro.resume();
        return !coro.done();
    }
    
    int current_value() { return coro.promise().current_value; }
    
    ~generator() { if (coro) coro.destroy(); }
    
private:
    handle coro;
    explicit generator(handle h) : coro(h) {}
};
#endif

#endif

/* TYPE_UNDEFINED: Forward declaration never defined */
struct undefined_type;  // Incomplete type
typedef struct undefined_type* undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types in various contexts */
int global_int = 42;
static char global_char = 'A';
float global_float = 3.14f;
double global_double = 2.71828;
_Bool global_bool = 1;
unsigned long global_ulong = 1000UL;
signed short global_short = -100;

/* TYPE_STRING: String literals and character arrays */
const char* global_string = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char char_array[50] = "Character Array";
const char* const const_string_ptr = "Constant Pointer";

/* TYPE_STRUCT: C structures with various compositions */
struct inner_struct {
    int x;
    char y;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  // Unnamed bitfield
    signed int value : 8;
};

struct packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

struct flexible_struct {
    int count;
    char data[];  // Flexible array member
};

struct nested_struct {
    struct inner_struct inner;
    int outer_value;
    struct {
        int anonymous_x;
        char anonymous_y;
    } anonymous;
};

/* TYPE_UNION: C-style unions */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char* string_ptr;
    struct inner_struct struct_val;
};

/* TYPE_ARRAY: Arrays of various dimensions */
int int_array_1d[10];
int int_array_2d[5][10];
int int_array_3d[2][3][4];
char* pointer_array[20];
struct inner_struct struct_array[15];

/* TYPE_CALLBACK: Function pointers */
typedef int (*binary_func_t)(int, int);
typedef void (*void_func_t)(void);
typedef char* (*string_func_t)(const char*);

struct callback_container {
    binary_func_t compare;
    void_func_t cleanup;
    string_func_t transform;
};

/* Function prototypes for callbacks */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void do_nothing(void) {}
char* to_upper(const char* str) { 
    static char buffer[256];
    // Simplified implementation
    for(int i = 0; str[i]; i++) {
        buffer[i] = (str[i] >= 'a' && str[i] <= 'z') ? str[i] - 32 : str[i];
    }
    return buffer;
}

#ifdef __cplusplus
/* TYPE_USER_STRUCT: C++ classes with various features */

// Base class with virtual functions
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() = 0;
    void non_virtual_func() {}
    
protected:
    int base_value;
};

// Derived class with single inheritance
class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    virtual void virtual_func() override {}
    
private:
    int derived_value;
};

// Multiple inheritance
class Interface1 {
public:
    virtual void interface1_func() = 0;
};

class Interface2 {
public:
    virtual void interface2_func() = 0;
};

class MultiDerived : public Interface1, public Interface2 {
public:
    virtual void interface1_func() override {}
    virtual void interface2_func() override {}
};

// Template class
template<typename T>
class Container {
public:
    Container() : data() {}
    void set(const T& value) { data = value; }
    T get() const { return data; }
    
private:
    T data;
};

// Class with anonymous union (C++11)
class UnionContainer {
public:
    UnionContainer() : type(0) {}
    
    union {
        int int_value;
        float float_value;
        double double_value;
    };
    
    int type;
};

// Lambda expressions as callbacks
auto lambda_add = [](int a, int b) -> int { return a + b; };
auto lambda_capture = [global_int](int x) -> int { return x + global_int; };

// Structured bindings (C++17)
struct Point { int x; int y; };
auto get_point() -> Point { return {10, 20}; }

// Initializer list usage
std::initializer_list<int> init_list = {1, 2, 3, 4, 5};

#endif

/* Global variables using undefined type pointer */
undefined_ptr_t global_undefined_ptr = NULL;

/* Function using various types */
void process_types(int argc, char** argv) {
    /* TYPE_SCALAR: Local variables */
    int local_int = argc;
    unsigned char local_uchar = 'B';
    short local_short = -50;
    long local_long = 100000L;
    float local_float = 1.234f;
    double local_double = 9.876;
    _Bool local_bool = 0;
    
    /* TYPE_STRUCT: Local instances */
    struct inner_struct local_inner = {10, 'X'};
    struct bitfield_struct local_bitfield = {1, 7, -50};
    struct nested_struct local_nested = {{5, 'Y'}, 100, {20, 'Z'}};
    
    /* TYPE_UNION: Local union */
    union data_union local_union;
    local_union.int_val = 42;
    
    /* TYPE_ARRAY: Local arrays */
    int local_array[argc > 0 ? argc : 1];  // VLA in C mode
    char local_char_array[] = "Local String";
    struct inner_struct local_struct_array[3];
    
    /* TYPE_POINTER: Various pointers */
    int* int_ptr = &local_int;
    const int* const_int_ptr = &global_int;
    volatile float* volatile_float_ptr = &local_float;
    char** argv_ptr = argv;
    int** double_int_ptr = &int_ptr;
    int (*array_ptr)[10] = &int_array_1d;
    struct inner_struct* struct_ptr = &local_inner;
    undefined_ptr_t* undefined_double_ptr = &global_undefined_ptr;
    
    /* TYPE_CALLBACK: Function pointer usage */
    binary_func_t func_ptr = (argc > 1) ? &add : &multiply;
    void_func_t void_ptr = &do_nothing;
    string_func_t string_ptr = &to_upper;
    
    struct callback_container callbacks = {func_ptr, void_ptr, string_ptr};
    
    /* Use function pointers */
    if (callbacks.compare) {
        int result = callbacks.compare(10, 20);
        (void)result;
    }
    
    if (callbacks.transform) {
        char* transformed = callbacks.transform("test");
        (void)transformed;
    }
    
    /* Take addresses to force type analysis */
    (void)&local_array;
    (void)&local_union;
    (void)&callbacks;
    
#ifdef __cplusplus
    /* TYPE_USER_STRUCT: C++ class usage */
    DerivedClass derived_obj;
    MultiDerived multi_obj;
    Container<int> int_container;
    Container<double> double_container;
    UnionContainer union_container;
    
    /* TYPE_LANG_STRUCT: C++ language constructs */
    
    // Lambda usage
    auto result = lambda_add(5, 3);
    (void)result;
    
    // Structured binding
    auto [x, y] = get_point();
    (void)x; (void)y;
    
    // Fold expression
    auto fold_result = sum_fold(1, 2, 3, 4, 5);
    (void)fold_result;
    
    // Initializer list
    for (auto val : init_list) {
        (void)val;
    }
    
    // Coroutine if supported
    #ifdef __cpp_coroutines
    auto coro_func = []() -> generator {
        co_yield 1;
        co_yield 2;
        co_yield 3;
    };
    #endif
    
    // Pointer to member
    int DerivedClass::* member_ptr = nullptr;
    (void)member_ptr;
    
    // Reference types
    int& int_ref = local_int;
    const double& double_ref = global_double;
    (void)int_ref;
    (void)double_ref;
#endif
    
    /* Prevent dead code elimination */
    if (argc > 0) {
        /* Branch 1: Use scalar types */
        global_int += local_int;
        global_float *= local_float;
    } else {
        /* Branch 2: Use pointer types */
        *int_ptr = 100;
        struct_ptr->x = 50;
    }
    
    if (argc > 1) {
        /* Branch 3: Use array types */
        for (int i = 0; i < (argc < 10 ? argc : 10); i++) {
            int_array_1d[i] = i;
            local_array[i] = argv[i][0];
        }
    }
    
    if (argc > 2) {
        /* Branch 4: Use union types */
        local_union.float_val = 3.14f;
        union_container.int_value = 42;
    }
}

/* Main function with observable output */
int main(int argc, char** argv) {
    /* Initialize global undefined pointer */
    global_undefined_ptr = (undefined_ptr_t)&global_int;
    
    /* Create instances of all types */
    struct inner_struct global_inner = {1, 'A'};
    struct bitfield_struct global_bitfield = {0, 5, 100};
    struct packed_struct global_packed = {'X', 123, 'Y'};
    
    union data_union global_data_union;
    global_data_union.int_val = 999;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        int_array_1d[i] = i * i;
        struct_array[i].x = i;
        struct_array[i].y = 'A' + i;
    }
    
    /* Process all types */
    process_types(argc, argv);
    
#ifdef __cplusplus
    /* C++ specific instantiations */
    DerivedClass* derived_ptr = new DerivedClass();
    MultiDerived multi_instance;
    Container<char*> string_container;
    
    /* Use template instantiations */
    int_container.set(42);
    double_container.set(3.14159);
    string_container.set(argv[0]);
    
    /* Complex type combinations */
    Container<Container<int>> nested_container;
    Container<int>* container_ptr = &int_container;
    
    delete derived_ptr;
#endif
    
    /* Observable output to prevent optimization */
    unsigned long hash = 0;
    
    /* Hash of sizeof results */
    hash += sizeof(struct inner_struct);
    hash += sizeof(union data_union);
    hash += sizeof(int_array_1d);
    hash += sizeof(global_undefined_ptr);
    
#ifdef __cplusplus
    hash += sizeof(DerivedClass);
    hash += sizeof(Container<int>);
#endif
    
    /* Hash of addresses */
    hash += (unsigned long)&global_int;
    hash += (unsigned long)&global_string;
    hash += (unsigned long)&global_inner;
    hash += (unsigned long)global_undefined_ptr;
    
    /* Print hash to ensure execution */
    printf("Type coverage hash: %lu\n", hash);
    
    return (int)(hash % 256);
}

/* Additional global to ensure TYPE_UNDEFINED is analyzed */
struct container_with_undefined {
    undefined_ptr_t ptr;
    int valid;
};

struct container_with_undefined global_container = {NULL, 1};
