float compare_valid() {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(2.0f);
    __m128 c = _mm_cmp_ss(a, b, _CMP_LT_OS);  // 1.0 < 2.0
    return _mm_cvtss_f32(c);  // Returns 0xFFFFFFFF (true) as float
}
