// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_RAMSESNODEBINDING_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_RAMSESNODEBINDING_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

#include "PropertyGen.h"
#include "RamsesBindingGen.h"
#include "RamsesReferenceGen.h"

namespace rlogic_serialization {

struct RamsesNodeBinding;
struct RamsesNodeBindingBuilder;

struct RamsesNodeBinding FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef RamsesNodeBindingBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_BASE = 4
  };
  const rlogic_serialization::RamsesBinding *base() const {
    return GetPointer<const rlogic_serialization::RamsesBinding *>(VT_BASE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_BASE) &&
           verifier.VerifyTable(base()) &&
           verifier.EndTable();
  }
};

struct RamsesNodeBindingBuilder {
  typedef RamsesNodeBinding Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_base(flatbuffers::Offset<rlogic_serialization::RamsesBinding> base) {
    fbb_.AddOffset(RamsesNodeBinding::VT_BASE, base);
  }
  explicit RamsesNodeBindingBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RamsesNodeBindingBuilder &operator=(const RamsesNodeBindingBuilder &);
  flatbuffers::Offset<RamsesNodeBinding> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RamsesNodeBinding>(end);
    return o;
  }
};

inline flatbuffers::Offset<RamsesNodeBinding> CreateRamsesNodeBinding(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<rlogic_serialization::RamsesBinding> base = 0) {
  RamsesNodeBindingBuilder builder_(_fbb);
  builder_.add_base(base);
  return builder_.Finish();
}

struct RamsesNodeBinding::Traits {
  using type = RamsesNodeBinding;
  static auto constexpr Create = CreateRamsesNodeBinding;
};

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_RAMSESNODEBINDING_RLOGIC_SERIALIZATION_H_
