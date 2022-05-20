#ifndef __SITO_BUF_H__
#define __SITO_BUF_H__

#include <cstring>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sitobuf {

class inner_string {
  bool type;  // true: std::string, false: char* buf
  union {
    struct {
      char *_buf;
      size_t _capacity;
      size_t _size;
    };
    std::string *s;
  };

 public:
  inner_string(char *buf, size_t n, size_t l = 0)
      : type(false), _buf(buf), _capacity(n), _size(l) {}
  inner_string(std::string &_s) : type(true), s(&_s) {}
  inner_string &operator=(const inner_string &_s) {
    type = _s.type;
    if (type)
      s = _s.s;
    else {
      _buf = _s._buf;
      _capacity = _s._capacity;
      _size = _s._size;
    }
    return *this;
  }
  size_t capacity() { return type ? s->capacity() : _capacity; }
  size_t size() const { return type ? s->size() : _size; }
  const char *data() const { return type ? s->data() : _buf; }
  void resize(size_t n) {
    if (type)
      s->resize(n);
    else {
      if (n > _capacity) throw "resize over capacity";
      _size = n;
    }
  }
  void reserve(size_t n) {
    if (type)
      s->reserve(n);
    else {
      if (n > _capacity) throw "reserve over capacity";
    }
  }
  inner_string &append(const char *_s, size_t _n) {
    if (type)
      s->append(_s, _n);
    else {
      memcpy(_buf + _size, _s, _n);
      _size += _n;
    }
    return *this;
  }
  inner_string &append(const inner_string &_s, size_t n) {
    if (type) {
      if (_s.type)
        s->append((*_s.s).data(), n);
      else
        s->append(_s._buf, _s._size, n);
    } else {
      if (_s.type)
        memcpy(_buf + _size, _s.s->data(), n);
      else
        memcpy(_buf + _size, _s._buf, n);
      _size += n;
    }
    return *this;
  }
  inner_string &append(const inner_string &_s) { return append(_s, _s.size()); }
  inner_string &replace(size_t pos, size_t l1, const char *_s, size_t l2) {
    if (type)
      s->replace(pos, l1, _s, l2);
    else {
      if (l1 == l2) {
        memcpy(_buf + pos, _s, l2);
      } else {
        if (_size + l2 - l1 > _capacity) throw "replace over capacity";
        memmove(_buf + pos + l2, _buf + pos + l1, _size - pos - l1);
        memcpy(_buf + pos, _s, l2);
        _size -= l1 - l2;
      }
    }
    return *this;
  }
};

struct burst_struct {
  uint32_t data_len;
};

class helper {
 public:
 private:
  template <size_t N>
  struct __tuple_size_helper {
    template <typename... P>
    __always_inline size_t operator()(std::tuple<P...> &t) {
      return __build_size_type_helper<decltype(std::get<N - 1>(t))>()(
                 std::get<N - 1>(t)) +
             __tuple_size_helper<N - 1>()(t);
    }
  };
  template <typename T, typename Enable = void>
  struct __build_size_type_helper {
    template <typename _Tp>
    constexpr __always_inline size_t operator()(_Tp &t) {
      return sizeof(_Tp);
    }
    __always_inline size_t operator()(std::string &t) {
      return sizeof(uint32_t) + t.size();
    }
    template <typename _Tp>
    __always_inline size_t operator()(std::vector<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (size_t i = 0; i < t.size(); ++i) {
        s += __build_size_type_helper<_Tp>()(t[i]);
      }
      return s;
    }
    template <typename K, typename V>
    __always_inline size_t operator()(std::map<K, V> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<K>()((K &)it.first);
        s += __build_size_type_helper<V>()(it.second);
      }
      return s;
    }
    template <typename K, typename V>
    __always_inline size_t operator()(std::unordered_map<K, V> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<K>()((K &)it.first);
        s += __build_size_type_helper<V>()(it.second);
      }
      return s;
    }
    template <typename _Tp>
    __always_inline size_t operator()(std::list<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename _Tp>
    __always_inline size_t operator()(std::set<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename _Tp>
    __always_inline size_t operator()(std::unordered_set<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename P1, typename P2>
    __always_inline size_t operator()(std::pair<P1, P2> &t) {
      return __build_size_type_helper<P1>()(t.first) +
             __build_size_type_helper<P2>()(t.second);
    }
    template <typename... P>
    __always_inline size_t operator()(std::tuple<P...> &t) {
      return __tuple_size_helper<sizeof...(P)>()(t);
    }
  };
  template <typename T>
  struct __build_size_type_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    __always_inline size_t operator()(T &t) { return sizeof(T) + t.data_len; }
  };

 public:
  template <typename T>
  static __always_inline size_t __build_size_type(T &t) {
    return __build_size_type_helper<T>()(t);
  }

  template <typename... Rest>
  static __always_inline size_t __build_size(Rest &...rest) {
    size_t sv[] = {__build_size_type(rest)...};
    size_t s = 0;
    for (size_t i = 0; i < sizeof...(Rest); ++i) s += sv[i];
    return s;
  }

 private:
  template <size_t N>
  struct __tuple_build_helper {
    template <typename... P>
    __always_inline void operator()(inner_string &buf, std::tuple<P...> &t) {
      __tuple_build_helper<N - 1>()(buf, t);
      __build_buf_type_helper<decltype(std::get<N - 1>(t))>()(
          buf, std::get<N - 1>(t));
    }
  };
  template <typename T, typename Enable = void>
  struct __build_buf_type_helper {
    template <typename _Tp>
    __always_inline void operator()(inner_string &buf, _Tp &t) {
      buf.append(reinterpret_cast<const char *>(&t), sizeof(_Tp));
    }
    __always_inline void operator()(inner_string &buf, std::string &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      buf.append(t);
    }
    template <typename _Tp>
    __always_inline void operator()(inner_string &buf, std::vector<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (size_t i = 0; i < t.size(); ++i) {
        __build_buf_type_helper<_Tp>()(buf, t[i]);
      }
    }
    template <typename K, typename V>
    __always_inline void operator()(inner_string &buf, std::map<K, V> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<K>()(buf, (K &)it.first);
        __build_buf_type_helper<V>()(buf, it.second);
      }
    }
    template <typename K, typename V>
    __always_inline void operator()(inner_string &buf,
                                    std::unordered_map<K, V> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<K>()(buf, (K &)it.first);
        __build_buf_type_helper<V>()(buf, it.second);
      }
    }
    template <typename _Tp>
    __always_inline void operator()(inner_string &buf, std::list<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename _Tp>
    __always_inline void operator()(inner_string &buf, std::set<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename _Tp>
    __always_inline void operator()(inner_string &buf,
                                    std::unordered_set<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename P1, typename P2>
    __always_inline void operator()(inner_string &buf, std::pair<P1, P2> &t) {
      __build_buf_type_helper<P1>()(buf, t.first);
      __build_buf_type_helper<P2>()(buf, t.second);
    }
    template <typename... P>
    __always_inline void operator()(inner_string &buf, std::tuple<P...> &t) {
      __tuple_build_helper<sizeof...(P)>()(buf, t);
    }
  };

  template <typename T>
  struct __build_buf_type_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    __always_inline void operator()(inner_string &buf, T &t) {
      buf.append(reinterpret_cast<const char *>(&t), sizeof(T) + t.data_len);
    }
  };

 public:
  template <typename T>
  static __always_inline uint8_t __build_buf_type(inner_string &buf, T &t) {
    __build_buf_type_helper<T>()(buf, t);
    return 0;
  }

  template <typename... Rest>
  static __always_inline void __build_buf(inner_string &buf, Rest &...rest) {
    // dummy array
    // exec order: left to right
    uint8_t _[] = {__build_buf_type(buf, rest)...};
  }

 private:
  template <size_t N>
  struct __tuple_parse_helper {
    template <typename... P>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::tuple<P...> *t) {
      size_t s = __tuple_parse_helper<N - 1>()(buf, i, t);
      return s + __parse_buf_helper<typename std::remove_reference<
                     decltype(std::get<N - 1>(*t))>::type>()(
                     buf, i + s, &std::get<N - 1>(*t));
    }
  };
  template <typename T, typename Enable = void>
  struct __parse_buf_helper {
    template <typename _Tp>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      _Tp *t) {
      if (buf.size() < i + sizeof(_Tp)) throw "parse type error";
      *t = *reinterpret_cast<const _Tp *>(buf.data() + i);
      return sizeof(_Tp);
    }
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::string *t) {
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      if (buf.size() < i + s + sizeof(uint32_t)) throw "parse string error";
      t->assign(buf.data() + i + sizeof(uint32_t), s);
      return s + sizeof(uint32_t);
    }
    template <typename _Tp>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::vector<_Tp> *t) {
      size_t ss = sizeof(uint32_t);
      if (buf.size() < i + sizeof(uint32_t)) throw "parse vector error";
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      t->resize(s);
      for (uint32_t _i = 0; _i < s; ++_i) {
        ss += __parse_buf_helper<_Tp>()(buf, i + ss, t->data() + _i);
      }
      return ss;
    }
    template <typename K, typename V>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::map<K, V> *t) {
      size_t ss = sizeof(uint32_t);
      if (buf.size() < i + sizeof(uint32_t)) throw "parse map error";
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      for (uint32_t _i = 0; _i < s; ++_i) {
        K k;
        V v;
        ss += __parse_buf_helper<K>()(buf, i + ss, &k);
        ss += __parse_buf_helper<V>()(buf, i + ss, &v);
        if (!t->emplace(k, v).second) throw "parse map error";
      }
      return ss;
    }
    template <typename _Tp>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::list<_Tp> *t) {
      size_t ss = sizeof(uint32_t);
      if (buf.size() < i + sizeof(uint32_t)) throw "parse list error";
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      for (uint32_t _i = 0; _i < s; ++_i) {
        _Tp e;
        ss += __parse_buf_helper<_Tp>()(buf, i + ss, &e);
        t->emplace_back(e);
      }
      return ss;
    }
    template <typename _Tp>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::set<_Tp> *t) {
      size_t ss = sizeof(uint32_t);
      if (buf.size() < i + sizeof(uint32_t)) throw "parse set error";
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      for (uint32_t _i = 0; _i < s; ++_i) {
        _Tp e;
        ss += __parse_buf_helper<_Tp>()(buf, i + ss, &e);
        if (!t->emplace(e).second) throw "parse set error";
      }
      return ss;
    }
    template <typename _Tp>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::unordered_set<_Tp> *t) {
      size_t ss = sizeof(uint32_t);
      if (buf.size() < i + sizeof(uint32_t)) throw "parse unordered_set error";
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      for (uint32_t _i = 0; _i < s; ++_i) {
        _Tp e;
        ss += __parse_buf_helper<_Tp>()(buf, i + ss, &e);
        if (!t->emplace(e).second) throw "parse unordered_set error";
      }
      return ss;
    }
    template <typename P1, typename P2>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::pair<P1, P2> *t) {
      size_t p1s = __parse_buf_helper<P1>()(buf, i, &t->first);
      size_t p2s = __parse_buf_helper<P2>()(buf, i + p1s, &t->second);
      return p1s + p2s;
    }
    template <typename... P>
    __always_inline size_t operator()(inner_string &buf, const size_t i,
                                      std::tuple<P...> *t) {
      return __tuple_parse_helper<sizeof...(P)>()(buf, i, t);
    }
  };

  template <typename T>
  struct __parse_buf_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    __always_inline size_t operator()(inner_string &buf, const size_t i, T *t) {
      // offsetof will cause warning
      if (buf.size() <
          i + sizeof(uint32_t) + ((uintptr_t)&t->data_len - (uintptr_t)t))
        throw "parse burst_struct error";
      uint32_t data_len = reinterpret_cast<const T *>(buf.data() + i)->data_len;
      if (buf.size() < i + sizeof(T) + data_len)
        throw "parse burst_struct error";
      memcpy(t, buf.data() + i, sizeof(T) + data_len);
      return sizeof(T) + data_len;
    }
  };

 public:
  template <typename T>
  static __always_inline uint8_t __parse_buf_type(inner_string &buf, size_t &i,
                                                  T *t) {
    size_t s = __parse_buf_helper<T>()(buf, i, t);
    i += s;
    return 0;
  }

  template <typename... Rest>
  static __always_inline void __parse_buf(inner_string &buf, const size_t i,
                                          Rest *...rest) {
    size_t _i = i;
    // dummy array
    // exec order: left to right
    uint8_t _[] = {__parse_buf_type(buf, _i, rest)...};
  }
};

template <>
struct helper::__tuple_size_helper<0> {
  template <typename... P>
  __always_inline size_t operator()(std::tuple<P...> &t) {
    return 0;
  }
};
template <>
struct helper::__tuple_build_helper<0> {
  template <typename... P>
  __always_inline void operator()(inner_string &buf, std::tuple<P...> &t) {}
};
template <>
struct helper::__tuple_parse_helper<0> {
  template <typename... P>
  __always_inline size_t operator()(inner_string &buf, const size_t i,
                                    std::tuple<P...> *t) {
    return 0;
  }
};

const uint16_t magic_number = 0x5c29;

/**
 * Get sitobuf size according to parameters.
 * Use this to alloc memory area for buffer.
 *
 * @param args... parameters to pack.
 *
 * @return sitobuf size
 */
template <typename... Args>
__always_inline size_t build_size(Args &&...args) {
  return sizeof(uint16_t) + sizeof(uint16_t) + helper::__build_size(args...);
}

/**
 * Build a sitobuf.
 * The number of parameters cannot over 65535.
 *
 * @param args... parameters to pack.
 *
 * @return a sitobuf string
 */
template <typename... Args>
std::string __always_inline build_buf(Args &&...args) {
  size_t buf_size = helper::__build_size(args...);
  uint16_t argc = sizeof...(Args);
  std::string buf;
  inner_string is(buf);
  buf.reserve(buf_size + sizeof(uint32_t));
  is.append(reinterpret_cast<const char *>(&sitobuf::magic_number),
            sizeof(uint16_t));
  is.append(reinterpret_cast<char *>(&argc), sizeof(uint16_t));
  helper::__build_buf(is, args...);
  return buf;
}

/**
 * Build a sitobuf to user-defined memory area.
 * The number of parameters cannot over 65535.
 *
 * @param buf a memory area structure.
 * @param args... parameters to pack.
 *
 * @return Reference to this buf.
 */
template <typename... Args>
__always_inline inner_string &build_buf(inner_string &buf, Args &&...args) {
  size_t buf_size = helper::__build_size(args...);
  if (buf_size > buf.capacity()) throw "buffer size is too small";
  uint16_t argc = sizeof...(Args);
  buf.resize(0);
  buf.append(reinterpret_cast<const char *>(&sitobuf::magic_number),
             sizeof(uint16_t));
  buf.append(reinterpret_cast<char *>(&argc), sizeof(uint16_t));
  helper::__build_buf(buf, args...);
  return buf;
}

/**
 * Parse a sitobuf.
 *
 * @param args... parameter pointers to unpack. The number and order of
 * parameter types must be same as calling sitobuf::build_buf()
 */
template <typename... Args>
__always_inline void parse_buf(inner_string &buf, Args *...args) {
  inner_string is(buf);
  if (*reinterpret_cast<const uint16_t *>(is.data()) != sitobuf::magic_number)
    throw "parse error: Unrecognized sitobuf";
  uint16_t buf_argc =
      *reinterpret_cast<const uint16_t *>(is.data() + sizeof(uint16_t));
  if (buf_argc != sizeof...(Args))
    throw "parse error: Incorrect number of parameters";
  helper::__parse_buf(is, sizeof(uint16_t) + sizeof(uint16_t), args...);
}

/**
 * Concat two sitobuf.
 * The sum of paramter number of two sitobufs cannot over 65535.
 *
 * @param buf1 first sitobuf
 * @param buf2 next sitobuf
 *
 * @return The result store in buf1
 */
__always_inline void concat_buf(inner_string &buf1, inner_string &buf2) {
  if (*reinterpret_cast<const uint16_t *>(buf1.data()) !=
          sitobuf::magic_number ||
      *reinterpret_cast<const uint16_t *>(buf2.data()) != sitobuf::magic_number)
    throw "concat error: Unrecognized sitobuf";
  uint16_t buf1_argc = *reinterpret_cast<const uint16_t *>(buf1.data() +
                                                           sizeof(uint16_t)),
           buf2_argc = *reinterpret_cast<const uint16_t *>(buf2.data() +
                                                           sizeof(uint16_t));
  uint16_t new_buf_argc = buf1_argc + buf2_argc;
  if (new_buf_argc < buf1_argc) throw "concat error: Too much arguments";
  buf1.append(buf2, sizeof(uint16_t) + sizeof(uint16_t));
  buf1.replace(sizeof(uint16_t), sizeof(uint16_t),
               reinterpret_cast<char *>(&new_buf_argc), sizeof(uint16_t));
}

/**
 * Append parameters to a sitobuf.
 *
 * @param buf an exist sitobuf
 * @param args... parameters to pack.
 *
 * @return The result store in buf
 */
template <typename... Args>
__always_inline void append_buf(inner_string &buf, Args &&...args) {
  uint16_t new_buf_argc = sizeof...(Args) + *reinterpret_cast<const uint16_t *>(
                                                buf.data() + sizeof(uint16_t));
  if (new_buf_argc < sizeof...(Args)) throw "append error: Too much arguments";
  size_t append_buf_size = helper::__build_size(args...);
  buf.reserve(buf.size() + append_buf_size);
  buf.replace(sizeof(uint16_t), sizeof(uint16_t),
              reinterpret_cast<char *>(&new_buf_argc), sizeof(uint16_t));
  helper::__build_buf(buf, args...);
}
}  // namespace sitobuf

#endif  // __SITO_BUF_H__