i₀ = 0
goto loop_header

loop_header:
i₁ = Φ(i₀, i₂)  // Merge point
if (i₁ < n) goto loop_body else goto exit

loop_body:
if (i₁ == 0) goto then_body else goto after_if

then_body:
// body

after_if:
i₂ = i₁ + 1
goto loop_header

exit:
// after loop
