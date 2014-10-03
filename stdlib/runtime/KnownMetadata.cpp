//===--- KnownMetadata.cpp - Swift Language ABI Known Metadata Objects ----===//
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
// Definitions of some builtin metadata objects.
//
//===----------------------------------------------------------------------===//

#include "swift/Runtime/Metadata.h"
#include "swift/Runtime/HeapObject.h"
#include "MetadataImpl.h"
#include "Private.h"
#include <cstring>
#include <climits>

using namespace swift;
using namespace metadataimpl;

/// Copy a value from one object to another based on the size in the
/// given type metadata.
OpaqueValue *swift::swift_copyPOD(OpaqueValue *dest, OpaqueValue *src,
                                  const Metadata *type) {
  return (OpaqueValue*) memcpy(dest, src, type->getValueWitnesses()->size);
}

/// A function which does a naive copy.
template <class T> static T *copy(T *dest, T *src, const Metadata *self) {
  *dest = *src;
  return dest;
}

#define POD_VALUE_WITNESS_TABLE(TYPE, SIZE) { \
  (value_witness_types::destroyBuffer*) &doNothing,                     \
  (value_witness_types::initializeBufferWithCopyOfBuffer*) &copy<TYPE>, \
  (value_witness_types::projectBuffer*) &projectBuffer,                 \
  (value_witness_types::deallocateBuffer*) &doNothing,                  \
  (value_witness_types::destroy*) &doNothing,                           \
  (value_witness_types::initializeBufferWithCopy*) &copy<TYPE>,         \
  (value_witness_types::initializeWithCopy*) &copy<TYPE>,               \
  (value_witness_types::assignWithCopy*) &copy<TYPE>,                   \
  (value_witness_types::initializeBufferWithTake*) &copy<TYPE>,         \
  (value_witness_types::initializeWithTake*) &copy<TYPE>,               \
  (value_witness_types::assignWithTake*) &copy<TYPE>,                   \
  (value_witness_types::allocateBuffer*) &projectBuffer,                \
  (value_witness_types::initializeBufferWithTakeOfBuffer*) &copy<TYPE>, \
  (value_witness_types::size) (SIZE),                                   \
  ValueWitnessFlags().withAlignment(SIZE).withPOD(true)                 \
                     .withInlineStorage(true),                          \
  (value_witness_types::stride) (SIZE)                                  \
}

namespace {
  // A type sized and aligned the way Swift wants Int128 (and Float80/Float128)
  // to be sized and aligned.
  struct alignas(16) int128_like {
    char data[16];
  };
}

// We use explicit sizes and alignments here just in case the C ABI
// under-aligns any or all of them.
const ValueWitnessTable swift::_TWVBi8_ =
  ValueWitnessTableForBox<NativeBox<uint8_t, 1>>::table;
const ValueWitnessTable swift::_TWVBi16_ =
  ValueWitnessTableForBox<NativeBox<uint16_t, 2>>::table;
const ValueWitnessTable swift::_TWVBi32_ =
  ValueWitnessTableForBox<NativeBox<uint32_t, 4>>::table;
const ValueWitnessTable swift::_TWVBi64_ =
  ValueWitnessTableForBox<NativeBox<uint64_t, 8>>::table;
const ValueWitnessTable swift::_TWVBi128_ =
  ValueWitnessTableForBox<NativeBox<int128_like, 16>>::table;

/// Store an invalid pointer value as an extra inhabitant of a heap object.
void swift::swift_storeHeapObjectExtraInhabitant(HeapObject **dest,
                                                 int index) {
  using namespace heap_object_abi;
  
  // This must be consistent with the storeHeapObjectExtraInhabitant
  // implementation in IRGen's GenType.cpp.
  
  // FIXME: We could use high spare bits to produce extra inhabitants, but we
  // probably won't need to.
  *dest = (HeapObject*)((uintptr_t)index << ObjCReservedLowBits);
}

/// Return the extra inhabitant index for an invalid pointer value, or -1 if
/// the pointer is valid.
int swift::swift_getHeapObjectExtraInhabitantIndex(HeapObject * const* src) {
  using namespace heap_object_abi;

  // This must be consistent with the getHeapObjectExtraInhabitant
  // implementation in IRGen's GenType.cpp.

  uintptr_t val = (uintptr_t)*src;

  // Return -1 for valid pointers.
  // FIXME: We could use high spare bits to produce extra inhabitants, but we
  // probably won't need to.
  if (val >= LeastValidPointerValue)
    return -1;

#if SWIFT_OBJC_INTEROP
  // Return -1 for ObjC tagged pointers.
  // FIXME: This check is unnecessary for known-Swift types.
  if (isObjCTaggedPointer((const void*) val))
    return -1;
#endif

  return (int)(val >> ObjCReservedLowBits);
}


/// The basic value-witness table for Swift object pointers.
const ExtraInhabitantsValueWitnessTable swift::_TWVBo =
  ValueWitnessTableForBox<SwiftRetainableBox>::table;

/// The value-witness table for pointer-aligned unmanaged pointer types.
const ExtraInhabitantsValueWitnessTable swift::_TWVMBo =
  ValueWitnessTableForBox<PointerPointerBox>::table;

#if SWIFT_OBJC_INTEROP
/*** Objective-C pointers ****************************************************/

// This section can reasonably be suppressed in builds that don't
// need to support Objective-C.

/// The basic value-witness table for ObjC object pointers.
const ExtraInhabitantsValueWitnessTable swift::_TWVBO =
  ValueWitnessTableForBox<ObjCRetainableBox>::table;
#endif

/*** Functions ***************************************************************/

/// The basic value-witness table for function types.
const ValueWitnessTable swift::_TWVFT_T_ =
  ValueWitnessTableForBox<AggregateBox<NativeBox<void*>,
                                       SwiftRetainableBox>>::table;

/*** Empty tuples ************************************************************/

/// The basic value-witness table for empty types.
const ValueWitnessTable swift::_TWVT_ =
  ValueWitnessTableForBox<AggregateBox<>>::table;

/*** Known metadata **********************************************************/

// Define some builtin opaque metadata.
#define OPAQUE_METADATA(TYPE) \
  const FullOpaqueMetadata swift::_TMd##TYPE = { \
    { &_TWV##TYPE },                             \
    { { MetadataKind::Opaque } }                 \
  };
OPAQUE_METADATA(Bi8_)
OPAQUE_METADATA(Bi16_)
OPAQUE_METADATA(Bi32_)
OPAQUE_METADATA(Bi64_)
OPAQUE_METADATA(Bi128_)
OPAQUE_METADATA(Bo)
#if SWIFT_OBJC_INTEROP
OPAQUE_METADATA(BO)
#endif

/// The standard metadata for the empty tuple.
const FullMetadata<TupleTypeMetadata> swift::_TMdT_ = {
  { &_TWVT_ },                 // ValueWitnesses
  {
    { MetadataKind::Tuple },   // Kind
    0,                         // NumElements
    nullptr                    // Labels
  }
};
