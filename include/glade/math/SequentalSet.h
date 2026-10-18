#pragma once

#include <vector>
#include <algorithm>
#include <glade/debug/log.h>

template <class ValueType> class SequentalSet {
public:
  typedef std::vector<ValueType> Values;
  typedef typename Values::iterator iterator;

private:
  Values values;

public:
  void insert(ValueType val) {
    ::log("iNSERTING");

    if (std::find(values.begin(), values.end(), val) == values.end()) {
      values.push_back(val);
    } else {
      ::log("Warning: element already exists");
    }
  }

  iterator begin() { return values.begin(); }
  iterator end() { return values.end(); }
  iterator erase(iterator position) { return values.erase(position); }
  void erase(ValueType val) { erase(std::find(values.begin(), values.end(), val)); }
  bool empty() { return values.empty(); }
};
