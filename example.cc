#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sitobuf.h"

using namespace std;
using namespace sitobuf;

int main() {
  string buf;
  {
    struct {
      int a;
      int b;
    } a;
    a.a = 90;
    a.b = 32;
    string s = "333";
    vector<int> v = {7, 8, 9, 1};
    vector<string> sv = {"4444", "55555"};
    vector<vector<int>> vv = {{11, 22, 33}, {22, 33, 44}};
    struct : public burst_struct {
      int a;
      int b;
      char d[0];
    } *b = (typeof(b))malloc(sizeof(*b) + 6);
    b->a = 11;
    b->b = 33;
    b->d[0] = '1';
    b->d[1] = '2';
    b->d[2] = '3';
    b->d[3] = '4';
    b->d[4] = '5';
    b->d[5] = '\0';
    b->data_len = 6;
    tuple<int, double, string, vector<int>> t = {9, 1.5, "333", {1, 2, 3}};
    pair<int, string> p = {1, "123"};
    map<int, string> m = {{1, "2"}, {2, "4"}};
    unordered_set<string> ss = {"123", "456"};
    list<int> l = {3, 4};
    map<string, vector<tuple<int, list<vector<string>>>>> tuf = {
        {"1",
         {{1, {{"1", "11"}, {"11", "111"}}},
          {2, {{"2", "22"}, {"22", "222"}}}}},
        {"3",
         {{3, {{"3", "33"}, {"33", "333"}}},
          {4, {{"4", "44"}, {"44", "444"}}}}}};
    buf = build_buf(4, 7.0, a, s, v, sv, vv, *b, vector<int>{1, 2, 3}, t, p, m,
                    ss, l, tuf);
    cout << "buf size: " << buf.size() << endl;
    for (size_t i = 0; i < buf.size(); i++) {
      printf("%02x ", (unsigned char)buf[i]);
    }
    cout << endl;
    cout << endl;
  }
  if (1) {
    int f4;
    double f7;
    struct {
      int a;
      int b;
    } fa;
    string fs;
    vector<int> fv;
    vector<string> fsv;
    vector<vector<int>> fvv;
    struct : public burst_struct {
      int a;
      int b;
      char d[0];
    } *fb = (decltype(fb))malloc(sizeof(*fb) + 6);
    vector<int> fanv;
    tuple<int, double, string, vector<int>> ft;
    pair<int, string> fp;
    map<int, string> fm;
    unordered_set<string> fss;
    list<int> fl;
    map<string, vector<tuple<int, list<vector<string>>>>> ftuf;
    parse_buf(buf, &f4, &f7, &fa, &fs, &fv, &fsv, &fvv, fb, &fanv, &ft, &fp,
              &fm, &fss, &fl, &ftuf);
#define PVAL(name) cout << #name ": " << name << endl
    PVAL(f4);
    PVAL(f7);
    PVAL(fa.a);
    PVAL(fa.b);
    PVAL(fs);
    PVAL(fv[0]);
    PVAL(fv[1]);
    PVAL(fv[2]);
    PVAL(fv[3]);
    PVAL(fsv[0]);
    PVAL(fsv[1]);
    PVAL(fvv[0][0]);
    PVAL(fvv[0][1]);
    PVAL(fvv[0][2]);
    PVAL(fvv[1][0]);
    PVAL(fvv[1][1]);
    PVAL(fvv[1][2]);
    PVAL(fb->a);
    PVAL(fb->b);
    PVAL(fb->data_len);
    PVAL(fb->d[0]);
    PVAL(fb->d[1]);
    PVAL(fb->d[2]);
    PVAL(fb->d[3]);
    PVAL(fb->d[4]);
    PVAL(fb->d[5]);
    PVAL(fanv[0]);
    PVAL(fanv[1]);
    PVAL(fanv[2]);
    PVAL(std::get<0>(ft));
    PVAL(std::get<1>(ft));
    PVAL(std::get<2>(ft));
    PVAL(std::get<3>(ft)[0]);
    PVAL(std::get<3>(ft)[1]);
    PVAL(std::get<3>(ft)[2]);
    PVAL(fp.first);
    PVAL(fp.second);
    PVAL(fm[1]);
    PVAL(fm[2]);
    PVAL(*fss.begin());
    PVAL(*(++fss.begin()));
    PVAL(*fl.begin());
    PVAL(*(++fl.begin()));
    PVAL(std::get<0>(ftuf["1"][0]));
    PVAL(std::get<1>(ftuf["1"][0]).begin()->at(0));
    PVAL(std::get<1>(ftuf["1"][0]).begin()->at(1));
    PVAL((++std::get<1>(ftuf["1"][0]).begin())->at(0));
    PVAL((++std::get<1>(ftuf["1"][0]).begin())->at(1));
    PVAL(std::get<0>(ftuf["1"][1]));
    PVAL(std::get<1>(ftuf["1"][1]).begin()->at(0));
    PVAL(std::get<1>(ftuf["1"][1]).begin()->at(1));
    PVAL((++std::get<1>(ftuf["1"][1]).begin())->at(0));
    PVAL((++std::get<1>(ftuf["1"][1]).begin())->at(1));
    PVAL(std::get<0>(ftuf["3"][0]));
    PVAL(std::get<1>(ftuf["3"][0]).begin()->at(0));
    PVAL(std::get<1>(ftuf["3"][0]).begin()->at(1));
    PVAL((++std::get<1>(ftuf["3"][0]).begin())->at(0));
    PVAL((++std::get<1>(ftuf["3"][0]).begin())->at(1));
    PVAL(std::get<0>(ftuf["3"][1]));
    PVAL(std::get<1>(ftuf["3"][1]).begin()->at(0));
    PVAL(std::get<1>(ftuf["3"][1]).begin()->at(1));
    PVAL((++std::get<1>(ftuf["3"][1]).begin())->at(0));
    PVAL((++std::get<1>(ftuf["3"][1]).begin())->at(1));
  }
  return 0;
}