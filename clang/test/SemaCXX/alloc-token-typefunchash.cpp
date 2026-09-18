// RUN: %clang_cc1 -falloc-token-mode=typefunchash -verify %s
// RUN: %clang_cc1 -falloc-token-mode=typefunchashpointersplit -verify %s
// RUN: %clang_cc1 -falloc-token-mode=typefunchashpointersplit -DCHECK_POINTER_SPLIT -verify %s
// expected-no-diagnostics

void foo() {
  unsigned long t = __builtin_infer_alloc_token(sizeof(int));
  (void)t;
}

#ifdef CHECK_POINTER_SPLIT
// Pointer types set the high bit.
static_assert(__builtin_infer_alloc_token(sizeof(char *)) >= (1ULL << 63));
static_assert(__builtin_infer_alloc_token(sizeof(int)) < (1ULL << 63));
#endif
