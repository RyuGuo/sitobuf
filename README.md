# sitobuf

## A simple protobuf for C++

Sitobuf is a **pure C++ header file** and is very simple to use. It can simply package multiple variables into one continuous binary data. This buffer stores binary data, which can be sent to other host over the network or stored to disk.

In particular, sitobuf supports build and parse for string, vector and their nesting (e.g., two-dimensional vector), and other common STL containers (e.g., pair and map).

## How to use it?

Call `sitobuf::build_buf()` and get a string buffer. The function supports an indefinite number of parameters. This function packs the parameters in order. Get the packed size by `string::size`.

```c++
#include "sitobuf.h"

int a, b;
float f, g;
vector<int> v = {0,1,2};
string s = "hello world!";
vecotr<vector<int>> vv = {{0,1,2},{0,1,2}};

string buf = sitobuf::build_buf(a, b, f, g, v, vv, s); // class ref
```

Call `sitobuf::parse_buf()` and get prototypes. The parsed value is stored in the space pointed to by the pointer of the incoming variable. **The return value parameter order needs to be the same as when calling `build_buf`**, otherwise it will cause an error.

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

A *b = (A*)malloc(sizeof(A) + 12);    // must allocate more memory for burst array
sitobuf::parse_buf(buf, b);
```

See file `example.cc` for more usage details.

## Restrictions

- Sitobuf cannot package complex member types in struct/class, such as STL, due to **packing is a shallow copy**. You can try packing it's members as separate parameters.

```c++
struct {int n, vector<int> v;} s;
// sitobuf::build_buf(s);          // ERROR
sitobuf::build_buf(s.n, s.v);      // OK
```

- Sitobuf cannot package pointer types. Cast to `uintptr_t` if necessary.
