// RUN: %clang_cc1 -falloc-token-mode=typefunchash -verify %s
// RUN: %clang_cc1 -falloc-token-mode=typefunchashpointersplit -verify %s

// Function context tracking for the builtin is not implemented yet.
void foo() {
  __builtin_infer_alloc_token(sizeof(int)); // expected-error {{__builtin_infer_alloc_token is not yet supported with allocation token mode}}
}
