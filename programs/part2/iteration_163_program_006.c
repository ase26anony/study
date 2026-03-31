#ifdef __cplusplus
#include <iostream>
#include <functional>
#include <initializer_list>
#include <tuple>
#include <coroutine>
#include <type_traits>
#include <cstddef>
#endif

/* ========== TYPE_UNDEFINED ========== */
struct undefined_struct;  // Forward declaration, never defined
typedef undefined_struct* undefined_ptr_t;

/* ========== TYPE_SCALAR ========== */
int global_int = 42;
unsigned int global_uint = 100u;
char global_char = 'A';
signed char global_schar = -10;
unsigned char global_uchar = 200;
short global_short = -1000;
unsigned short global_ushort = 2000;
long global_long = 100000L;
unsigned long global_ulong = 200000UL;
long long global_llong = 10000000000LL;
unsigned long long global_ullong = 20000000000ULL;
float global_float = 3.14f;
double global_double = 2.71828;
long double global_ldouble = 1.41421356L;
_Bool global_bool = 1;
#ifdef __cplusplus
bool global_cppbool = true;
#endif

/* ========== TYPE_STRING ========== */
const char* global_cstring = "Hello, World!";
const wchar_t* global_wstring = L"Wide String";
char global_char_array[50] = "Character array";
const char global_const_array[] = "Constant array";

/* ========== TYPE_STRUCT ========== */
struct SimpleStruct {
    int x;
    double y;
    char name[20];
};

struct NestedStruct {
    SimpleStruct inner;
    int outer;
};

struct BitFieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int signed_field : 8;
};

struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct FlexibleArrayStruct {
    int count;
    int data[];  // Flexible array member
};

/* ========== TYPE_USER_STRUCT (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    BaseClass() : base_value(0) {}
    virtual ~BaseClass() {}
    virtual void virtual_func() { std::cout << "Base\n"; }
    int base_value;
protected:
    double protected_value;
private:
    char private_char;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    void virtual_func() override { std::cout << "Derived\n"; }
    int derived_value;
};

class MultipleInheritance : public BaseClass, public SimpleStruct {
public:
    MultipleInheritance() : extra(0) {}
    int extra;
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : value(val) {}
    T get_value() const { return value; }
private:
    T value;
};

template<typename K, typename V>
class Pair {
public:
    Pair(K k, V v) : key(k), value(v) {}
    K key;
    V value;
};
#endif

/* ========== TYPE_UNION ========== */
union SimpleUnion {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union ComplexUnion {
    struct {
        int type;
        union {
            int int_val;
            double double_val;
            char* string_val;
        } data;
    } tagged;
    long long raw;
};

#ifdef __cplusplus
struct StructWithAnonymousUnion {
    int type;
    union {
        int int_member;
        double double_member;
        char* ptr_member;
    };
};
#endif

/* ========== TYPE_POINTER ========== */
int* single_ptr = &global_int;
int** double_ptr = &single_ptr;
const int* const_ptr = &global_int;
volatile int* volatile_ptr = &global_int;
int* restrict restrict_ptr = &global_int;

typedef int (*func_ptr_t)(int, double);
typedef void (*void_func_ptr_t)(void);
typedef const char* (*string_func_ptr_t)(const char*);

#ifdef __cplusplus
typedef void (BaseClass::*member_func_ptr_t)();
typedef int (DerivedClass::*member_data_ptr_t);
#endif

/* ========== TYPE_ARRAY ========== */
int simple_array[10];
int* pointer_array[20];
int multi_dim_array[3][4][5];
SimpleStruct struct_array[5];
int incomplete_array[] = {1, 2, 3, 4, 5};

struct ArrayContainer {
    int fixed[10];
    int* ptr_array[5];
    SimpleStruct structs[3];
};

#ifdef __cplusplus
template<typename T, size_t N>
class ArrayWrapper {
    T data[N];
public:
    T& operator[](size_t idx) { return data[idx]; }
};
#endif

/* ========== TYPE_CALLBACK ========== */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, void*);

struct CallbackContainer {
    Comparator compare;
    Callback notify;
    void* user_data;
};

void register_callback(Callback cb, void* data) {
    if (cb) cb(42, data);
}

int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

#ifdef __cplusplus
template<typename Func>
void process_with_callback(Func f) {
    f(100);
}

std::function<int(int)> create_closure(int base) {
    return [base](int x) { return x + base; };
}
#endif

/* ========== TYPE_LANG_STRUCT (C++ only) ========== */
#ifdef __cplusplus
template<typename... Args>
auto sum_all(Args... args) {
    return (args + ...);  // Fold expression
}

struct CoroutineType {
    struct promise_type {
        CoroutineType get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

CoroutineType dummy_coroutine() {
    co_return;
}

template<typename T>
class WithInitializerList {
public:
    WithInitializerList(std::initializer_list<T> init) {
        for (const auto& item : init) {
            // Process items
        }
    }
};

auto create_lambda_with_capture() {
    int x = 10;
    double y = 3.14;
    return [x, y](int z) -> double {
        return x * y + z;
    };
}
#endif

/* ========== Function declarations ========== */
void use_undefined_pointer(undefined_ptr_t ptr);
void process_scalars(int i, unsigned u, float f, double d);
SimpleStruct create_struct(int x, double y, const char* name);
void use_arrays(int arr[], int size);
void call_via_function_pointer(func_ptr_t fp);

/* ========== Main function ========== */
int main(int argc, char* argv[]) {
    // Use argc to prevent dead code elimination
    int mode = (argc > 1) ? argv[1][0] : 'A';
    
    /* Trigger TYPE_UNDEFINED */
    undefined_ptr_t undef_ptr = nullptr;
    use_undefined_pointer(undef_ptr);
    
    /* Trigger TYPE_SCALAR */
    process_scalars(global_int, global_uint, global_float, global_double);
    
    // Local scalars
    signed char local_schar = -5;
    unsigned short local_ushort = 500;
    long long local_llong = 9999999999LL;
    _Bool local_bool = 0;
    
    /* Trigger TYPE_STRING */
    const char* local_string = "Local string";
    wchar_t wide_local[] = L"Local wide";
    
    /* Trigger TYPE_STRUCT */
    SimpleStruct s1 = create_struct(1, 2.0, "Test");
    NestedStruct ns = {{2, 3.0, "Inner"}, 10};
    BitFieldStruct bfs = {1, 5, 9, -10};
    PackedStruct ps = {'X', 123, 45.67};
    
    // Array of structs
    SimpleStruct structs[3] = {
        {1, 1.1, "First"},
        {2, 2.2, "Second"},
        {3, 3.3, "Third"}
    };
    
    /* Trigger TYPE_USER_STRUCT (C++ only) */
    #ifdef __cplusplus
    BaseClass* base = new DerivedClass();
    DerivedClass derived;
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14);
    Pair<int, const char*> pair(1, "one");
    
    // Use virtual function
    base->virtual_func();
    derived.virtual_func();
    
    // Template instantiation
    ArrayWrapper<int, 10> wrapper;
    wrapper[0] = 100;
    #endif
    
    /* Trigger TYPE_UNION */
    SimpleUnion su;
    su.as_int = 42;
    
    ComplexUnion cu;
    cu.tagged.type = 1;
    cu.tagged.data.int_val = 100;
    
    #ifdef __cplusplus
    StructWithAnonymousUnion sau;
    sau.type = 2;
    sau.double_member = 3.14159;
    #endif
    
    /* Trigger TYPE_POINTER */
    int local_int = 100;
    int* local_ptr = &local_int;
    int** local_dptr = &local_ptr;
    
    // Function pointers
    func_ptr_t fp = nullptr;
    void_func_ptr_t vfp = nullptr;
    
    /* Trigger TYPE_ARRAY */
    int local_array[5] = {1, 2, 3, 4, 5};
    use_arrays(local_array, 5);
    
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    // Variable-length array (C99)
    #ifndef __cplusplus
    int vla_size = (argc > 2) ? atoi(argv[2]) : 10;
    int vla[vla_size];
    for (int i = 0; i < vla_size; i++) vla[i] = i;
    #endif
    
    /* Trigger TYPE_CALLBACK */
    CallbackContainer cc = {compare_ints, nullptr, nullptr};
    
    int data = 42;
    register_callback([](int val, void* d) {
        *(int*)d = val;
    }, &data);
    
    #ifdef __cplusplus
    auto lambda = [](int x) { return x * 2; };
    process_with_callback(lambda);
    
    auto closure = create_closure(10);
    int result = closure(5);
    #endif
    
    /* Trigger TYPE_LANG_STRUCT (C++ only) */
    #ifdef __cplusplus
    // Lambda with different captures
    int capture1 = 10;
    auto lambda1 = [capture1](int x) { return x + capture1; };
    auto lambda2 = [&capture1]() { capture1++; };
    
    // Initializer list
    WithInitializerList<int> wil{1, 2, 3, 4, 5};
    
    // Structured bindings (C++17)
    auto [a, b] = std::make_pair(10, 3.14);
    
    // Fold expression
    int fold_result = sum_all(1, 2, 3, 4, 5);
    
    // Coroutine
    dummy_coroutine();
    
    // Complex lambda with init capture
    auto complex_lambda = [value = 42](int x) mutable {
        value += x;
        return value;
    };
    #endif
    
    /* Generate observable output to prevent optimization */
    unsigned long hash = 0;
    
    // Mix all sizes and addresses
    hash ^= (unsigned long)sizeof(SimpleStruct);
    hash ^= (unsigned long)&global_int;
    hash ^= (unsigned long)global_int;
    hash ^= (unsigned long)&s1;
    #ifdef __cplusplus
    hash ^= (unsigned long)base;
    hash ^= (unsigned long)&derived;
    #endif
    hash ^= (unsigned long)&su;
    hash ^= (unsigned long)local_array[0];
    hash ^= (unsigned long)fp;
    
    // Print something based on mode
    if (mode == 'A') {
        #ifdef __cplusplus
        std::cout << "Hash: " << hash << "\n";
        #else
        printf("Hash: %lu\n", hash);
        #endif
    } else if (mode == 'B') {
        // Different code path
        #ifdef __cplusplus
        std::cout << "Mode B: " << data << "\n";
        #else
        printf("Mode B: %d\n", data);
        #endif
    }
    
    #ifdef __cplusplus
    delete base;
    #endif
    
    return (int)(hash % 256);
}

/* ========== Function definitions ========== */
void use_undefined_pointer(undefined_ptr_t ptr) {
    // Cannot dereference undefined pointer
    (void)ptr;
}

void process_scalars(int i, unsigned u, float f, double d) {
    volatile int result = i + (int)u + (int)f + (int)d;
    (void)result;
}

SimpleStruct create_struct(int x, double y, const char* name) {
    SimpleStruct s = {x, y, ""};
    // Copy name safely
    for (int i = 0; i < 19 && name[i]; i++) {
        s.name[i] = name[i];
    }
    s.name[19] = '\0';
    return s;
}

void use_arrays(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        arr[i] *= 2;
    }
}

void call_via_function_pointer(func_ptr_t fp) {
    if (fp) {
        fp(10, 3.14);
    }
}
