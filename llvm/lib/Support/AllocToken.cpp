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

  StringRef Name = Metadata.TypeName;
  SmallString<128> TypeAndFunctionName;
  if (Mode == AllocTokenMode::TypeFuncHash ||
      Mode == AllocTokenMode::TypeFuncHashPointerSplit) {
    if (!Metadata.FunctionName)
      return std::nullopt;
    TypeAndFunctionName = Name;
    TypeAndFunctionName.push_back(':');
    TypeAndFunctionName.append(*Metadata.FunctionName);
    Name = TypeAndFunctionName;
  }

  switch (Mode) {
  case AllocTokenMode::Increment:
  case AllocTokenMode::Random:
    // Stateful modes cannot be implemented as a pure function.
    return std::nullopt;

  case AllocTokenMode::TypeHash:
  case AllocTokenMode::TypeFuncHash:
    return getStableSipHash(Name) % MaxTokens;

  case AllocTokenMode::TypeHashPointerSplit:
  case AllocTokenMode::TypeFuncHashPointerSplit: {
    if (MaxTokens == 1)
      return 0;
    const uint64_t HalfTokens = MaxTokens / 2;
    uint64_t Hash = getStableSipHash(Name) % HalfTokens;
    if (Metadata.ContainsPointer)
      Hash += HalfTokens;
    return Hash;
  }
  }

  llvm_unreachable("");
}
