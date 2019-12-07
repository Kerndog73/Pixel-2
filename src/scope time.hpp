//
//  scope time.hpp
//  Animera
//
//  Created by Indi Kernick on 1/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef scope_time_hpp
#define scope_time_hpp

#ifdef ENABLE_SCOPE_TIME

#include <chrono>
#include <unordered_map>

class ScopeTime {
public:
  explicit ScopeTime(const char *);
  ~ScopeTime();
  
  static void print();
  static void reset();
  
private:
  using Clock = std::chrono::high_resolution_clock;

  Clock::time_point start;

  struct TreeNode {
    size_t calls;
    Clock::duration time;
    std::unordered_map<const char *, TreeNode> children;
    const char *name;
    TreeNode *parent;
  };
  
  static inline TreeNode tree {0, {}, {}, "ROOT", nullptr};
  static inline TreeNode *current = &tree;
  
  static void printImpl(const TreeNode *, int);
  
  static constexpr int name_indent = 2;
  static constexpr int num_prec = 4;
  static constexpr int name_width = 48;
  static constexpr int rest_width = 24;
};

inline ScopeTime::ScopeTime(const char *name) {
  TreeNode *prevCurrent = current;
  current = &current->children[name];
  current->parent = prevCurrent;
  current->name = name;
  start = Clock::now();
}

inline ScopeTime::~ScopeTime() {
  current->time += Clock::now() - start;
  ++current->calls;
  current = current->parent;
}

#define CONCAT_IMPL(A, B) A##B
#define CONCAT(A, B) CONCAT_IMPL(A, B)
#define SCOPE_TIME(NAME) ScopeTime CONCAT(scope_time_, __LINE__) {NAME}
#define SCOPE_TIME_PRINT() ScopeTime::print()
#define SCOPE_TIME_RESET() ScopeTime::reset()

#else

#define SCOPE_TIME(NAME)
#define SCOPE_TIME_PRINT()
#define SCOPE_TIME_RESET()

#endif

#endif
