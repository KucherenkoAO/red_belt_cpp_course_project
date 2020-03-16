#pragma once

#include <istream>
#include <ostream>
#include <indexed_docs.h>

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  IndexedDocs index;
};
