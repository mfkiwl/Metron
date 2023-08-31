#include "CNodeExpression.hpp"

#include "metrolib/core/Log.h"
#include "CInstance.hpp"

//------------------------------------------------------------------------------

uint32_t CNodeExpression::debug_color() const {
  return COL_AQUA;
}

Err CNodeExpression::emit(Cursor& cursor) {
  return cursor.emit_default(this);
}

bool CNodeExpression::is_integer_constant() {
  //if (child_count() != 1) return false;
  /*
  auto node_unit = child("unit");
  if (!node_unit || node_unit->child_count() != 1) return false;
  auto node_constant = node_unit->child("constant");
  if (!node_constant) return false;
  auto node_int = node_constant->child("int");
  if (!node_int) return false;
  */
  return false;
}

//------------------------------------------------------------------------------

Err CNodeBinaryExp::trace_read(IContext* context) {
  Err err;

  err << child("lhs")->trace_read(context);
  err << child("rhs")->trace_read(context);
  return Err();
}

//------------------------------------------------------------------------------

Err CNodePrefixExp::trace_read(IContext* context) {
  return child("rhs")->trace_read(context);
}

//------------------------------------------------------------------------------

Err CNodeSuffixExp::trace(IContext* context, TraceAction action) {
  return child("lhs")->trace(context, action);
}

//------------------------------------------------------------------------------

Err CNodeAssignExp::trace(IContext* context, TraceAction action) {
  //Err err;
  //err <<
  //return child("lhs")->trace(context, action);
  dump_tree();
  assert(false);
  return Err();
}

//------------------------------------------------------------------------------

Err CNodeIdentifierExp::trace(IContext* context, TraceAction action) {
  auto field_name = get_text();
  INamed* named = context->resolve(field_name);
  assert(named);
  if (auto field = dynamic_cast<CInstField*>(named)) {
    return field->log_action(this, action);
  }
  if (auto arg = dynamic_cast<CInstArg*>(named)) {
    return arg->log_action(this, action);
  }
  assert(false);
  return Err();
}

//------------------------------------------------------------------------------

Err CNodeConstant::emit(Cursor& cursor) {
  return cursor.emit_raw(this);
}

Err CNodeConstant::trace(IContext* context, TraceAction action) {
  return Err();
}

//------------------------------------------------------------------------------

uint32_t CNodeOperator::debug_color() const {
  return COL_SKY;
}

Err CNodeOperator::emit(Cursor& cursor) {
  return cursor.emit_default(this);
}

Err CNodeOperator::trace(IContext* context, TraceAction action) {
  return Err();
}

//------------------------------------------------------------------------------
