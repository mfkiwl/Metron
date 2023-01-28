#include "MtTracer2.h"
#include "MtNode.h"
#include "MtMethod.h"
#include "MtChecker.h"

//------------------------------------------------------------------------------

MtTracer2::MtTracer2(MtModLibrary* lib, MtModuleInstance* root_inst, bool verbose)
: lib(lib),
  root_inst(root_inst),
  verbose(verbose) {
}

CHECK_RETURN Err MtTracer2::log_action(MtFieldInstance* field_inst, TraceAction action, SourceRange source) {

  Err err;
#if 0
  assert(method_ctx->context_type == CTX_METHOD);
  assert(method_ctx);
  assert(dst_ctx);

  if (action == CTX_WRITE) {
    if (dst_ctx->context_type != CTX_RETURN) {
      method_ctx->method->writes.insert(dst_ctx);
    }
  }

  auto old_state = dst_ctx->log_top.state;
  auto new_state = merge_action(old_state, action);
  dst_ctx->log_top.state = new_state;

  if (new_state == CTX_INVALID) {
    printf("Invalid context state at\n");
    for (auto c = source.start; c != source.end; c++) {
      putc(*c, stdout);
    }
    printf("\n");

  }
#endif
  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_method(MtMethod* method) {
  Err err;

  auto method_inst = root_inst->get_method(method->name());

  auto node_body = method->_node.get_field(field_body);
  if (MtChecker::has_non_terminal_return(node_body)) {
    err << ERR("Method %s has non-terminal return\n", method->cname());
  }

  err << trace_sym_function_definition(method_inst, method->_node);

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_identifier(MtMethodInstance* inst, MnNode node, TraceAction action) {
  Err err;

  switch (node.sym) {
    case sym_qualified_identifier:
    case alias_sym_namespace_identifier:
      // pretty sure these can't do anything
      break;
    case sym_identifier:
    case alias_sym_field_identifier: {
      MtFieldInstance* field_inst = inst->_module->get_field(node.text());
      err << log_action(field_inst, action, node.get_source());
      break;
    }
    default:
      err << trace_default(inst, node);
      break;
  }
  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_declarator(MtMethodInstance* inst, MnNode node) {
  Err err;

  switch (node.sym) {
    case sym_identifier:
      err << trace_identifier(inst, node, CTX_READ);
      break;
    case sym_init_declarator:
      //err << trace_sym_init_declarator(ctx, node);
      break;
    default:
      err << trace_default(inst, node);
      break;
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_statement(MtMethodInstance* inst, MnNode node) {
  Err err;

  switch (node.sym) {
    case sym_compound_statement:
      err << trace_sym_compound_statement(inst, node);
      break;
    case sym_case_statement:
      //err << trace_sym_case_statement(inst, node);
      break;
    case sym_if_statement:
      //err << trace_sym_if_statement(inst, node);
      break;
    case sym_expression_statement:
      err << trace_expression(inst, node.child(0), CTX_READ);
      break;
    case sym_switch_statement:
      //err << trace_sym_switch_statement(inst, node);
      break;
    case sym_return_statement:
      //err << trace_sym_return_statement(inst, node);
      break;
    case sym_for_statement:
      //err << trace_sym_for_statement(inst, node);
      break;

    default:
      err << trace_default(inst, node);
      break;
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_expression(MtMethodInstance* inst, MnNode node, TraceAction action) {
  Err err;

  switch (node.sym) {
    case sym_identifier:
    case sym_qualified_identifier:
      err << trace_identifier(inst, node, action);
      break;
    case sym_conditional_expression:
      //err << trace_sym_conditional_expression(inst, node);
      break;
    case sym_field_expression:
      //err << trace_sym_field_expression(inst, node, action);
      break;
    case sym_subscript_expression:
      err << trace_expression(inst, node.get_field(field_index), CTX_READ);
      err << trace_expression(inst, node.get_field(field_argument), action);
      break;
    case sym_call_expression:
      //err << trace_sym_call_expression(inst, node);
      break;
    case sym_update_expression:
      // this is "i++" or similar, which is a read and a write.
      err << trace_expression(inst, node.get_field(field_argument), CTX_READ);
      err << trace_expression(inst, node.get_field(field_argument), CTX_WRITE);
      break;
    case sym_assignment_expression:
      //err << trace_sym_assignment_expression(inst, node);
      break;
    case sym_parenthesized_expression:
      err << trace_expression(inst, node.child(1), action);
      break;

    case sym_unary_expression:
      err << trace_expression(inst, node.get_field(field_argument), CTX_READ);
      break;

    case sym_binary_expression:
      //err << trace_sym_binary_expression(inst, node);
      break;

    case sym_initializer_list:
      //err << trace_sym_initializer_list(inst, node);
      break;

      /*
      case sym_condition_clause:
        err << trace_expression(ctx, node.child(1), action);
        break;
      */

    default:
      err << trace_default(inst, node);
      break;
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_default(MtMethodInstance* inst, MnNode node) {
  Err err;
  if (!node.is_named()) return err;

  switch (node.sym) {
    case sym_comment:
    case sym_using_declaration:
    case sym_number_literal:
    case sym_string_literal:
    case sym_break_statement:
      break;
    default:
      // KCOV_OFF
      err << ERR("%s : No handler for %s\n", __func__, node.ts_node_type());
      node.error();
      break;
      // KCOV_ON
  }

  return err;
}























































//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_sym_case_statement(MtMethodInstance* inst, MnNode node) {
  Err err;
  assert(node.sym == sym_case_statement);

  // Everything after the colon should be statements.

  bool hit_colon = false;
  for (auto child : node) {
    if (child.sym == anon_sym_COLON) {
      hit_colon = true;
      continue;
    }
    if (hit_colon) {
      err << trace_statement(inst, child);
    }
  }

  if (!MtChecker::ends_with_break(node)) {
    err << ERR("Case statement in %s does not end with break\n", inst->_name.c_str());
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_sym_compound_statement(MtMethodInstance* inst, MnNode node) {
  Err err;
  assert(node.sym == sym_compound_statement);

  bool noconvert = false;
  bool dumpit = false;

  for (const auto& child : node) {
    if (noconvert) { noconvert = false; continue; }
    if (dumpit) { child.dump_tree(); dumpit = false; }
    if (child.sym == sym_comment && child.contains("dumpit")) { dumpit = true; }
    if (child.sym == sym_comment && child.contains("metron_noconvert")) { noconvert = true; }

    if (child.sym == sym_declaration) {
      err << trace_sym_declaration(inst, child);
    } else if (child.is_statement()) {
      err << trace_statement(inst, child);
    } else {
      err << trace_default(inst, child);
    }
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_sym_declaration(MtMethodInstance* inst, MnNode node) {
  Err err;
  assert(node.sym == sym_declaration);

  auto node_type = node.get_field(field_type);
  auto node_decl = node.get_field(field_declarator);

  err << trace_declarator(inst, node_decl);

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_sym_for_statement(MtMethodInstance* inst, MnNode node) {
  Err err;

  for (auto c : node) {
    if (c.field == field_initializer) {
      if (c.sym == sym_declaration) {
        err << trace_sym_declaration(inst, c);
      }
      else {
        err << trace_expression(inst, c, CTX_READ);
      }
    }
    else if (c.is_expression()) {
      err << trace_expression(inst, c, CTX_READ);
    }
    else if (c.is_statement()) {
      err << trace_statement(inst, c);
    }
    else {
      err << trace_default(inst, c);
    }
  }

  return err;
}

//------------------------------------------------------------------------------

CHECK_RETURN Err MtTracer2::trace_sym_function_definition(MtMethodInstance* inst, MnNode node) {
  Err err;
  assert(node.sym == sym_function_definition);

  auto node_type = node.get_field(field_type);
  auto node_decl = node.get_field(field_declarator);
  auto node_body = node.get_field(field_body);

  err << trace_sym_compound_statement(inst, node_body);

  return err;
}

//------------------------------------------------------------------------------
