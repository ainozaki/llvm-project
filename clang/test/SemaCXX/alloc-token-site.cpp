// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typesitehashpointersplit
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -fexperimental-new-constant-interpreter -falloc-token-mode=typesitehashpointersplit
// expected-no-diagnostics

constexpr auto function_token_1() {
  return __builtin_infer_alloc_token(sizeof(int));
}

constexpr auto function_token_2() {
  return __builtin_infer_alloc_token(sizeof(int));
}

constexpr bool tokens_equal_in_same_function() {
  auto token1 = __builtin_infer_alloc_token(sizeof(int));
  auto token2 = __builtin_infer_alloc_token(sizeof(int));
  return token1 == token2;
}

constexpr auto pointer_function_token() {
  return __builtin_infer_alloc_token(sizeof(char *));
}

static_assert(function_token_1() != function_token_2());
static_assert(tokens_equal_in_same_function());
static_assert(function_token_1() < 9223372036854775807ULL);
static_assert(pointer_function_token() >= 9223372036854775807ULL);
