   __m128i acc = _mm_load_si128((__m128i*)init);
   __m128i mul = _mm_load_si128((__m128i*)mul);
   for (int i = 0; i < 5000; i++) {
       __m128i in = _mm_set1_epi32(input[i]);
       acc = _mm_add_epi32(_mm_mullo_epi32(acc, mul), in);
   }
   _mm_store_si128((__m128i*)result, acc);
