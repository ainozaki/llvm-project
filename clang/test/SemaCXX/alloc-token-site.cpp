// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typesitehashpointersplit
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -fexperimental-new-constant-interpreter -falloc-token-mode=typesitehashpointersplit
// expected-no-diagnostics

constexpr auto site_token_1 = __builtin_infer_alloc_token(sizeof(int));
constexpr auto site_token_2 = __builtin_infer_alloc_token(sizeof(int));
constexpr auto pointer_site_token =
    __builtin_infer_alloc_token(sizeof(char *));

static_assert(site_token_1 != site_token_2);
static_assert(site_token_1 < 9223372036854775807ULL);
static_assert(pointer_site_token >= 9223372036854775807ULL);
