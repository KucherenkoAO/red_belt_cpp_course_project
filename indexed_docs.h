#pragma once
#include <set>
#include <vector>
#include <map>
#include <string>
#include <deque>

using namespace std;

class IndexedDocs {
public:
  using Relevation = size_t;
  using DocId = size_t;

  void Add(const string& document);
  vector<pair<size_t, size_t>> GetRelevationDocs(const vector<string_view> words);
  const string& GetDocument(size_t id) const {
    return docs[id].doc;
  }

private:
  struct Doc_t {
      const string doc;
      map<string_view, size_t> words_in_doc;
      Doc_t(string str);
      Doc_t(Doc_t && d);
      size_t CalculateRelevation(const vector<string_view>& words) const;
  };

  map<string_view, set<size_t>> docs_with_word;
  set<size_t> zero_set;
  deque<Doc_t> docs;
  const set<size_t> & DocWithWord(string_view word) const;
};
