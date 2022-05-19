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

struct burst_struct {
  uint32_t data_len;
};

class helper {
 private:
  template <size_t N>
  struct __tuple_size_helper {
    template <typename... P>
    size_t __always_inline operator()(std::tuple<P...> &t) {
      return __build_size_type_helper<decltype(std::get<N - 1>(t))>()(
                 std::get<N - 1>(t)) +
             __tuple_size_helper<N - 1>()(t);
    }
  };
  template <typename T, typename Enable = void>
  struct __build_size_type_helper {
    template <typename _Tp>
    constexpr size_t __always_inline operator()(_Tp &t) {
      return sizeof(_Tp);
    }
    size_t __always_inline operator()(std::string &t) {
      return sizeof(uint32_t) + t.size();
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::vector<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (size_t i = 0; i < t.size(); ++i) {
        s += __build_size_type_helper<_Tp>()(t[i]);
      }
      return s;
    }
    template <typename K, typename V>
    size_t __always_inline operator()(std::map<K, V> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<K>()((K &)it.first);
        s += __build_size_type_helper<V>()(it.second);
      }
      return s;
    }
    template <typename K, typename V>
    size_t __always_inline operator()(std::unordered_map<K, V> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<K>()((K &)it.first);
        s += __build_size_type_helper<V>()(it.second);
      }
      return s;
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::list<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::set<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::unordered_set<_Tp> &t) {
      size_t s = sizeof(uint32_t);
      for (auto it : t) {
        s += __build_size_type_helper<_Tp>()(it);
      }
      return s;
    }
    template <typename P1, typename P2>
    size_t __always_inline operator()(std::pair<P1, P2> &t) {
      return __build_size_type_helper<P1>()(t.first) +
             __build_size_type_helper<P2>()(t.second);
    }
    template <typename... P>
    size_t __always_inline operator()(std::tuple<P...> &t) {
      return __tuple_size_helper<sizeof...(P)>()(t);
    }
  };
  template <typename T>
  struct __build_size_type_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    size_t __always_inline operator()(T &t) { return sizeof(T) + t.data_len; }
  };

 public:
  template <typename T>
  static size_t __always_inline __build_size(T &t) {
    return __build_size_type_helper<T>()(t);
  }

  template <typename T, typename... Rest>
  static size_t __always_inline __build_size(T &t, Rest &...rest) {
    return __build_size_type_helper<T>()(t) + __build_size(rest...);
  }

 private:
  template <size_t N>
  struct __tuple_build_helper {
    template <typename... P>
    void __always_inline operator()(std::string &buf, std::tuple<P...> &t) {
      __tuple_build_helper<N - 1>()(buf, t);
      __build_buf_type_helper<decltype(std::get<N - 1>(t))>()(
          buf, std::get<N - 1>(t));
    }
  };
  template <typename T, typename Enable = void>
  struct __build_buf_type_helper {
    template <typename _Tp>
    void __always_inline operator()(std::string &buf, _Tp &t) {
      buf.append(reinterpret_cast<const char *>(&t), sizeof(_Tp));
    }
    void __always_inline operator()(std::string &buf, std::string &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      buf.append(t);
    }
    template <typename _Tp>
    void __always_inline operator()(std::string &buf, std::vector<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (size_t i = 0; i < t.size(); ++i) {
        __build_buf_type_helper<_Tp>()(buf, t[i]);
      }
    }
    template <typename K, typename V>
    void __always_inline operator()(std::string &buf, std::map<K, V> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<K>()(buf, (K &)it.first);
        __build_buf_type_helper<V>()(buf, it.second);
      }
    }
    template <typename K, typename V>
    void __always_inline operator()(std::string &buf,
                                    std::unordered_map<K, V> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<K>()(buf, (K &)it.first);
        __build_buf_type_helper<V>()(buf, it.second);
      }
    }
    template <typename _Tp>
    void __always_inline operator()(std::string &buf, std::list<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename _Tp>
    void __always_inline operator()(std::string &buf, std::set<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename _Tp>
    void __always_inline operator()(std::string &buf,
                                    std::unordered_set<_Tp> &t) {
      uint32_t s = (uint32_t)t.size();
      buf.append(reinterpret_cast<char *>(&s), sizeof(uint32_t));
      for (auto it : t) {
        __build_buf_type_helper<_Tp>()(buf, it);
      }
    }
    template <typename P1, typename P2>
    void __always_inline operator()(std::string &buf, std::pair<P1, P2> &t) {
      __build_buf_type_helper<P1>()(buf, t.first);
      __build_buf_type_helper<P2>()(buf, t.second);
    }
    template <typename... P>
    void __always_inline operator()(std::string &buf, std::tuple<P...> &t) {
      __tuple_build_helper<sizeof...(P)>()(buf, t);
    }
  };

  template <typename T>
  struct __build_buf_type_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    void __always_inline operator()(std::string &buf, T &t) {
      buf.append(reinterpret_cast<const char *>(&t), sizeof(T) + t.data_len);
    }
  };

 public:
  template <typename T>
  static void __always_inline __build_buf(std::string &buf, T &t) {
    __build_buf_type_helper<T>()(buf, t);
  }

  template <typename T, typename... Rest>
  static void __always_inline __build_buf(std::string &buf, T &t,
                                          Rest &...rest) {
    __build_buf_type_helper<T>()(buf, t);
    __build_buf(buf, rest...);
  }

 private:
  template <size_t N>
  struct __tuple_parse_helper {
    template <typename... P>
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      _Tp *t) {
      if (buf.size() < i + sizeof(_Tp)) throw "parse type error";
      *t = *reinterpret_cast<const _Tp *>(buf.data() + i);
      return sizeof(_Tp);
    }
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      std::string *t) {
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      if (buf.size() < i + s + sizeof(uint32_t)) throw "parse string error";
      t->assign(buf.data() + i + sizeof(uint32_t), s);
      return s + sizeof(uint32_t);
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
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
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      std::pair<P1, P2> *t) {
      size_t p1s = __parse_buf_helper<P1>()(buf, i, &t->first);
      size_t p2s = __parse_buf_helper<P2>()(buf, i + p1s, &t->second);
      return p1s + p2s;
    }
    template <typename... P>
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      std::tuple<P...> *t) {
      return __tuple_parse_helper<sizeof...(P)>()(buf, i, t);
    }
  };

  template <typename T>
  struct __parse_buf_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    size_t __always_inline operator()(std::string &buf, const size_t i, T *t) {
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
  static void __always_inline __parse_buf(std::string &buf, const size_t i,
                                          T *t) {
    __parse_buf_helper<T>()(buf, i, t);
  }

  template <typename T, typename... Rest>
  static void __always_inline __parse_buf(std::string &buf, const size_t i,
                                          T *t, Rest *...rest) {
    size_t s = __parse_buf_helper<T>()(buf, i, t);
    __parse_buf(buf, i + s, rest...);
  }
};

template <>
struct helper::__tuple_size_helper<0> {
  template <typename... P>
  size_t __always_inline operator()(std::tuple<P...> &t) {
    return 0;
  }
};
template <>
struct helper::__tuple_build_helper<0> {
  template <typename... P>
  void __always_inline operator()(std::string &buf, std::tuple<P...> &t) {}
};
template <>
struct helper::__tuple_parse_helper<0> {
  template <typename... P>
  size_t __always_inline operator()(std::string &buf, const size_t i,
                                    std::tuple<P...> *t) {
    return 0;
  }
};

const uint16_t magic_number = 0x5c29;

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
  buf.reserve(buf_size + sizeof(uint32_t));
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
void __always_inline parse_buf(std::string &buf, Args *...args) {
  if (*reinterpret_cast<const uint16_t *>(buf.data()) != sitobuf::magic_number)
    throw "parse error: Unrecognized sitobuf";
  uint16_t buf_argc =
      *reinterpret_cast<const uint16_t *>(buf.data() + sizeof(uint16_t));
  if (buf_argc != sizeof...(Args))
    throw "parse error: Incorrect number of parameters";
  helper::__parse_buf(buf, sizeof(uint16_t) + sizeof(uint16_t), args...);
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
void __always_inline concat_buf(std::string &buf1, std::string &buf2) {
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
void __always_inline append_buf(std::string &buf, Args &&...args) {
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