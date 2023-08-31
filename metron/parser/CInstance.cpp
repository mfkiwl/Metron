#include "CInstance.hpp"

#include "CNodeCall.hpp"
#include "CNodeClass.hpp"
#include "CNodeDeclaration.hpp"
#include "CNodeExpression.hpp"
#include "CNodeField.hpp"
#include "CNodeFunction.hpp"
#include "CNodeStruct.hpp"
#include "CNodeType.hpp"
#include "NodeTypes.hpp"

//------------------------------------------------------------------------------

CHECK_RETURN Err CInstLog::log_action(CNode* node, TraceAction action) {
  Err err;

  if (action != ACT_READ && action != ACT_WRITE) {
    assert(false);
    exit(-1);
  }

  auto old_state = state_stack.back();
  auto new_state = merge_action(old_state, action);

  // LOG_R("%s %s %s\n", to_string(action), to_string(old_state),
  // to_string(new_state));

  auto t = typeid(*this).name();

  if (old_state != new_state) {
    action_log.push_back({old_state, new_state, action, node});
  }

  state_stack.back() = new_state;

  if (new_state == CTX_INVALID) {
    LOG_R("Trace error: state went from %s to %s\n", to_string(old_state),
          to_string(new_state));
    err << ERR("Invalid context state\n");
  }

  return err;
}

//------------------------------------------------------------------------------

CInstClass::CInstClass(CNodeClass* node_class) : node_class(node_class) {
  for (auto node_field : node_class->all_fields) {
    auto field_inst = new CInstField(node_field);
    inst_fields.push_back(field_inst);
  }
}

//----------------------------------------

std::string_view CInstClass::get_name() const { return node_class->get_name(); }

//----------------------------------------

INamed* CInstClass::resolve(std::string_view name) {
  for (auto f : inst_fields) {
    if (f->node_field->get_name() == name) return f;
  }
  LOG_R("Could not resolve %.*s\n", name.size(), name.data());
  assert(false);
  return nullptr;
}

//----------------------------------------

Err CInstClass::log_action(CNode* node, TraceAction action) {
  Err err;
  for (auto f : inst_fields) {
    err << f->log_action(node, action);
  }
  return err;
}

//----------------------------------------

void CInstClass::dump_tree() {
  auto name = node_class->get_name();
  LOG_G("Class %.*s\n", int(name.size()), name.data());
  LOG_INDENT_SCOPE();
  for (auto f : inst_fields)  f->dump_tree();
  for (auto f : entry_points) f->dump_tree();
}

//------------------------------------------------------------------------------

CInstStruct::CInstStruct(CNodeStruct* node_struct) : node_struct(node_struct) {
  assert(node_struct);
  for (auto node_field : node_struct->all_fields) {
    auto inst_field = new CInstField(node_field);
    inst_fields.push_back(inst_field);
  }
}

//----------------------------------------

std::string_view CInstStruct::get_name() const {
  return node_struct->get_name();
}

//----------------------------------------

INamed* CInstStruct::resolve(std::string_view name) {
  for (auto field : inst_fields) {
    if (field->get_name() == name) return field;
  }
  return nullptr;
}

//----------------------------------------

Err CInstStruct::log_action(CNode* node, TraceAction action) {
  Err err;
  for (auto f : inst_fields) {
    err << f->log_action(node, action);
  }
  return err;
}

//----------------------------------------

void CInstStruct::dump_tree() {
  auto name = node_struct->get_name();
  LOG_G("Struct %.*s\n", int(name.size()), name.data());
  LOG_INDENT_SCOPE();
  for (auto f : inst_fields) f->dump_tree();
}

//------------------------------------------------------------------------------

/*
[000.012]  ┃       ┣━━╸▆ CNodeField =
[000.012]  ┃       ┃   ┣━━╸▆ type : CNodeClassType =
[000.012]  ┃       ┃   ┃   ┗━━╸▆ name : CNodeIdentifier = "Submod"
[000.012]  ┃       ┃   ┗━━╸▆ name : CNodeIdentifier = "s"

[000.012]  ┃       ┣━━╸▆ CNodeField =
[000.012]  ┃       ┃   ┣━━╸▆ type : CNodeStructType =
[000.012]  ┃       ┃   ┃   ┗━━╸▆ name : CNodeIdentifier = "Substruct"
[000.012]  ┃       ┃   ┗━━╸▆ name : CNodeIdentifier = "t"

[000.013]  ┃       ┣━━╸▆ CNodeField =
[000.013]  ┃       ┃   ┣━━╸▆ type : CNodeBuiltinType =
[000.013]  ┃       ┃   ┃   ┗━━╸▆ name : CNodeIdentifier = "int"
[000.013]  ┃       ┃   ┗━━╸▆ name : CNodeIdentifier = "x"
*/

CInstField::CInstField(CNodeField* node_field) : node_field(node_field) {
  if (node_field->_type_class) {
    inst_value = new CInstClass(node_field->_type_class);
  } else if (node_field->_type_struct) {
    inst_value = new CInstStruct(node_field->_type_struct);
  } else if (node_field->is_array()) {
    auto node_type = node_field->child<CNodeType>();
    auto node_array = node_field->child("array");
    inst_value = new CInstArray(node_type, node_array);
  } else {
    auto node_type = node_field->child<CNodeType>();
    inst_value = new CInstPrimitive(node_type);
  }
}

//----------------------------------------

std::string_view CInstField::get_name() const { return node_field->get_name(); }

//----------------------------------------

void CInstField::dump_tree() {
  auto name = node_field->get_name();

  LOG_G("Field %.*s\n", int(name.size()), name.data());

  {
    LOG_INDENT_SCOPE();

    if (auto dumpable = dynamic_cast<IDumpable*>(inst_value)) {
      dumpable->dump_tree();
    }
  }
}

//------------------------------------------------------------------------------

CInstReturn::CInstReturn(CNodeType* node_type) : node_type(node_type) {
  if (auto builtin = node_type->as_a<CNodeBuiltinType>()) {
    inst_value = new CInstPrimitive(node_type);
  }
  else if (auto node_struct = node_type->as_a<CNodeStructType>()) {
    auto top = node_type->ancestor<CNodeTranslationUnit>();

    printf("%p\n", top);

    //inst_value = new CInstStruct(node_type);
  }
}

void CInstReturn::dump_tree() {
  LOG_G("Return : ");
  dynamic_cast<IDumpable*>(inst_value)->dump_tree();
}

//------------------------------------------------------------------------------

CInstCall::CInstCall(CInstClass* parent, CNodeFunction* node_function, CNodeCall* node_call)
    : parent(parent), node_function(node_function), node_call(node_call) {

  //auto func_name = node_call->get_name();

  LOG_R("----------\n");
  node_function->dump_tree();

  //LOG_M("CInstFunction(%p, %p) = %.*s\n", parent, node_function, int(func_name.size()), func_name.data());

  auto node_params = node_function->child("params");
  for (auto p : node_params) {
    if (auto param = p->as_a<CNodeDeclaration>()) {
      auto inst_arg = new CInstArg(param);
      inst_args.push_back(inst_arg);
    }
  }

  auto node_return = node_function->child_as<CNodeType>("return_type");
  assert(node_return);
  inst_return = new CInstReturn(node_return);
}

//----------------------------------------

std::string_view CInstCall::get_name() const {
  return node_function->get_name();
}

//----------------------------------------

INamed* CInstCall::resolve(std::string_view name) {
  for (auto p : inst_args) {
    if (p->get_name() == name) return p;
  }

  if (name == "return") {
    return inst_return;
  }

  assert(parent);
  return parent->resolve(name);
}

//----------------------------------------

void CInstCall::dump_tree() {
   auto name = node_function->get_name();
   LOG_G("Call %.*s\n", int(name.size()), name.data());
   LOG_INDENT_SCOPE();
  for (auto p : inst_args)  p->dump_tree();
  if (inst_return) dynamic_cast<IDumpable*>(inst_return)->dump_tree();
  for (auto c : inst_calls) c->dump_tree();
  //inst_return->dump_tree();
}

//------------------------------------------------------------------------------

CInstArg::CInstArg(CNodeDeclaration* node_param)
    : node_param(node_param) {
  if (node_param->is_array()) {
    auto node_type  = node_param->child_as<CNodeType>("type");
    auto node_array = node_param->child_as<CNode>("array");

    inst_value = new CInstArray(node_type, node_array);
  } else {
    auto node_type = node_param->child_as<CNodeType>("type");

    if (node_type->as_a<CNodeBuiltinType>()) {
      inst_value = new CInstPrimitive(node_type);
    } else {
      inst_value = new CInstStruct(node_param->_type_struct);
    }
  }
}

//----------------------------------------

std::string_view CInstArg::get_name() const { return node_param->get_name(); }

//----------------------------------------

void CInstArg::dump_tree() {
  auto name = node_param->get_name();
  LOG_G("Param %.*s : ", int(name.size()), name.data());
  dynamic_cast<IDumpable*>(inst_value)->dump_tree();
}

//------------------------------------------------------------------------------

CInstPrimitive::CInstPrimitive(CNodeType* node_type) : node_type(node_type) {}

//----------------------------------------

void CInstPrimitive::dump_tree() {
  auto prim_type = node_type->get_text();
  LOG_G("Primitive %.*s", int(prim_type.size()), prim_type.data());
  for (auto state : state_stack) {
    LOG_G(" %s", to_string(state));
  }
  LOG_G("\n");
}

//------------------------------------------------------------------------------

CInstArray::CInstArray(CNodeType* node_type, CNode* node_array)
    : node_type(node_type), node_array(node_array) {}

//----------------------------------------

void CInstArray::dump_tree() {
  auto text_type = node_type->get_text();
  auto text_array = node_array->get_text();
  LOG_G("Array %.*s%.*s\n", int(text_type.size()), text_type.data(),
        int(text_array.size()), text_array.data());
}

//------------------------------------------------------------------------------
