#pragma once

#include "object_holder.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Ast {
class Statement;
}

class TestRunner;

namespace Runtime {

class Object {
 public:
  virtual ~Object() = default;
  virtual void Print(std::ostream &os) = 0;
  [[nodiscard]]virtual bool IsTrue() const = 0;
};

template<typename T>
class ValueObject : public Object {
 public:
  ValueObject(T v) : value(v) {}

  void Print(std::ostream &os) override {
    os << value;
  }

  const T &GetValue() const {
    return value;
  }

  virtual bool IsTrue() const override {
    return false;
  }

 private:
  T value;

  friend class Bool;
  friend class String;
  friend class Number;
};

class String : public ValueObject<std::string> {
  using ValueObject<std::string>::ValueObject;
 public:
  bool IsTrue() const override {
    return (!GetValue().empty());
  }
};

class Number : public ValueObject<int> {
  using ValueObject<int>::ValueObject;
 public:
  bool IsTrue() const override {
    return (GetValue() != 0);
  }
};

class Bool : public ValueObject<bool> {
 public:
  using ValueObject<bool>::ValueObject;
  void Print(std::ostream &os) override;
  bool IsTrue() const override {
    return GetValue();
  }
};

struct Method {
  std::string name;
  std::vector<std::string> formal_params;
  std::unique_ptr<Ast::Statement> body;
};

class Class;

struct ClassInfo {
  std::string name;
  std::unordered_map<std::string, Method> methods;
  const Class *parent;
};

class Class : public Object {
 public:
  explicit Class(std::string name,
                 std::vector<Method> methods,
                 const Class *parent);
  const Method *GetMethod(const std::string &name) const;
  const std::string &GetName() const;
  void Print(std::ostream &os) override;
  bool IsTrue() const override {
    return true;
  }

 private:
  ClassInfo class_info_;
};

class ClassInstance : public Object {
 public:
  explicit ClassInstance(const Class &cls);

  void Print(std::ostream &os) override;

  ObjectHolder Call(const std::string &method,
                    const std::vector<ObjectHolder> &actual_args);
  bool HasMethod(const std::string &method, size_t argument_count) const;

  Closure &Fields();
  const Closure &Fields() const;

  bool IsTrue() const override {
    return true;
  }

 private:
  const Class &class_;
  Closure fields_;

  friend bool Equal(ObjectHolder lhs, ObjectHolder rhs);
};

void RunObjectsTests(TestRunner &test_runner);

}
