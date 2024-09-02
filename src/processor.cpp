#include "processor.h"
#include "simdjson.h"
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace simdjson;

using operation_id = uint64_t;

using operation_time = uint64_t;

class build_operation {
public:
  build_operation(const std::string &display_name, operation_time start_time)
      : display_name_(display_name), start_time_(start_time),
        end_time_(start_time) {}

  void set_end_time(operation_time end_time) { end_time_ = end_time; }

  const std::string &display_name() const { return display_name_; }

  // Const accessor for value computed as end_time - start_time
  operation_time value() const { return end_time_ - start_time_; }

private:
  std::string display_name_;
  operation_time start_time_;
  operation_time end_time_;
};

using operation_id_set = std::unordered_set<operation_id>;

using operation_relation = std::unordered_multimap<operation_id, operation_id>;

using operation_map = std::unordered_map<operation_id, build_operation>;

class build_operation_hierarchy {
  operation_map operations;
  operation_id_set roots;
  operation_relation hierarchy;

public:
  const build_operation &operation(operation_id id) const {
    return operations.at(id);
  }

  const operation_id_set &get_roots() const { return roots; }

  auto children(operation_id id) const { return hierarchy.equal_range(id); }

  void begin_build_operation(operation_id id, const std::string &display_name,
                             operation_time start_time,
                             std::optional<operation_id> parent_id) {
    build_operation op{display_name, start_time};
    operations.emplace(id, op);

    if (parent_id) {
      hierarchy.emplace(parent_id.value(), id);
    } else {
      roots.emplace(id);
    }
  }

  void end_build_operation(operation_id id, operation_time end_time) {
    operations.at(id).set_end_time(end_time);
  }
};

class build_operation_hierarchy_printer {
  const build_operation_hierarchy &hierarchy;
  std::ostream &output;
  int indent_level;

public:
  build_operation_hierarchy_printer(const build_operation_hierarchy &hierarchy,
                                    std::ostream &output)
      : hierarchy(hierarchy), output(output), indent_level(0) {}

  void print() {
    output << "const data = ";
    begin_operation("$");
    begin_children();
    auto needs_comma = false;
    for (auto root_id : hierarchy.get_roots()) {
      if (needs_comma)
        output << ',';
      else
        needs_comma = true;
      print_operation(root_id);
    };
    end_children();
    end_operation();
  }

private:
  void print_operation(operation_id id) {
    print_operation(id, hierarchy.operation(id));
  }

  void print_operation(operation_id id, const build_operation &op) {
    indentation();
    begin_operation(op.display_name());
    auto children = hierarchy.children(id);
    auto has_children = children.first != children.second;
    auto has_child_with_value = false;
    if (has_children) {
      begin_children();
      auto needs_comma = false;
      for (auto it = children.first; it != children.second; ++it) {
        auto child_id = it->second;
        const auto &child = hierarchy.operation(child_id);
        if (child.value() <= 0) {
          // Ignore build operations with no significant duration
          continue;
        } else {
          has_child_with_value = true;
        }
        if (needs_comma)
          output << ',';
        else
          needs_comma = true;
        print_operation(child_id, child);
      }
      end_children();
    }
    if (!has_child_with_value) {
      if (has_children)
        output << ',';
      indentation() << "value: " << op.value() << std::endl;
    }
    end_operation();
  }

  void begin_operation(const std::string &name) {
    output << '{' << std::endl;
    indent();
    indentation() << "name: \"" << name << "\"," << std::endl;
  }
  void end_operation() {
    dedent();
    indentation() << "}" << std::endl;
  }
  void begin_children() {
    indentation() << "children: [" << std::endl;
    indent();
  }
  void end_children() {
    dedent();
    indentation() << "]" << std::endl;
  }
  void indent() { ++indent_level; }
  void dedent() { --indent_level; }
  std::ostream &indentation() {
    for (auto i = 0; i < indent_level; ++i) {
      output << "  ";
    }
    return output;
  }
};

inline void progress(char ch) { std::cout << ch; }

void generate_data_file_from_build_operations(std::istream &json_lines,
                                              const char *output_file) {

  build_operation_hierarchy hierarchy;

  {
    ondemand::parser parser;
    std::string display_name;
    std::string line;

    while (std::getline(json_lines, line)) {
      auto json = parser.iterate(line);

      if (json["displayName"].get_string(display_name, true) == SUCCESS) {
        auto json_id = json["id"].get_uint64();
        auto json_start_time = json["startTime"].get_uint64();
        if (json_id.error() || json_start_time.error()) {
          progress('x');
        } else {
          progress('<');
          auto id = json_id.value();
          auto start_time = json_start_time.value();
          auto json_parent_id = json["parentId"].get_uint64();
          auto parent_id = json_parent_id.error() == SUCCESS
                               ? std::make_optional(json_parent_id.value())
                               : std::nullopt;
          hierarchy.begin_build_operation(id, display_name, start_time,
                                          parent_id);
        }
        continue;
      }
      auto json_end_time = json["endTime"].get_uint64();
      if (json_end_time.error() == SUCCESS) {
        auto json_id = json["id"].get_uint64();
        if (json_id.error()) {
          progress('x');
        } else {
          progress('>');
          hierarchy.end_build_operation(json_id.value(), json_end_time.value());
        }
        continue;
      }

      progress(json.error() == SUCCESS ? '.' : 'x');
    }
  }

  std::ofstream output{output_file};
  build_operation_hierarchy_printer printer{hierarchy, output};
  printer.print();
  output.close();
}
