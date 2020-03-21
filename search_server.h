#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <future>

using namespace std;

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
    void UpdateDocumentBaseThread(istream& document_input);
    void UpdateDocumentBase(istream& document_input);
    void ServQueriesThread(istream& query_input, ostream& search_results_output);
    void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
    array<future<void>, 4> futures;
    shared_ptr<InvertedIndex> index;
};


