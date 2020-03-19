#pragma once
#include <set>
#include <vector>
#include <map>
#include <string>
#include <deque>
#include <mutex>

using namespace std;

template <typename T>
class Synchronized {
public:
  explicit Synchronized(T initial = T())
    : value(move(initial))
  {
  }

  struct Access {
    T& ref_to_value;
    lock_guard<mutex> guard;
  };

  Access GetAccess() {
    return {value, lock_guard(m)};
  }

private:
  T value;
  mutex m;
};

struct Doc_t {
    const string doc;
    size_t doc_num;
    map<string_view, size_t> words_in_doc;
    Doc_t(string str, size_t num);
    Doc_t(Doc_t && d);
    void CountWordsInDoc();
    size_t CalculateRelevation(const vector<string_view>& words) const;
};


class IndexedDocs {
public:
    using Relevation = size_t;
    using ChunkId = size_t;
    using DocId = pair<ChunkId, size_t>;
    IndexedDocs();
    IndexedDocs & operator = (IndexedDocs && id);
  void Add(string document, size_t num);
  vector<pair<Relevation, size_t>> GetRelevationDocs(const vector<string_view> & words) const;
  const string& GetDocument(DocId id) const {
    return chunks[id.first].GetAccess().ref_to_value[id.second].doc;
  }

private:
  size_t CHUNKS_COUNT = 50;
  using Chunk = vector<Doc_t>;
  mutable vector<Synchronized<Chunk>> chunks;
  map<string_view, set<DocId>> docs_with_word;
  mutex docs_with_word_mutex;
  set<DocId> zero_set;
  const set<DocId> & DocWithWord(string_view word) const;
};
