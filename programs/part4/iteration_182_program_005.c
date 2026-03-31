__attribute__((noinline, used))
float compare_valid(int cond) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    
    // Only use valid comparison predicates
    // For example, compare for "less than" (ordered, signaling)
    __m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS);
    
    return _mm_cvtss_f32(c);
}
