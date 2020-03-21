#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <future>
#include <deque>

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

class InvertedIndex {
public:
  void Add(string document);
  const vector<size_t> & Lookup(const string& word) const;

  const string& GetDocument(size_t id) const {
    return docs[id];
  }
  size_t GetDocsCount() const {
      return docs.size();
  }

private:
  map<string, vector<size_t>> index;
  vector<string> docs;
  vector<size_t> empty_vector;
};

class SearchServer {
public:
    SearchServer() = default;
    explicit SearchServer(istream& document_input);
    void UpdateDocumentBase(istream& document_input);
    void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
    void UpdateDocumentBaseThread(istream& document_input);
    void QueriesStreamThread(istream& query_input, ostream& search_results_output);
    Synchronized<shared_ptr<InvertedIndex>> index;
    bool is_first_db_update = true;
    Synchronized<deque<future<void>>> futures;
};
