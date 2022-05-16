# sitobuf

## A simple protobuf for C++

Sitobuf is a **pure C++ header file** and is very simple to use. It can simply package multiple variables into one continuous binary data. This buffer stores binary data, which can be sent to other host over the network or stored to disk.

In particular, sitobuf supports build and parse for vector and their nesting (e.g., two-dimensional vector) and string.

## How to use it?

Call `sitobuf::build_buf()` and get a string buf. The function supports an indefinite number of parameters. This function packs the parameters in order. Get the packed size by `string::size`.

```c++
#include "sitobuf.h"

int a, b;
float f, g;
vector<int> v = {0,1,2};
string s = "hello world!";
vecotr<vector<int>> vv = {{0,1,2},{0,1,2}};

string buf = sitobuf::build_buf(a, b, f, g, v, vv, s); // class ref
```

Call `sitobuf::parse_buf()` and get prototypes. The parsed value is stored in the space pointed to by the pointer of the incoming variable. The return value parameter order needs to be the same as when build_buf, otherwise it will cause an error.

```c++
sitobuf::parse_buf(buf, &a, &b, &f, &g, &v, &vv, &s); // class pointer
```

Moreover, sitobuf supports packaging of classes that have burst array members, such as:

```c++
struct A : public sitobuf::burst_struct {
  int a;
  char data[0];     // a burst array
};

A *a = (A*)malloc(sizeof(A) + 12);
memcpy(a.data, "hello world!", 12);
// burst_strcut::data_len, store the length of burst array data
a->data_len = 12;
string buf = sitobuf::build_buf(*a);

A *b = (A*)malloc(sizeof(A) + 12);
sitobuf::parse_buf(buf, b);
```

## Restrictions

- Sitobuf cannot package complex member types in struct/class, such as STL and other struct/class members. You can try packing as a separate parameter.

  ```c++
  struct {vector<int> v;} s;
  sitobuf::build_buf(s.v);      // OK
  // sitobuf::build_buf(s);     // ERROR

  pair<int,string> kv;
  sitobuf::build_buf(kv.first, kv.second);    // OK
  // sitobuf::build_buf(kv);                  // ERROR
  ```

- Sitobuf cannot package pointer types. Cast to `uintptr_t` if necessary.

- Sitobuf cannot pack STL containers other than vector, use vector transit if necessary.

  ```c++
  map<int, string> kvs;

  // sitobuf::build_buf(kvs);   // ERROR

  vector<int> ks;
  vector<string> vs;
  for (auto &kv : map) { 
    ks.push_back(kv.first);
    vs.push_back(kv.second);
  }
  sitobuf::build_buf(ks, vs);   // OK
  ```