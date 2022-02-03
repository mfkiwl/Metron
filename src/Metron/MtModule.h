#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tree_sitter/api.h"
#include "MtIterator.h"

typedef std::function<void(MtHandle)> NodeVisitor;
typedef std::function<void(MtHandle parent, MtHandle child)> NodeVisitor2;
typedef std::vector<uint8_t> blob;

//------------------------------------------------------------------------------

struct MtModule {

  std::string input_filename;
  std::string output_filename;

  const TSLanguage* lang = nullptr;
  TSParser* parser = nullptr;

  blob src_blob;
  TSTree* tree = nullptr;
  const char* source = nullptr;
  const char* source_end = nullptr;
  MtHandle root;

  std::vector<MtHandle> moduleparams;

  std::vector<MtHandle> inputs;
  std::vector<MtHandle> outputs;

  std::vector<MtHandle> localparams;
  std::vector<MtHandle> fields;
  std::vector<MtHandle> submodules;

  MtHandle node_init;
  MtHandle node_tick;
  MtHandle node_tock;

  std::vector<MtHandle> tasks;
  std::vector<MtHandle> functions;

  MtHandle module_template;
  MtHandle module_class;
  MtHandle module_param_list;
  std::string module_name;

  MtModule();
  ~MtModule();
  void load(const std::string& input_filename, const std::string& output_filename);

  void dump_node(MtHandle n, int index = 0, int field = 0, int depth = 0);
  void dump_tree(MtHandle n, int index, int field, int depth, int maxdepth);
  void dump_tree(MtHandle n, int maxdepth) { dump_tree(n, 0, n.field, 0, maxdepth); }
  void dump_tree(MtHandle n) { dump_tree(n, 0, n.field, 0, 255); }

  void visit_tree(MtHandle n, NodeVisitor cv);
  void visit_tree2(MtHandle parent, NodeVisitor2 cv);

  // Text handling
  const char* start(MtHandle n);
  const char* end(MtHandle n);
  std::string body(MtHandle n);
  bool match(MtHandle n, const char* s);

  std::string node_to_name(MtHandle n);
  std::string node_to_type(MtHandle n);

  // Field introspection
  bool field_is_primitive(MtHandle n);
  bool field_is_module(MtHandle n);
  bool field_is_static(MtHandle n);
  bool field_is_const(MtHandle n);
  bool field_is_param(MtHandle n);
  bool field_is_input(MtHandle n);
  bool field_is_output(MtHandle n);

  MtHandle get_by_id(std::vector<MtHandle>& handles, MtHandle id);

  MtHandle get_field_by_id(MtHandle id)    { return get_by_id(fields, id); }
  MtHandle get_task_by_id(MtHandle id)     { return get_by_id(tasks, id); }
  MtHandle get_function_by_id(MtHandle id) { return get_by_id(functions, id);}
  MtHandle get_input_by_id(MtHandle id)    { return get_by_id(inputs, id); }
  MtHandle get_output_by_id(MtHandle id)   { return get_by_id(outputs, id); }

  // Scanner
  void find_module();
  void collect_moduleparams();
  void collect_fields();
};

//------------------------------------------------------------------------------
