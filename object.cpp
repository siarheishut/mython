#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>
#include <unordered_map>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream &os) {
  auto str_method = class_.GetMethod("__str__");
  if (str_method) {
    str_method->body->Execute(fields_)->Print(os);
  } else {
    os << this;
  }
}

bool ClassInstance::HasMethod(const std::string &method,
                              size_t argument_count) const {
  auto m = class_.GetMethod(method);
  if (m) {
    return (m->formal_params.size() == argument_count);
  }
  return false;
}

const Closure &ClassInstance::Fields() const {
  return fields_;
}

Closure &ClassInstance::Fields() {
  return fields_;
}

ClassInstance::ClassInstance(const Class &cls) : class_(cls) {
  fields_["self"] = ObjectHolder::Share(*this);
}

ObjectHolder ClassInstance::Call(const std::string &method,
                                 const std::vector<ObjectHolder> &actual_args) {
  Closure method_args;
  auto *method_ = class_.GetMethod(method);
  for (int i = 0; i < method_->formal_params.size(); ++i) {
    method_args[method_->formal_params[i]] = actual_args[i];
  }
  for (const auto &[field, value] : fields_) {
    method_args[field] = value;
  }
  return method_->body->Execute(method_args);
}

Class::Class(std::string name,
             std::vector<Method> methods,
             const Class *parent) {
  std::unordered_map<std::string, Method> methods_map;
  for (auto &method : methods) {
    methods_map[method.name] = std::move(method);
  }
  class_info_ = {.name = std::move(name), .methods = std::move(methods_map),
      .parent = parent};
}

const Method *Class::GetMethod(const std::string &name) const {
  auto found = class_info_.methods.find(name);
  if (found != class_info_.methods.end()) {
    return &found->second;
  } else if (class_info_.parent) {
    return class_info_.parent->GetMethod(name);
  } else {
    return nullptr;
  }
}

void Class::Print(ostream &os) {
  os << class_info_.name;
}

const std::string &Class::GetName() const {
  return class_info_.name;
}

void Bool::Print(std::ostream &os) {
  bool b = GetValue();
  if (b) {
    os << "True";
  } else {
    os << "False";
  }
}

} /* namespace Runtime */
