#include "search_server.h"
#include "iterator_range.h"
#include "parse.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <future>

const size_t FILL_THREAD_CAPACITY = 1'000;
const size_t SEARCH_THREAD_CAPACITY = 2'000;

void PushDocs(IndexedDocs & index, vector<string>&& str, size_t pos) {
    for (auto & s : str)
        index.Add(move(s), pos++);
}

vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
  IndexedDocs new_index;

  vector<string> docs;
  docs.reserve(FILL_THREAD_CAPACITY);
  vector<future<void>> futures;
  for (string current_document; getline(document_input, current_document); ) {
      if (docs.size() < FILL_THREAD_CAPACITY)
          docs.push_back(move(current_document));
      else {
          futures.push_back(async(PushDocs, ref(new_index), move(docs), FILL_THREAD_CAPACITY * futures.size()));
          docs.reserve(FILL_THREAD_CAPACITY);
      }
  }
  futures.push_back(async(PushDocs, ref(new_index), move(docs), FILL_THREAD_CAPACITY * futures.size()));
  for(auto & f : futures)
      f.get();

  index = move(new_index);
}

void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {
  for (string current_query; getline(query_input, current_query); ) {
    auto words = SplitBy(current_query, ' ');
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    auto search_results = index.GetRelevationDocs(words);
    search_results_output << current_query << ':';
    for (auto [hitcount, docid] : Head(search_results, 5)) {
      search_results_output << " {"
        << "docid: " << docid << ", "
        << "hitcount: " << hitcount << '}';
    }
    search_results_output << endl;
  }
}


