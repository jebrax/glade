#pragma once

#include <list>

template <class ValueType> class Tree {
public:
  typedef typename std::list<Tree<ValueType>> Children;
  typedef typename Children::iterator ChildrenI;

  class Action {
  public:
    void operator()(Tree<ValueType> &tree) { ::printf("%d\n", tree.getValue()); }
  };

  static void walkDepthFirstPostfix(Tree<ValueType> &tree, Action &func) {
    typename Tree<ValueType>::ChildrenI child = tree.getChildren()->begin();

    while (child != tree.getChildren()->end()) {
      walkDepthFirstPostfix(*child, func);
      ++child;
    }

    func(tree);
  }

  static void walkDepthFirstPrefix(Tree<ValueType> &tree, Action &func) {
    func(tree);
    typename Tree<ValueType>::ChildrenI child = tree.getChildren()->begin();

    while (child != tree.getChildren()->end()) {
      walkDepthFirstPrefix(*child, func);
      ++child;
    }
  }

private:
  Children children;
  ValueType val;

public:
  Tree(const Tree<ValueType> &other) {
    val = other.val;
    children = other.children;
  }

  Tree(const ValueType &val_) : val(val_) {}
  ValueType getValue() const { return val; }
  void setValue(const ValueType &val_) { this->val = val_; }
  void addChild(const Tree<ValueType> &child) { children.push_back(child); }
  Children *getChildren() { return &children; }
};
