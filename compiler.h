#pragma once

#include <unordered_map>

#include "vm.h"

namespace calc {
struct CompileState {
  std::unordered_map<std::string, unsigned> variables;
  uint8_t current{};
  uint8_t GetAddress(const std::string& name) {
    auto it = variables.find(name);
    if (it == variables.end()) {
      variables[name] = current++;
      return current - 1;
    } else {
      return it->second;
    }
  }

  bool Find(const std::string& name) {
    return variables.find(name) != variables.end();
  }
};

void append_op(OpCode op, std::string& code) {
  code.resize(code.size() + 1);
  *(OpCode*)&code[code.size() - 1] = op;
}

void append_number(int num, std::string& code) {
  code.resize(code.size() + sizeof(int));
  *(int*)&code[code.size() - sizeof(int)] = num;
}

void append_address(uint8_t address, std::string& code) {
  code.resize(code.size() + 1);
  *(uint8_t*)&code[code.size() - 1] = address;
}

bool compile(const tao::pegtl::parse_tree::node& node, CompileState& state,
             std::string& code) {
  if (node.is_root())
    return compile(*node.children[0], state, code);  // skip root

  if (node.children.empty()) {
    if (node.is_type<integer>()) {
      append_op(OP_PUSH, code);
      append_number(std::stoi(node.string()), code);
      return true;
    } else if (node.is_type<identifier>()) {
      if (state.Find(node.string())) {
        append_op(OP_LOAD, code);
        append_address(state.GetAddress(node.string()), code);
        return true;
      } else {
        std::cerr << "Variable is not defined: " << node.string() << std::endl;
        return false;
      }
    } else {
      std::cerr << "Node error" << std::endl;
      return false;
    }
  } else {
    if (node.children.size() == 2) {
      if (node.is_type<plus>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_ADD, code);
        return true;
      } else if (node.is_type<minus>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_SUB, code);
        return true;
      } else if (node.is_type<multiply>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_MUL, code);
        return true;
      } else if (node.is_type<divide>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_DIV, code);
        return true;
      } else if (node.is_type<modulo>()) {
        if (!compile(*node.children[0], state, code)) return false;
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_MOD, code);
        return true;
      } else if (node.is_type<assign>()) {
        if (!compile(*node.children[1], state, code)) return false;
        append_op(OP_STORE, code);
        append_address(state.GetAddress(node.children[0]->string()), code);
        return true;
      } else {
        std::cerr << "Node error" << std::endl;
        return false;
      }
    } else if (node.children.size() == 1) {
      if (!compile(*node.children[0], state, code)) return false;
      append_op(OP_PRINT, code);
      return true;
    } else {
      std::cerr << "Node error" << std::endl;
      return false;
    }
  }
};

}  // namespace calc
