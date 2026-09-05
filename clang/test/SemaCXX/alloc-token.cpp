// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-max=0
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -fexperimental-new-constant-interpreter
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typehash -DMODE_TYPEHASH
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typefunchash -DMODE_TYPEFUNCHASH
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typefunchashpointersplit -DMODE_TYPEFUNCHASHPOINTERSPLIT
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-mode=typefunchashpointersplit -DMODE_TYPEFUNCHASHPOINTERSPLIT -fexperimental-new-constant-interpreter
// RUN: %clang_cc1 -triple x86_64-linux-gnu -std=c++23 -fsyntax-only -verify %s -falloc-token-max=2 -DTOKEN_MAX=2
// RUN: %clang_cc1 -triple arm-linux-androideabi -std=c++23 -fsyntax-only -verify %s -falloc-token-max=2 -DTOKEN_MAX=2
// RUN: %clang_cc1 -triple arm-linux-androideabi -std=c++23 -fsyntax-only -verify %s -falloc-token-max=2 -DTOKEN_MAX=2 -fexperimental-new-constant-interpreter

#if !__has_builtin(__builtin_infer_alloc_token)
#error "missing __builtin_infer_alloc_token"
#endif

struct NoPtr {
  int x;
  long y;
};

struct WithPtr {
  int a;
  char *buf;
};

// Check specific known values; these are guaranteed to be stable.
#ifdef MODE_TYPEHASH
static_assert(__builtin_infer_alloc_token(sizeof(int)) == 2689373973731826898ULL);
static_assert(__builtin_infer_alloc_token(sizeof(char*)) == 2250492667400517147ULL);
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr)) == 7465259095297095368ULL);
static_assert(__builtin_infer_alloc_token(sizeof(WithPtr)) == 11898882936532569145ULL);
#elif defined(MODE_TYPEFUNCHASH)
static_assert(__builtin_infer_alloc_token(sizeof(int)) == 11800231689914271711ULL);
static_assert(__builtin_infer_alloc_token(sizeof(char*)) == 6045774506664513975ULL);
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr)) == 4181086577478151473ULL);
static_assert(__builtin_infer_alloc_token(sizeof(WithPtr)) == 240793509323179847ULL);
#elif defined(MODE_TYPEFUNCHASHPOINTERSPLIT)
static_assert(__builtin_infer_alloc_token(sizeof(int)) == 2576859653059495904ULL);
static_assert(__builtin_infer_alloc_token(sizeof(char*)) == 15269146543519289782ULL);
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr)) == 4181086577478151473ULL);
static_assert(__builtin_infer_alloc_token(sizeof(WithPtr)) == 9464165546177955654ULL);
#elif defined(TOKEN_MAX)
#  if TOKEN_MAX == 2
static_assert(__builtin_infer_alloc_token(sizeof(int)) == 0);
static_assert(__builtin_infer_alloc_token(sizeof(char*)) == 1);
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr)) == 0);
static_assert(__builtin_infer_alloc_token(sizeof(WithPtr)) == 1);
#  else
#    error "unhandled TOKEN_MAX case"
#  endif
#else
static_assert(__builtin_infer_alloc_token(sizeof(int)) == 2689373973731826898ULL);
static_assert(__builtin_infer_alloc_token(sizeof(char*)) == 11473864704255292954ULL);
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr)) == 7465259095297095368ULL);
static_assert(__builtin_infer_alloc_token(sizeof(WithPtr)) == 11898882936532569145ULL);
#endif

// Template function.
template <typename T>
constexpr unsigned long get_token() {
  return __builtin_infer_alloc_token(sizeof(T));
}

#if !defined(MODE_TYPEFUNCHASH) && !defined(MODE_TYPEFUNCHASHPOINTERSPLIT)
static_assert(__builtin_infer_alloc_token(sizeof(int)) == get_token<int>());

// Test complex expressions.
static_assert(__builtin_constant_p(__builtin_infer_alloc_token(sizeof(int))));
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr) * 2, 1) == get_token<NoPtr>());
static_assert(__builtin_infer_alloc_token(1, 4 + sizeof(NoPtr)) == get_token<NoPtr>());
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr) << 8) == get_token<NoPtr>());

// Test usable as a template param.
template <unsigned long ID, typename T>
struct token_for_type {
  static_assert(ID == get_token<T>());
  static constexpr unsigned long value = ID;
};
static_assert(token_for_type<__builtin_infer_alloc_token(sizeof(int)), int>::value == get_token<int>());
#else
// Test complex expressions.
static_assert(__builtin_constant_p(__builtin_infer_alloc_token(sizeof(int))));
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr) * 2, 1) == __builtin_infer_alloc_token(sizeof(NoPtr)));
static_assert(__builtin_infer_alloc_token(1, 4 + sizeof(NoPtr)) == __builtin_infer_alloc_token(sizeof(NoPtr)));
static_assert(__builtin_infer_alloc_token(sizeof(NoPtr) << 8) == __builtin_infer_alloc_token(sizeof(NoPtr)));

// Test usable as a template param.
template <unsigned long ID, typename T>
struct token_for_type {
  static_assert(ID == __builtin_infer_alloc_token(sizeof(T)));
  static constexpr unsigned long value = ID;
};
static_assert(token_for_type<__builtin_infer_alloc_token(sizeof(int)), int>::value == __builtin_infer_alloc_token(sizeof(int)));
#endif

constexpr unsigned long func_a() {
  return __builtin_infer_alloc_token(sizeof(int));
}
constexpr unsigned long func_b() {
  return __builtin_infer_alloc_token(sizeof(int));
}
#if defined(MODE_TYPEFUNCHASH) || defined(MODE_TYPEFUNCHASHPOINTERSPLIT)
static_assert(func_a() != func_b());
static_assert(func_a() != __builtin_infer_alloc_token(sizeof(int)));
#else
static_assert(func_a() == func_b());
#endif

template <typename T = void>
void template_test() {
  __builtin_infer_alloc_token(T()); // no error if not instantiated
}

template <typename T>
void negative_template_test() {
  __builtin_infer_alloc_token(T()); // expected-error {{argument may not have 'void' type}}
}

void negative_tests() {
  __builtin_infer_alloc_token(); // expected-error {{too few arguments to function call}}
  __builtin_infer_alloc_token((void)0); // expected-error {{argument may not have 'void' type}}
  negative_template_test<void>(); // expected-note {{in instantiation of function template specialization 'negative_template_test<void>' requested here}}
  constexpr auto inference_fail = __builtin_infer_alloc_token(123); // expected-error {{must be initialized by a constant expression}} \
                                                                    // expected-note {{could not infer allocation type for __builtin_infer_alloc_token}}

  // PR178892: Ensure struct arguments don't crash the bytecode interpreter.
  struct S {};
  constexpr auto struct_arg = __builtin_infer_alloc_token(S()); // expected-error {{must be initialized by a constant expression}} \
                                                                // expected-note {{could not infer allocation type for __builtin_infer_alloc_token}}
}
