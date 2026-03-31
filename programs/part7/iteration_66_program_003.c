/* Prevent dead-code elimination */
extern void __attribute__((noinline, noclone)) 
use_ptr(const void *ptr, int size);

/* GCC vector extension */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8f __attribute__((vector_size(32)));

/* Struct for contiguous member assignment */
struct ContigStruct {
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
};

/* Union for testing */
union MixedUnion {
    struct {
        int x;
        int y;
        int z;
    } coords;
    long long ll;
    double dbl;
};

/* Multi-dimensional array */
int md_arr[4][8];

/* Checksum calculation */
static int checksum = 0;

int main(void) {
    /* Requirement 1: Constant-bounded array assignments */
    int arr1[10] = {0};
    constexpr int lo1 = 2;
    constexpr int hi1 = 4;
    
    /* Case 1: count = 3 (hi - lo + 1 = 4 - 2 + 1 = 3) */
    /* This should trigger: count > 2 && TYPE_SIZE * count fits uhwi */
    for (int i = lo1; i <= hi1; i++) {
        arr1[i] = i * 10;
        checksum += arr1[i];
    }
    
    /* Case 2: count = 2 */
    constexpr int lo2 = 5;
    constexpr int hi2 = 6;
    for (int i = lo2; i <= hi2; i++) {
        arr1[i] = i * 20;
        checksum += arr1[i];
    }
    
    /* Case 3: count = 1 */
    constexpr int lo3 = 8;
    constexpr int hi3 = 8;
    arr1[lo3] = 999;
    checksum += arr1[lo3];
    
    /* Requirement 2: Mixed target types */
    /* Memory target with count > 2 */
    volatile int volatile_arr[20];
    constexpr int vlo = 10;
    constexpr int vhi = 15;  /* count = 6 */
    for (int i = vlo; i <= vhi; i++) {
        volatile_arr[i] = i * 3;
        checksum += volatile_arr[i];
    }
    
    /* Atomic target */
    _Atomic int atomic_arr[10];
    constexpr int alo = 1;
    constexpr int ahi = 3;  /* count = 3 */
    for (int i = alo; i <= ahi; i++) {
        atomic_arr[i] = i * 7;
        checksum += atomic_arr[i];
    }
    
    /* Requirement 3: Struct element assignment */
    struct ContigStruct cs = {0};
    
    /* Assign to contiguous members using compound literal */
    /* This treats struct as array-like target */
    struct ContigStruct tmp = {.a = 100, .b = 200, .c = 300, .d = 400};
    cs = tmp;  /* Should trigger count > 2 path */
    checksum += cs.a + cs.b + cs.c + cs.d;
    
    /* Requirement 4: Vector extension slicing */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {10, 20, 30, 40};
    
    /* Assign slice of vector - may not be MEM_P if in register */
    constexpr int vec_lo = 1;
    constexpr int vec_hi = 3;  /* count = 3 */
    for (int i = vec_lo; i <= vec_hi; i++) {
        vec1[i] = vec2[i] * 2;
        checksum += vec1[i];
    }
    
    /* Larger vector */
    v8f fvec1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8f fvec2 = {2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    constexpr int fvec_lo = 2;
    constexpr int fvec_hi = 7;  /* count = 6 */
    for (int i = fvec_lo; i <= fvec_hi; i++) {
        fvec1[i] = fvec2[i] * 1.5f;
        checksum += (int)fvec1[i];
    }
    
    /* Requirement 5: Volatile and atomic qualifiers */
    volatile v4si volatile_vec = {0};
    constexpr int vv_lo = 0;
    constexpr int vv_hi = 2;  /* count = 3 */
    for (int i = vv_lo; i <= vv_hi; i++) {
        volatile_vec[i] = i * 50;
        checksum += volatile_vec[i];
    }
    
    /* Requirement 6: Multi-dimensional array section */
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            md_arr[i][j] = i * 10 + j;
        }
    }
    
    /* Assign to contiguous sub-array: arr[1][2..5] */
    constexpr int md_row = 1;
    constexpr int md_col_lo = 2;
    constexpr int md_col_hi = 5;  /* count = 4 */
    for (int j = md_col_lo; j <= md_col_hi; j++) {
        md_arr[md_row][j] = j * 100;
        checksum += md_arr[md_row][j];
    }
    
    /* Additional test: Union member assignment */
    union MixedUnion mu;
    mu.coords.x = 111;
    mu.coords.y = 222;
    mu.coords.z = 333;  /* 3 contiguous int members */
    checksum += mu.coords.x + mu.coords.y + mu.coords.z;
    
    /* Prevent optimization */
    use_ptr(arr1, sizeof(arr1));
    use_ptr(&cs, sizeof(cs));
    use_ptr(&vec1, sizeof(vec1));
    use_ptr(&fvec1, sizeof(fvec1));
    use_ptr(&volatile_vec, sizeof(volatile_vec));
    use_ptr(md_arr, sizeof(md_arr));
    use_ptr(&mu, sizeof(mu));
    
    return checksum & 0xFF;  /* Return non-zero checksum */
}

/* Dummy function definition to satisfy linker */
void __attribute__((noinline, noclone)) 
use_ptr(const void *ptr, int size) {
    /* Empty - just to prevent optimization */
    (void)ptr;
    (void)size;
}
