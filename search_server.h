#pragma once

#include <istream>
#include <ostream>
#include <indexed_docs.h>

const size_t FILL_THREAD_CAPACITY = 2'000;
const size_t SEARCH_THREAD_CAPACITY = 500;

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  IndexedDocs index;
};
