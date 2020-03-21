#pragma once

#include <istream>
#include <ostream>
#include <vector>
#include <map>
#include <string>
#include <memory>

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
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  shared_ptr<InvertedIndex> index;
};
