// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_LOGICENGINE_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_LOGICENGINE_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

#include "ApiObjectsGen.h"
#include "LinkGen.h"
#include "LuaScriptGen.h"
#include "PropertyGen.h"
#include "RamsesAppearanceBindingGen.h"
#include "RamsesBindingGen.h"
#include "RamsesCameraBindingGen.h"
#include "RamsesNodeBindingGen.h"
#include "RamsesReferenceGen.h"

namespace rlogic_serialization {

struct Version;
struct VersionBuilder;

struct LogicEngine;
struct LogicEngineBuilder;

struct Version FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef VersionBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_V_MAJOR = 4,
    VT_V_MINOR = 6,
    VT_V_PATCH = 8,
    VT_V_STRING = 10,
    VT_V_FILEFORMATVERSION = 12
  };
  uint32_t v_major() const {
    return GetField<uint32_t>(VT_V_MAJOR, 0);
  }
  uint32_t v_minor() const {
    return GetField<uint32_t>(VT_V_MINOR, 0);
  }
  uint32_t v_patch() const {
    return GetField<uint32_t>(VT_V_PATCH, 0);
  }
  const flatbuffers::String *v_string() const {
    return GetPointer<const flatbuffers::String *>(VT_V_STRING);
  }
  uint32_t v_fileFormatVersion() const {
    return GetField<uint32_t>(VT_V_FILEFORMATVERSION, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_V_MAJOR) &&
           VerifyField<uint32_t>(verifier, VT_V_MINOR) &&
           VerifyField<uint32_t>(verifier, VT_V_PATCH) &&
           VerifyOffset(verifier, VT_V_STRING) &&
           verifier.VerifyString(v_string()) &&
           VerifyField<uint32_t>(verifier, VT_V_FILEFORMATVERSION) &&
           verifier.EndTable();
  }
};

struct VersionBuilder {
  typedef Version Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_v_major(uint32_t v_major) {
    fbb_.AddElement<uint32_t>(Version::VT_V_MAJOR, v_major, 0);
  }
  void add_v_minor(uint32_t v_minor) {
    fbb_.AddElement<uint32_t>(Version::VT_V_MINOR, v_minor, 0);
  }
  void add_v_patch(uint32_t v_patch) {
    fbb_.AddElement<uint32_t>(Version::VT_V_PATCH, v_patch, 0);
  }
  void add_v_string(flatbuffers::Offset<flatbuffers::String> v_string) {
    fbb_.AddOffset(Version::VT_V_STRING, v_string);
  }
  void add_v_fileFormatVersion(uint32_t v_fileFormatVersion) {
    fbb_.AddElement<uint32_t>(Version::VT_V_FILEFORMATVERSION, v_fileFormatVersion, 0);
  }
  explicit VersionBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  VersionBuilder &operator=(const VersionBuilder &);
  flatbuffers::Offset<Version> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Version>(end);
    return o;
  }
};

inline flatbuffers::Offset<Version> CreateVersion(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t v_major = 0,
    uint32_t v_minor = 0,
    uint32_t v_patch = 0,
    flatbuffers::Offset<flatbuffers::String> v_string = 0,
    uint32_t v_fileFormatVersion = 0) {
  VersionBuilder builder_(_fbb);
  builder_.add_v_fileFormatVersion(v_fileFormatVersion);
  builder_.add_v_string(v_string);
  builder_.add_v_patch(v_patch);
  builder_.add_v_minor(v_minor);
  builder_.add_v_major(v_major);
  return builder_.Finish();
}

struct Version::Traits {
  using type = Version;
  static auto constexpr Create = CreateVersion;
};

inline flatbuffers::Offset<Version> CreateVersionDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint32_t v_major = 0,
    uint32_t v_minor = 0,
    uint32_t v_patch = 0,
    const char *v_string = nullptr,
    uint32_t v_fileFormatVersion = 0) {
  auto v_string__ = v_string ? _fbb.CreateString(v_string) : 0;
  return rlogic_serialization::CreateVersion(
      _fbb,
      v_major,
      v_minor,
      v_patch,
      v_string__,
      v_fileFormatVersion);
}

struct LogicEngine FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef LogicEngineBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RAMSESVERSION = 4,
    VT_RLOGICVERSION = 6,
    VT_APIOBJECTS = 8
  };
  const rlogic_serialization::Version *ramsesVersion() const {
    return GetPointer<const rlogic_serialization::Version *>(VT_RAMSESVERSION);
  }
  const rlogic_serialization::Version *rlogicVersion() const {
    return GetPointer<const rlogic_serialization::Version *>(VT_RLOGICVERSION);
  }
  const rlogic_serialization::ApiObjects *apiObjects() const {
    return GetPointer<const rlogic_serialization::ApiObjects *>(VT_APIOBJECTS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_RAMSESVERSION) &&
           verifier.VerifyTable(ramsesVersion()) &&
           VerifyOffsetRequired(verifier, VT_RLOGICVERSION) &&
           verifier.VerifyTable(rlogicVersion()) &&
           VerifyOffset(verifier, VT_APIOBJECTS) &&
           verifier.VerifyTable(apiObjects()) &&
           verifier.EndTable();
  }
};

struct LogicEngineBuilder {
  typedef LogicEngine Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_ramsesVersion(flatbuffers::Offset<rlogic_serialization::Version> ramsesVersion) {
    fbb_.AddOffset(LogicEngine::VT_RAMSESVERSION, ramsesVersion);
  }
  void add_rlogicVersion(flatbuffers::Offset<rlogic_serialization::Version> rlogicVersion) {
    fbb_.AddOffset(LogicEngine::VT_RLOGICVERSION, rlogicVersion);
  }
  void add_apiObjects(flatbuffers::Offset<rlogic_serialization::ApiObjects> apiObjects) {
    fbb_.AddOffset(LogicEngine::VT_APIOBJECTS, apiObjects);
  }
  explicit LogicEngineBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  LogicEngineBuilder &operator=(const LogicEngineBuilder &);
  flatbuffers::Offset<LogicEngine> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<LogicEngine>(end);
    fbb_.Required(o, LogicEngine::VT_RAMSESVERSION);
    fbb_.Required(o, LogicEngine::VT_RLOGICVERSION);
    return o;
  }
};

inline flatbuffers::Offset<LogicEngine> CreateLogicEngine(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<rlogic_serialization::Version> ramsesVersion = 0,
    flatbuffers::Offset<rlogic_serialization::Version> rlogicVersion = 0,
    flatbuffers::Offset<rlogic_serialization::ApiObjects> apiObjects = 0) {
  LogicEngineBuilder builder_(_fbb);
  builder_.add_apiObjects(apiObjects);
  builder_.add_rlogicVersion(rlogicVersion);
  builder_.add_ramsesVersion(ramsesVersion);
  return builder_.Finish();
}

struct LogicEngine::Traits {
  using type = LogicEngine;
  static auto constexpr Create = CreateLogicEngine;
};

inline const rlogic_serialization::LogicEngine *GetLogicEngine(const void *buf) {
  return flatbuffers::GetRoot<rlogic_serialization::LogicEngine>(buf);
}

inline const rlogic_serialization::LogicEngine *GetSizePrefixedLogicEngine(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<rlogic_serialization::LogicEngine>(buf);
}

inline bool VerifyLogicEngineBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<rlogic_serialization::LogicEngine>(nullptr);
}

inline bool VerifySizePrefixedLogicEngineBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<rlogic_serialization::LogicEngine>(nullptr);
}

inline void FinishLogicEngineBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<rlogic_serialization::LogicEngine> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedLogicEngineBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<rlogic_serialization::LogicEngine> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_LOGICENGINE_RLOGIC_SERIALIZATION_H_
