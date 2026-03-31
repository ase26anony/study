// main.cpp - C++ part targeting explicit, mutable, location, segment attributes
#include <iostream>
#include <thread>
#include <atomic>

// Force section attribute for DW_AT_segment
__attribute__((section("mysection")))
int section_var = 42;

// Thread-local for complex location info
thread_local int tls_var = 100;

// Class with explicit constructor
class ExplicitClass {
public:
    int value;
    explicit ExplicitClass(int v) : value(v) {}
    
    // Mutable member for DW_AT_mutable
    mutable int mutable_counter = 0;
    
    int get_value() const {
        // Modifying mutable member in const function
        mutable_counter++;
        return value;
    }
};

// Class with mutable member
class WithMutable {
private:
    int regular;
public:
    mutable int change_me;  // DW_AT_mutable
    
    WithMutable() : regular(1), change_me(2) {}
    
    void modify() const {
        change_me++;  // Allowed even in const function
    }
};

// Template to force type elaboration
template<typename T>
class Container {
    T data;
public:
    explicit Container(const T& d) : data(d) {}  // DW_AT_explicit
    T get() const { return data; }
};

// Function with register variable (may trigger location lists)
void use_register_var() {
    register int reg_var asm("r12") = 123;
    asm volatile("" : : "r"(reg_var));  // Force use
}

// Prototyped function (DW_AT_prototyped)
int prototyped_func(int a, int b, int c);

int prototyped_func(int a, int b, int c) {
    return a + b * c;
}

// Fortran declarations
extern "C" {
    void fortran_sub_(char* str, int* len, int str_len);
    void fortran_derived_type_();
    int fortran_func_();
}

// Ada declarations (if available)
#ifdef __GNUC__
extern "C" {
    void ada_task_test();
    void ada_record_test();
}
#endif

int main() {
    // Use explicit constructor
    ExplicitClass obj1(10);
    ExplicitClass obj2 = ExplicitClass(20);  // Explicit prevents implicit conversion
    
    // Use mutable member
    WithMutable mut;
    mut.modify();
    std::cout << "Mutable value: " << mut.change_me << std::endl;
    
    // Use template with explicit constructor
    Container<int> cont(42);
    std::cout << "Container: " << cont.get() << std::endl;
    
    // Use section variable
    std::cout << "Section var: " << section_var << std::endl;
    
    // Use TLS variable
    tls_var = 200;
    std::cout << "TLS var: " << tls_var << std::endl;
    
    // Use register variable
    use_register_var();
    
    // Call prototyped function
    std::cout << "Prototyped: " << prototyped_func(1, 2, 3) << std::endl;
    
    // Call Fortran code
    char str[] = "Hello";
    int len = 5;
    fortran_sub_(str, &len, 5);
    
    fortran_derived_type_();
    
    int fort_result = fortran_func_();
    std::cout << "Fortran result: " << fort_result << std::endl;
    
    // Prevent dead code elimination
    volatile int keep_alive = obj1.get_value();
    keep_alive += obj2.value;
    
    return 0;
}
