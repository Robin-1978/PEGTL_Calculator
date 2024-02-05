#pragma once
#include <iostream>
#include <vector>

#include "grammer.h"

namespace calc {

enum OpCode : unsigned char {
  // Control flow
  OP_NOP,     // No operation
  OP_HALT,    // Halt execution
  OP_JUMP,    // Unconditional jump
  OP_JUMPIF,  // Conditional jump
  OP_CALL,    // Call function
  OP_RETURN,  // Return from function

  // Data manipulation
  OP_LOAD,   // Load data from memory to the stack
  OP_STORE,  // Store the top stack value to memory

  // Arithmetic operations
  OP_ADD,  // Addition
  OP_SUB,  // Subtraction
  OP_MUL,  // Multiplication
  OP_DIV,  // Division
  OP_MOD,  // Modulo

  // Logical operations
  OP_AND,  // Logical AND
  OP_OR,   // Logical OR
  OP_NOT,  // Logical NOT

  // Comparison operations
  OP_EQ,   // Equal to
  OP_NEQ,  // Not equal to
  OP_LT,   // Less than
  OP_GT,   // Greater than
  OP_LTE,  // Less than or equal to
  OP_GTE,  // Greater than or equal to

  // Register operations
  // OP_MOVE,   // Move the value from one register to another
  // OP_LOADI,  // Load an immediate value to a register

  // Stack operations
  OP_PUSH,  // Push a value onto the stack
  OP_POP,   // Pop a value from the stack

  // Other
  OP_PRINT,  // Print the top stack value
};

struct vm {
  std::vector<int> stack;
  std::vector<int> memory;
  std::vector<int> registers;

  bool run(std::string& code, unsigned& ip) {
    while (ip < code.size()) {
      OpCode op = (OpCode)code[ip++];
      switch (op) {
        case OP_PUSH:
          stack.push_back(*(int*)&code[ip]);
          ip += 4;
          break;
        case OP_POP:
          stack.pop_back();
          break;
        case OP_STORE: {
          uint8_t address = (uint8_t)code[ip++];
          if (address < memory.size()) {
            memory[address] = stack.back();
            stack.pop_back();
          } else if (address == memory.size()) {
            memory.push_back(stack.back());
            stack.pop_back();
          } else {
            std::cerr << "Error address" << std::endl;
          }
        } break;
        case OP_LOAD: {
          uint8_t address = (uint8_t)code[ip++];
          if (address < memory.size()) {
            stack.push_back(memory[address]);
          } else {
            std::cerr << "Error address" << std::endl;
          }
        } break;
        case OP_ADD:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] + stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_SUB:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] - stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_MUL:
          stack[stack.size() - 2] =
              stack[stack.size() - 2] * stack[stack.size() - 1];
          stack.pop_back();
          break;
        case OP_DIV:
          if (stack[stack.size() - 1] == 0) {
            std::cerr << "Divide zero" << std::endl;
          } else {
            stack[stack.size() - 2] =
                stack[stack.size() - 2] / stack[stack.size() - 1];
            stack.pop_back();
          }
          break;
        case OP_NOP:
          break;
        case OP_HALT:
          return true;
        case OP_MOD:
          if (stack[stack.size() - 1] == 0) {
            std::cerr << "Divide zero" << std::endl;
          } else {
            stack[stack.size() - 2] =
                stack[stack.size() - 2] % stack[stack.size() - 1];
            stack.pop_back();
          }
          break;
        default:
          std::cerr << "Unknown OP" << std::endl;
          break;
      }
    }
    return true;
  }
};

}  // namespace calc
