#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure &closure) {
  return closure[var_name] = right_value->Execute(closure);
}

Assignment::Assignment(string var, unique_ptr<Statement> rv) {
  var_name = std::move(var);
  right_value = std::move(rv);
}

VariableValue::VariableValue(string var_name) {
  dotted_ids.push_back(std::move(var_name));
}

VariableValue::VariableValue(vector<string> dotted_ids) {
  this->dotted_ids = std::move(dotted_ids);
}

ObjectHolder VariableValue::Execute(Closure &closure) {
  if (closure.find(dotted_ids[0]) == closure.end())
    throw std::runtime_error("No such variable!");

  if (dotted_ids.size() == 1) {
    return closure[dotted_ids[0]];
  }
  auto class_ = closure[dotted_ids[0]].TryAs<Runtime::ClassInstance>();
  return class_->Fields()[dotted_ids[1]];
}

unique_ptr<Print> Print::Variable(string var) {
  return make_unique<Print>(make_unique<VariableValue>(std::move(var)));
}

Print::Print(unique_ptr<Statement> argument) {
  args.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args) {
  this->args = std::move(args);
}

ObjectHolder Print::Execute(Closure &closure) {
  bool first = true;
  for (auto &arg : args) {
    if (!first) {
      *output << ' ';
    }
    first = false;

    auto value = arg->Execute(closure);
    if (value) {
      value->Print(*output);
    } else {
      *output << "None";
    }
  }
  *output << '\n';

  return ObjectHolder::None();
}

ostream *Print::output = &cout;

void Print::SetOutputStream(ostream &output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
    unique_ptr<Statement> object,
    string method,
    vector<unique_ptr<Statement>> args
) {
  this->object = std::move(object);
  this->method = std::move(method);
  this->args = std::move(args);
}

ObjectHolder MethodCall::Execute(Closure &closure) {
  vector<ObjectHolder> act_args;
  act_args.reserve(args.size());
  for (auto &arg : args) {
    act_args.push_back(arg->Execute(closure));
  }

  auto *this_class = object->Execute(closure).TryAs<Runtime::ClassInstance>();

  return this_class->Call(method, act_args);
}

ObjectHolder Stringify::Execute(Closure &closure) {
  ostringstream out;
  argument->Execute(closure)->Print(out);
  return ObjectHolder::Own(Runtime::String(out.str()));
}

ObjectHolder Add::Execute(Closure &closure) {
  auto lhs_holder = lhs->Execute(closure);
  auto rhs_holder = rhs->Execute(closure);
  if (lhs_holder.TryAs<Runtime::Number>() &&
      rhs_holder.TryAs<Runtime::Number>()) {
    int lhs_val = lhs_holder.TryAs<Runtime::Number>()->GetValue();
    int rhs_val = rhs_holder.TryAs<Runtime::Number>()->GetValue();
    return ObjectHolder::Own(Runtime::Number(lhs_val + rhs_val));
  } else if (lhs_holder.TryAs<Runtime::String>() &&
      rhs_holder.TryAs<Runtime::String>()) {
    std::string lhs_val = lhs_holder.TryAs<Runtime::String>()->GetValue();
    std::string rhs_val = rhs_holder.TryAs<Runtime::String>()->GetValue();
    return ObjectHolder::Own(Runtime::String(lhs_val + rhs_val));
  } else if (lhs_holder.TryAs<Runtime::ClassInstance>()) {
    auto lhs_ = lhs_holder.TryAs<Runtime::ClassInstance>();
    if (lhs_->HasMethod("__add__", 1)) {
      return lhs_->Call("__add__", {rhs_holder});
    }
  }

  throw runtime_error("Bad addition");
}

ObjectHolder Sub::Execute(Closure &closure) {
  auto lhs_holder = lhs->Execute(closure);
  auto rhs_holder = rhs->Execute(closure);
  if (lhs_holder.TryAs<Runtime::Number>() &&
      rhs_holder.TryAs<Runtime::Number>()) {
    auto lhs_val = lhs_holder.TryAs<Runtime::Number>()->GetValue();
    auto rhs_val = rhs_holder.TryAs<Runtime::Number>()->GetValue();
    return ObjectHolder::Own(Runtime::Number(lhs_val - rhs_val));
  }

  throw runtime_error("Bad subtraction");
}

ObjectHolder Mult::Execute(Runtime::Closure &closure) {
  auto lhs_holder = lhs->Execute(closure);
  auto rhs_holder = rhs->Execute(closure);
  if (lhs_holder.TryAs<Runtime::Number>() &&
      rhs_holder.TryAs<Runtime::Number>()) {
    auto lhs_val = lhs_holder.TryAs<Runtime::Number>()->GetValue();
    auto rhs_val = rhs_holder.TryAs<Runtime::Number>()->GetValue();
    return ObjectHolder::Own(Runtime::Number(lhs_val * rhs_val));
  }

  throw runtime_error("Bad multiplication");
}

ObjectHolder Div::Execute(Runtime::Closure &closure) {
  auto lhs_holder = lhs->Execute(closure);
  auto rhs_holder = rhs->Execute(closure);
  if (lhs_holder.TryAs<Runtime::Number>() &&
      rhs_holder.TryAs<Runtime::Number>()) {
    auto lhs_val = lhs_holder.TryAs<Runtime::Number>()->GetValue();
    auto rhs_val = rhs_holder.TryAs<Runtime::Number>()->GetValue();
    return ObjectHolder::Own(Runtime::Number(lhs_val / rhs_val));
  }

  throw runtime_error("Bad division");
}

ObjectHolder Compound::Execute(Closure &closure) {
  for (auto &statement : statements) {
    if (dynamic_cast<Return *>(statement.get()))
      return statement->Execute(closure);

    if (dynamic_cast<IfElse *>(statement.get()) ||
        dynamic_cast<MethodCall *>(statement.get())) {
      ObjectHolder result = statement->Execute(closure);
      if (result) {
        return result;
      }
    } else {
      statement->Execute(closure);
    }
  }

  return Runtime::ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure &closure) {
  return statement->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_)
    : class_name(class_.TryAs<Runtime::Class>()->GetName()),
      cls(std::move(class_)) {}

ObjectHolder ClassDefinition::Execute(Runtime::Closure &closure) {
  closure[class_name] = cls;
  return ObjectHolder::None();
}

FieldAssignment::FieldAssignment(
    VariableValue object, string field_name, unique_ptr<Statement> rv
)
    : object(std::move(object)),
      field_name(std::move(field_name)),
      right_value(std::move(rv)) {
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure &closure) {
  auto this_class = object.Execute(closure).TryAs<Runtime::ClassInstance>();
  auto &field = this_class->Fields()[field_name];
  field = right_value->Execute(closure);
  return field;
}

IfElse::IfElse(
    unique_ptr<Statement> condition,
    unique_ptr<Statement> if_body,
    unique_ptr<Statement> else_body
) {
  this->condition = std::move(condition);
  this->if_body = std::move(if_body);
  this->else_body = std::move(else_body);
}

ObjectHolder IfElse::Execute(Runtime::Closure &closure) {
  auto cond = condition->Execute(closure);

  if (Runtime::IsTrue(cond)) {
    return if_body->Execute(closure);
  } else if (else_body) {
    return else_body->Execute(closure);
  }

  return ObjectHolder::None();
}

ObjectHolder Or::Execute(Runtime::Closure &closure) {
  ObjectHolder lhs_h = lhs->Execute(closure);
  ObjectHolder rhs_h = rhs->Execute(closure);
  if (!lhs_h) {
    lhs_h = ObjectHolder::Own(Runtime::Bool(false));
  }
  if (!rhs_h) {
    rhs_h = ObjectHolder::Own(Runtime::Bool(false));
  }
  return Runtime::ObjectHolder::Own(
      Runtime::Bool(lhs_h->IsTrue() || rhs_h->IsTrue())
  );
}

ObjectHolder And::Execute(Runtime::Closure &closure) {
  ObjectHolder lhs_h = lhs->Execute(closure);
  ObjectHolder rhs_h = rhs->Execute(closure);
  if (!lhs_h) {
    lhs_h = ObjectHolder::Own(Runtime::Bool(false));
  }
  if (!rhs_h) {
    rhs_h = ObjectHolder::Own(Runtime::Bool(false));
  }
  return Runtime::ObjectHolder::Own(
      Runtime::Bool(lhs_h->IsTrue() && rhs_h->IsTrue())
  );
}

ObjectHolder Not::Execute(Runtime::Closure &closure) {
  ObjectHolder arg = argument->Execute(closure);
  if (!arg) arg = ObjectHolder::Own(Runtime::Bool{false});
  return Runtime::ObjectHolder::Own(Runtime::Bool(!arg->IsTrue()));
}

Comparison::Comparison(
    Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
)
    : comparator(std::move(cmp)), left(std::move(lhs)), right(std::move(rhs)) {}

ObjectHolder Comparison::Execute(Runtime::Closure &closure) {
  return ObjectHolder::Own(Runtime::Bool{
      comparator(left->Execute(closure), right->Execute(closure))
  });
}

NewInstance::NewInstance(
    const Runtime::Class &class_, vector<unique_ptr<Statement>> args
)
    : class_(class_), args(std::move(args)) {}

NewInstance::NewInstance(const Runtime::Class &class_)
    : NewInstance(class_, {}) {}

ObjectHolder NewInstance::Execute(Runtime::Closure &closure) {
  auto *new_instance = new Runtime::ClassInstance(class_);
  if (new_instance->HasMethod("__init__", args.size())) {
    std::vector<ObjectHolder> actual_args;
    actual_args.reserve(args.size());
    for (auto &statement : args) {
      actual_args.push_back(statement->Execute(closure));
    }
    new_instance->Call("__init__", actual_args);
  }

  return ObjectHolder::Share(*new_instance);
}

} /* namespace Ast */
