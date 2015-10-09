//===--- IterativeTypeChecker.h - Iterative Type Checker --------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file defines the IterativeTypeChecker class, which performs
//  iterative type checking by tracking the set of outstanding
//  type-checking requests and servicing them as needed.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SEMA_ITERATIVE_TYPE_CHECKER_H
#define SWIFT_SEMA_ITERATIVE_TYPE_CHECKER_H

#include "swift/Sema/TypeCheckRequest.h"
#include "swift/AST/DiagnosticEngine.h"
#include "swift/Basic/LLVM.h"
#include "llvm/ADT/STLExtras.h"

namespace swift {

class ASTContext;
class TypeChecker;

/// An iterative type checker that processes type check requests to
/// ensure that the AST has the information needed by the client.
class IterativeTypeChecker {
  /// The underyling (non-iterative) type checker on which this iterative
  /// type checker depends.
  ///
  /// Each dependency on the non-iterative type checker potentially
  /// introduces recursion and should be eliminated.
  TypeChecker &TC;

  // Declare the is<request kind>Satisfied predicates,
  // enumerateDependenciesOf<request kind> functions, and
  // satisfy<request kind> functions.
#define TYPE_CHECK_REQUEST(Request,PayloadName)                         \
  bool is##Request##Satisfied(                                          \
         TypeCheckRequest::PayloadName##PayloadType payload);           \
  void process##Request(                                                \
         TypeCheckRequest::PayloadName##PayloadType payload,            \
         UnsatisfiedDependency unsatisfiedDependency);

#include "swift/Sema/TypeCheckRequestKinds.def"

  void process(TypeCheckRequest request,
               UnsatisfiedDependency unsatisfiedDependency);

public:
  IterativeTypeChecker(TypeChecker &tc) : TC(tc) { }

  ASTContext &getASTContext() const;

  DiagnosticEngine &getDiags() const;

  template<typename ...ArgTypes>
  InFlightDiagnostic diagnose(ArgTypes &&...Args) {
    return getDiags().diagnose(std::forward<ArgTypes>(Args)...);
  }

  /// Determine whether the given request has already been satisfied.
  bool isSatisfied(TypeCheckRequest request);

  /// Satisfy the given request.
  void satisfy(TypeCheckRequest request);
};

}

#endif /* SWIFT_SEMA_ITERATIVE_TYPE_CHECKER_H */
