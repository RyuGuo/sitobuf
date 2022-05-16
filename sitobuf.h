#ifndef __SITO_BUF_H__
#define __SITO_BUF_H__

#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace sitobuf {

struct burst_struct {
  uint32_t data_len;
};

class helper {
 private:
  template <typename T, typename Enable = void>
  struct __build_size_type_helper {
    template <typename _T>
    size_t __always_inline operator()(_T &t) {
      return sizeof(_T);
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
  template <typename T, typename Enable = void>
  struct __build_buf_type_helper {
    template <typename _T>
    void __always_inline operator()(std::string &buf, _T &t) {
      buf.append(reinterpret_cast<const char *>(&t), sizeof(T));
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
  template <typename T, typename Enable = void>
  struct __parse_buf_helper {
    template <typename _T>
    size_t __always_inline operator()(std::string &buf, const size_t i, _T *t) {
      *t = *reinterpret_cast<const T *>(buf.data() + i);
      return sizeof(T);
    }
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      std::string *t) {
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      t->assign(buf.data() + i + sizeof(uint32_t), s);
      return s + sizeof(uint32_t);
    }
    template <typename _Tp>
    size_t __always_inline operator()(std::string &buf, const size_t i,
                                      std::vector<_Tp> *t) {
      size_t ss = 0;
      uint32_t s = *reinterpret_cast<const uint32_t *>(buf.data() + i);
      t->resize(s);
      for (size_t _i = 0; _i < s; ++_i) {
        ss += __parse_buf_helper<_Tp>()(buf, i + ss + sizeof(uint32_t),
                                        t->data() + _i);
      }
      return ss + sizeof(uint32_t);
    }
  };

  template <typename T>
  struct __parse_buf_helper<
      T,
      typename std::enable_if<std::is_base_of<burst_struct, T>::value>::type> {
    size_t __always_inline operator()(std::string &buf, const size_t i, T *t) {
      uint32_t data_len = reinterpret_cast<const T *>(buf.data() + i)->data_len;
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

template <typename... Args>
std::string __always_inline build_buf(Args &&...args) {
  size_t buf_size = helper::__build_size(args...);
  uint32_t argc = sizeof...(Args);
  std::string buf;
  buf.reserve(buf_size + sizeof(uint32_t));
  buf.append(reinterpret_cast<char *>(&argc), sizeof(uint32_t));
  helper::__build_buf(buf, args...);
  return buf;
}

template <typename... Args>
void __always_inline parse_buf(std::string &buf, Args *...args) {
  uint32_t buf_argc = *reinterpret_cast<const uint32_t *>(buf.data());
  if (buf_argc != sizeof...(Args)) {
    throw "Incorrect number of parameters";
  }
  helper::__parse_buf(buf, sizeof(uint32_t), args...);
}
}  // namespace sitobuf

#endif  // __SITO_BUF_H__