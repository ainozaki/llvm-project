//===- AllocToken.cpp - Allocation Token Calculation ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Definition of AllocToken modes and shared calculation of stateless token IDs.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/AllocToken.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SipHash.h"

using namespace llvm;

std::optional<AllocTokenMode>
llvm::getAllocTokenModeFromString(StringRef Name) {
  return StringSwitch<std::optional<AllocTokenMode>>(Name)
      .Case("increment", AllocTokenMode::Increment)
      .Case("random", AllocTokenMode::Random)
      .Case("typehash", AllocTokenMode::TypeHash)
      .Case("typehashpointersplit", AllocTokenMode::TypeHashPointerSplit)
      .Case("typefunchash", AllocTokenMode::TypeFuncHash)
      .Case("typefunchashpointersplit",
            AllocTokenMode::TypeFuncHashPointerSplit)
      .Case("default", DefaultAllocTokenMode)
      .Default(std::nullopt);
}

StringRef llvm::getAllocTokenModeAsString(AllocTokenMode Mode) {
  switch (Mode) {
  case AllocTokenMode::Increment:
    return "increment";
  case AllocTokenMode::Random:
    return "random";
  case AllocTokenMode::TypeHash:
    return "typehash";
  case AllocTokenMode::TypeHashPointerSplit:
    return "typehashpointersplit";
  case AllocTokenMode::TypeFuncHash:
    return "typefunchash";
  case AllocTokenMode::TypeFuncHashPointerSplit:
    return "typefunchashpointersplit";
  }
  llvm_unreachable("Unknown AllocTokenMode");
}

std::optional<uint64_t> llvm::getAllocToken(AllocTokenMode Mode,
                                            const AllocTokenMetadata &Metadata,
                                            uint64_t MaxTokens) {
  assert(MaxTokens && "Must provide non-zero max tokens");

  switch (Mode) {
  case AllocTokenMode::Increment:
  case AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    return std::nullopt;

  case AllocTokenMode::TypeHash:
    return getStableSipHash(Metadata.TypeName) % MaxTokens;

  case AllocTokenMode::TypeHashPointerSplit: {
    if (MaxTokens == 1)
      return 0;
    const uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableSipHash(Metadata.TypeName) % HalfTokens;
    if (Metadata.ContainsPointer)
      Hash += HalfTokens;
    return Hash;
  }

  case AllocTokenMode::TypeFuncHash:
  case AllocTokenMode::TypeFuncHashPointerSplit: {
    if (!Metadata.FunctionName)
      return std::nullopt;

    if (MaxTokens < 8)
      return std::nullopt;

    unsigned MaxBits;
    if (MaxTokens == UINT64_MAX)
      MaxBits = 64;
    else if (MaxTokens == 0xFFFFFFFFULL)
      MaxBits = 32;
    else
      MaxBits = llvm::Log2_64(MaxTokens);

    if (MaxBits < 3)
      return std::nullopt;

    uint64_t TypeHash =
        Metadata.TypeName.empty() ? 0 : getStableSipHash(Metadata.TypeName);
    uint64_t FuncHash = getStableSipHash(*Metadata.FunctionName);

    unsigned FuncBits = MaxBits / 2;
    uint64_t FuncVal =
        (FuncBits == 64) ? FuncHash : (FuncHash % (1ULL << FuncBits));

    if (Mode == AllocTokenMode::TypeFuncHashPointerSplit) {
      unsigned TypeBits = (MaxBits - 1) - FuncBits;
      uint64_t TypeVal =
          (TypeBits == 64) ? TypeHash : (TypeHash % (1ULL << TypeBits));
      uint64_t PointerVal =
          Metadata.ContainsPointer ? (1ULL << (MaxBits - 1)) : 0;
      return PointerVal | (TypeVal << FuncBits) | FuncVal;
    }

    unsigned TypeBits = MaxBits - FuncBits;
    uint64_t TypeVal =
        (TypeBits == 64) ? TypeHash : (TypeHash % (1ULL << TypeBits));
    return (TypeVal << FuncBits) | FuncVal;
  }
  }

  llvm_unreachable("Unknown AllocTokenMode");
}
