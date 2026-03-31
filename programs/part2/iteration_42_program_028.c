// Scalar version (what you wrote)
if (a[i] > b[i]) { out1[i] = a[i] - b[i]; } else { out1[i] = 0; }

// Vectorized version (conceptual)
mask = _mm256_cmpgt_epi32(a_vec, b_vec);  // Compare 8 elements at once
diff = _mm256_sub_epi32(a_vec, b_vec);
result = _mm256_blendv_epi8(zero_vec, diff, mask);  // Blend based on mask
