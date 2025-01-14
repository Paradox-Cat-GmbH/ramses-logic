// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PROPERTY_RLOGIC_SERIALIZATION_H_
#define FLATBUFFERS_GENERATED_PROPERTY_RLOGIC_SERIALIZATION_H_

#include "flatbuffers/flatbuffers.h"

namespace rlogic_serialization {

struct float_s;

struct vec2f_s;

struct vec3f_s;

struct vec4f_s;

struct int32_s;

struct int64_s;

struct vec2i_s;

struct vec3i_s;

struct vec4i_s;

struct bool_s;

struct string_s;
struct string_sBuilder;

struct Property;
struct PropertyBuilder;

enum class EPropertyRootType : uint8_t {
  Primitive = 0,
  Struct = 1,
  Array = 2,
  MIN = Primitive,
  MAX = Array
};

inline const EPropertyRootType (&EnumValuesEPropertyRootType())[3] {
  static const EPropertyRootType values[] = {
    EPropertyRootType::Primitive,
    EPropertyRootType::Struct,
    EPropertyRootType::Array
  };
  return values;
}

inline const char * const *EnumNamesEPropertyRootType() {
  static const char * const names[4] = {
    "Primitive",
    "Struct",
    "Array",
    nullptr
  };
  return names;
}

inline const char *EnumNameEPropertyRootType(EPropertyRootType e) {
  if (flatbuffers::IsOutRange(e, EPropertyRootType::Primitive, EPropertyRootType::Array)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesEPropertyRootType()[index];
}

enum class PropertyValue : uint8_t {
  NONE = 0,
  float_s = 1,
  vec2f_s = 2,
  vec3f_s = 3,
  vec4f_s = 4,
  int32_s = 5,
  int64_s = 6,
  vec2i_s = 7,
  vec3i_s = 8,
  vec4i_s = 9,
  string_s = 10,
  bool_s = 11,
  MIN = NONE,
  MAX = bool_s
};

inline const PropertyValue (&EnumValuesPropertyValue())[12] {
  static const PropertyValue values[] = {
    PropertyValue::NONE,
    PropertyValue::float_s,
    PropertyValue::vec2f_s,
    PropertyValue::vec3f_s,
    PropertyValue::vec4f_s,
    PropertyValue::int32_s,
    PropertyValue::int64_s,
    PropertyValue::vec2i_s,
    PropertyValue::vec3i_s,
    PropertyValue::vec4i_s,
    PropertyValue::string_s,
    PropertyValue::bool_s
  };
  return values;
}

inline const char * const *EnumNamesPropertyValue() {
  static const char * const names[13] = {
    "NONE",
    "float_s",
    "vec2f_s",
    "vec3f_s",
    "vec4f_s",
    "int32_s",
    "int64_s",
    "vec2i_s",
    "vec3i_s",
    "vec4i_s",
    "string_s",
    "bool_s",
    nullptr
  };
  return names;
}

inline const char *EnumNamePropertyValue(PropertyValue e) {
  if (flatbuffers::IsOutRange(e, PropertyValue::NONE, PropertyValue::bool_s)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesPropertyValue()[index];
}

template<typename T> struct PropertyValueTraits {
  static const PropertyValue enum_value = PropertyValue::NONE;
};

template<> struct PropertyValueTraits<rlogic_serialization::float_s> {
  static const PropertyValue enum_value = PropertyValue::float_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec2f_s> {
  static const PropertyValue enum_value = PropertyValue::vec2f_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec3f_s> {
  static const PropertyValue enum_value = PropertyValue::vec3f_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec4f_s> {
  static const PropertyValue enum_value = PropertyValue::vec4f_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::int32_s> {
  static const PropertyValue enum_value = PropertyValue::int32_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::int64_s> {
  static const PropertyValue enum_value = PropertyValue::int64_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec2i_s> {
  static const PropertyValue enum_value = PropertyValue::vec2i_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec3i_s> {
  static const PropertyValue enum_value = PropertyValue::vec3i_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::vec4i_s> {
  static const PropertyValue enum_value = PropertyValue::vec4i_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::string_s> {
  static const PropertyValue enum_value = PropertyValue::string_s;
};

template<> struct PropertyValueTraits<rlogic_serialization::bool_s> {
  static const PropertyValue enum_value = PropertyValue::bool_s;
};

bool VerifyPropertyValue(flatbuffers::Verifier &verifier, const void *obj, PropertyValue type);
bool VerifyPropertyValueVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) float_s FLATBUFFERS_FINAL_CLASS {
 private:
  float v_;

 public:
  float_s() {
    memset(static_cast<void *>(this), 0, sizeof(float_s));
  }
  float_s(float _v)
      : v_(flatbuffers::EndianScalar(_v)) {
  }
  float v() const {
    return flatbuffers::EndianScalar(v_);
  }
};
FLATBUFFERS_STRUCT_END(float_s, 4);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec2f_s FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;

 public:
  vec2f_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec2f_s));
  }
  vec2f_s(float _x, float _y)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)) {
  }
  float x() const {
    return flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return flatbuffers::EndianScalar(y_);
  }
};
FLATBUFFERS_STRUCT_END(vec2f_s, 8);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec3f_s FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  vec3f_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec3f_s));
  }
  vec3f_s(float _x, float _y, float _z)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        z_(flatbuffers::EndianScalar(_z)) {
  }
  float x() const {
    return flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return flatbuffers::EndianScalar(z_);
  }
};
FLATBUFFERS_STRUCT_END(vec3f_s, 12);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec4f_s FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;
  float w_;

 public:
  vec4f_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec4f_s));
  }
  vec4f_s(float _x, float _y, float _z, float _w)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        z_(flatbuffers::EndianScalar(_z)),
        w_(flatbuffers::EndianScalar(_w)) {
  }
  float x() const {
    return flatbuffers::EndianScalar(x_);
  }
  float y() const {
    return flatbuffers::EndianScalar(y_);
  }
  float z() const {
    return flatbuffers::EndianScalar(z_);
  }
  float w() const {
    return flatbuffers::EndianScalar(w_);
  }
};
FLATBUFFERS_STRUCT_END(vec4f_s, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) int32_s FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t v_;

 public:
  int32_s() {
    memset(static_cast<void *>(this), 0, sizeof(int32_s));
  }
  int32_s(int32_t _v)
      : v_(flatbuffers::EndianScalar(_v)) {
  }
  int32_t v() const {
    return flatbuffers::EndianScalar(v_);
  }
};
FLATBUFFERS_STRUCT_END(int32_s, 4);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(8) int64_s FLATBUFFERS_FINAL_CLASS {
 private:
  int64_t v_;

 public:
  int64_s() {
    memset(static_cast<void *>(this), 0, sizeof(int64_s));
  }
  int64_s(int64_t _v)
      : v_(flatbuffers::EndianScalar(_v)) {
  }
  int64_t v() const {
    return flatbuffers::EndianScalar(v_);
  }
};
FLATBUFFERS_STRUCT_END(int64_s, 8);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec2i_s FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t x_;
  int32_t y_;

 public:
  vec2i_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec2i_s));
  }
  vec2i_s(int32_t _x, int32_t _y)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)) {
  }
  int32_t x() const {
    return flatbuffers::EndianScalar(x_);
  }
  int32_t y() const {
    return flatbuffers::EndianScalar(y_);
  }
};
FLATBUFFERS_STRUCT_END(vec2i_s, 8);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec3i_s FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t x_;
  int32_t y_;
  int32_t z_;

 public:
  vec3i_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec3i_s));
  }
  vec3i_s(int32_t _x, int32_t _y, int32_t _z)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        z_(flatbuffers::EndianScalar(_z)) {
  }
  int32_t x() const {
    return flatbuffers::EndianScalar(x_);
  }
  int32_t y() const {
    return flatbuffers::EndianScalar(y_);
  }
  int32_t z() const {
    return flatbuffers::EndianScalar(z_);
  }
};
FLATBUFFERS_STRUCT_END(vec3i_s, 12);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(4) vec4i_s FLATBUFFERS_FINAL_CLASS {
 private:
  int32_t x_;
  int32_t y_;
  int32_t z_;
  int32_t w_;

 public:
  vec4i_s() {
    memset(static_cast<void *>(this), 0, sizeof(vec4i_s));
  }
  vec4i_s(int32_t _x, int32_t _y, int32_t _z, int32_t _w)
      : x_(flatbuffers::EndianScalar(_x)),
        y_(flatbuffers::EndianScalar(_y)),
        z_(flatbuffers::EndianScalar(_z)),
        w_(flatbuffers::EndianScalar(_w)) {
  }
  int32_t x() const {
    return flatbuffers::EndianScalar(x_);
  }
  int32_t y() const {
    return flatbuffers::EndianScalar(y_);
  }
  int32_t z() const {
    return flatbuffers::EndianScalar(z_);
  }
  int32_t w() const {
    return flatbuffers::EndianScalar(w_);
  }
};
FLATBUFFERS_STRUCT_END(vec4i_s, 16);

FLATBUFFERS_MANUALLY_ALIGNED_STRUCT(1) bool_s FLATBUFFERS_FINAL_CLASS {
 private:
  uint8_t v_;

 public:
  bool_s() {
    memset(static_cast<void *>(this), 0, sizeof(bool_s));
  }
  bool_s(bool _v)
      : v_(flatbuffers::EndianScalar(static_cast<uint8_t>(_v))) {
  }
  bool v() const {
    return flatbuffers::EndianScalar(v_) != 0;
  }
};
FLATBUFFERS_STRUCT_END(bool_s, 1);

struct string_s FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef string_sBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_V = 4
  };
  const flatbuffers::String *v() const {
    return GetPointer<const flatbuffers::String *>(VT_V);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_V) &&
           verifier.VerifyString(v()) &&
           verifier.EndTable();
  }
};

struct string_sBuilder {
  typedef string_s Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_v(flatbuffers::Offset<flatbuffers::String> v) {
    fbb_.AddOffset(string_s::VT_V, v);
  }
  explicit string_sBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  string_sBuilder &operator=(const string_sBuilder &);
  flatbuffers::Offset<string_s> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<string_s>(end);
    return o;
  }
};

inline flatbuffers::Offset<string_s> Createstring_s(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> v = 0) {
  string_sBuilder builder_(_fbb);
  builder_.add_v(v);
  return builder_.Finish();
}

struct string_s::Traits {
  using type = string_s;
  static auto constexpr Create = Createstring_s;
};

inline flatbuffers::Offset<string_s> Createstring_sDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *v = nullptr) {
  auto v__ = v ? _fbb.CreateString(v) : 0;
  return rlogic_serialization::Createstring_s(
      _fbb,
      v__);
}

struct Property FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef PropertyBuilder Builder;
  struct Traits;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_NAME = 4,
    VT_ROOTTYPE = 6,
    VT_CHILDREN = 8,
    VT_VALUE_TYPE = 10,
    VT_VALUE = 12
  };
  const flatbuffers::String *name() const {
    return GetPointer<const flatbuffers::String *>(VT_NAME);
  }
  rlogic_serialization::EPropertyRootType rootType() const {
    return static_cast<rlogic_serialization::EPropertyRootType>(GetField<uint8_t>(VT_ROOTTYPE, 0));
  }
  const flatbuffers::Vector<flatbuffers::Offset<rlogic_serialization::Property>> *children() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<rlogic_serialization::Property>> *>(VT_CHILDREN);
  }
  rlogic_serialization::PropertyValue value_type() const {
    return static_cast<rlogic_serialization::PropertyValue>(GetField<uint8_t>(VT_VALUE_TYPE, 0));
  }
  const void *value() const {
    return GetPointer<const void *>(VT_VALUE);
  }
  template<typename T> const T *value_as() const;
  const rlogic_serialization::float_s *value_as_float_s() const {
    return value_type() == rlogic_serialization::PropertyValue::float_s ? static_cast<const rlogic_serialization::float_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec2f_s *value_as_vec2f_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec2f_s ? static_cast<const rlogic_serialization::vec2f_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec3f_s *value_as_vec3f_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec3f_s ? static_cast<const rlogic_serialization::vec3f_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec4f_s *value_as_vec4f_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec4f_s ? static_cast<const rlogic_serialization::vec4f_s *>(value()) : nullptr;
  }
  const rlogic_serialization::int32_s *value_as_int32_s() const {
    return value_type() == rlogic_serialization::PropertyValue::int32_s ? static_cast<const rlogic_serialization::int32_s *>(value()) : nullptr;
  }
  const rlogic_serialization::int64_s *value_as_int64_s() const {
    return value_type() == rlogic_serialization::PropertyValue::int64_s ? static_cast<const rlogic_serialization::int64_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec2i_s *value_as_vec2i_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec2i_s ? static_cast<const rlogic_serialization::vec2i_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec3i_s *value_as_vec3i_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec3i_s ? static_cast<const rlogic_serialization::vec3i_s *>(value()) : nullptr;
  }
  const rlogic_serialization::vec4i_s *value_as_vec4i_s() const {
    return value_type() == rlogic_serialization::PropertyValue::vec4i_s ? static_cast<const rlogic_serialization::vec4i_s *>(value()) : nullptr;
  }
  const rlogic_serialization::string_s *value_as_string_s() const {
    return value_type() == rlogic_serialization::PropertyValue::string_s ? static_cast<const rlogic_serialization::string_s *>(value()) : nullptr;
  }
  const rlogic_serialization::bool_s *value_as_bool_s() const {
    return value_type() == rlogic_serialization::PropertyValue::bool_s ? static_cast<const rlogic_serialization::bool_s *>(value()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_NAME) &&
           verifier.VerifyString(name()) &&
           VerifyField<uint8_t>(verifier, VT_ROOTTYPE) &&
           VerifyOffset(verifier, VT_CHILDREN) &&
           verifier.VerifyVector(children()) &&
           verifier.VerifyVectorOfTables(children()) &&
           VerifyField<uint8_t>(verifier, VT_VALUE_TYPE) &&
           VerifyOffset(verifier, VT_VALUE) &&
           VerifyPropertyValue(verifier, value(), value_type()) &&
           verifier.EndTable();
  }
};

template<> inline const rlogic_serialization::float_s *Property::value_as<rlogic_serialization::float_s>() const {
  return value_as_float_s();
}

template<> inline const rlogic_serialization::vec2f_s *Property::value_as<rlogic_serialization::vec2f_s>() const {
  return value_as_vec2f_s();
}

template<> inline const rlogic_serialization::vec3f_s *Property::value_as<rlogic_serialization::vec3f_s>() const {
  return value_as_vec3f_s();
}

template<> inline const rlogic_serialization::vec4f_s *Property::value_as<rlogic_serialization::vec4f_s>() const {
  return value_as_vec4f_s();
}

template<> inline const rlogic_serialization::int32_s *Property::value_as<rlogic_serialization::int32_s>() const {
  return value_as_int32_s();
}

template<> inline const rlogic_serialization::int64_s *Property::value_as<rlogic_serialization::int64_s>() const {
  return value_as_int64_s();
}

template<> inline const rlogic_serialization::vec2i_s *Property::value_as<rlogic_serialization::vec2i_s>() const {
  return value_as_vec2i_s();
}

template<> inline const rlogic_serialization::vec3i_s *Property::value_as<rlogic_serialization::vec3i_s>() const {
  return value_as_vec3i_s();
}

template<> inline const rlogic_serialization::vec4i_s *Property::value_as<rlogic_serialization::vec4i_s>() const {
  return value_as_vec4i_s();
}

template<> inline const rlogic_serialization::string_s *Property::value_as<rlogic_serialization::string_s>() const {
  return value_as_string_s();
}

template<> inline const rlogic_serialization::bool_s *Property::value_as<rlogic_serialization::bool_s>() const {
  return value_as_bool_s();
}

struct PropertyBuilder {
  typedef Property Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) {
    fbb_.AddOffset(Property::VT_NAME, name);
  }
  void add_rootType(rlogic_serialization::EPropertyRootType rootType) {
    fbb_.AddElement<uint8_t>(Property::VT_ROOTTYPE, static_cast<uint8_t>(rootType), 0);
  }
  void add_children(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<rlogic_serialization::Property>>> children) {
    fbb_.AddOffset(Property::VT_CHILDREN, children);
  }
  void add_value_type(rlogic_serialization::PropertyValue value_type) {
    fbb_.AddElement<uint8_t>(Property::VT_VALUE_TYPE, static_cast<uint8_t>(value_type), 0);
  }
  void add_value(flatbuffers::Offset<void> value) {
    fbb_.AddOffset(Property::VT_VALUE, value);
  }
  explicit PropertyBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  PropertyBuilder &operator=(const PropertyBuilder &);
  flatbuffers::Offset<Property> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Property>(end);
    return o;
  }
};

inline flatbuffers::Offset<Property> CreateProperty(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    rlogic_serialization::EPropertyRootType rootType = rlogic_serialization::EPropertyRootType::Primitive,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<rlogic_serialization::Property>>> children = 0,
    rlogic_serialization::PropertyValue value_type = rlogic_serialization::PropertyValue::NONE,
    flatbuffers::Offset<void> value = 0) {
  PropertyBuilder builder_(_fbb);
  builder_.add_value(value);
  builder_.add_children(children);
  builder_.add_name(name);
  builder_.add_value_type(value_type);
  builder_.add_rootType(rootType);
  return builder_.Finish();
}

struct Property::Traits {
  using type = Property;
  static auto constexpr Create = CreateProperty;
};

inline flatbuffers::Offset<Property> CreatePropertyDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    rlogic_serialization::EPropertyRootType rootType = rlogic_serialization::EPropertyRootType::Primitive,
    const std::vector<flatbuffers::Offset<rlogic_serialization::Property>> *children = nullptr,
    rlogic_serialization::PropertyValue value_type = rlogic_serialization::PropertyValue::NONE,
    flatbuffers::Offset<void> value = 0) {
  auto name__ = name ? _fbb.CreateString(name) : 0;
  auto children__ = children ? _fbb.CreateVector<flatbuffers::Offset<rlogic_serialization::Property>>(*children) : 0;
  return rlogic_serialization::CreateProperty(
      _fbb,
      name__,
      rootType,
      children__,
      value_type,
      value);
}

inline bool VerifyPropertyValue(flatbuffers::Verifier &verifier, const void *obj, PropertyValue type) {
  switch (type) {
    case PropertyValue::NONE: {
      return true;
    }
    case PropertyValue::float_s: {
      return verifier.Verify<rlogic_serialization::float_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec2f_s: {
      return verifier.Verify<rlogic_serialization::vec2f_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec3f_s: {
      return verifier.Verify<rlogic_serialization::vec3f_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec4f_s: {
      return verifier.Verify<rlogic_serialization::vec4f_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::int32_s: {
      return verifier.Verify<rlogic_serialization::int32_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::int64_s: {
      return verifier.Verify<rlogic_serialization::int64_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec2i_s: {
      return verifier.Verify<rlogic_serialization::vec2i_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec3i_s: {
      return verifier.Verify<rlogic_serialization::vec3i_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::vec4i_s: {
      return verifier.Verify<rlogic_serialization::vec4i_s>(static_cast<const uint8_t *>(obj), 0);
    }
    case PropertyValue::string_s: {
      auto ptr = reinterpret_cast<const rlogic_serialization::string_s *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case PropertyValue::bool_s: {
      return verifier.Verify<rlogic_serialization::bool_s>(static_cast<const uint8_t *>(obj), 0);
    }
    default: return true;
  }
}

inline bool VerifyPropertyValueVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyPropertyValue(
        verifier,  values->Get(i), types->GetEnum<PropertyValue>(i))) {
      return false;
    }
  }
  return true;
}

}  // namespace rlogic_serialization

#endif  // FLATBUFFERS_GENERATED_PROPERTY_RLOGIC_SERIALIZATION_H_
