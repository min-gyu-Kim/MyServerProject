#pragma once

#if defined(NOT_USE_STD)
#else
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace core {
using String = ::std::string;
template <typename T>
using List = ::std::list<T>;
template <typename T>
using Vector = ::std::vector<T>;
template <typename T>
using Stack = ::std::stack<T>;
template <typename T>
using PriorityQueue = ::std::priority_queue<T>;
template <typename T>
using Queue = ::std::queue<T>;
template <typename K, typename V>
using Map = ::std::map<K, V>;
template <typename K, typename V>
using UnorderedMap = ::std::unordered_map<K, V>;
template <typename... Args>
using Tuple = ::std::tuple<Args...>;
} // namespace core
#endif
