#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>


vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
  auto new_index = make_shared<InvertedIndex>();

  for (string current_document; getline(document_input, current_document); ) {
    new_index->Add(move(current_document));
  }

  index = new_index;
}

void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {
  for (string current_query; getline(query_input, current_query); ) {

      map<string, size_t> words;
      for (auto & word : SplitIntoWords(current_query))
          ++words[move(word)];

    vector<size_t> docid_count(index->GetDocsCount());
    for (const auto& [word, count_word] : words) {
      for (const size_t docid : index->Lookup(word)) {
        docid_count[docid] += count_word;
      }
    }

    vector<pair<size_t, size_t>> search_results;
    search_results.reserve(docid_count.size());
    for (size_t i = 0; i < docid_count.size(); ++i) {
        if (docid_count[i])
        search_results.emplace_back(i, docid_count[i]);
    }
    partial_sort(
      begin(search_results),
      min(end(search_results), begin(search_results) + 5),
      end(search_results),
      [](pair<size_t, size_t> lhs, pair<size_t, size_t> rhs) {
        int64_t lhs_docid = lhs.first;
        auto lhs_hit_count = lhs.second;
        int64_t rhs_docid = rhs.first;
        auto rhs_hit_count = rhs.second;
        return make_pair(lhs_hit_count, -lhs_docid) > make_pair(rhs_hit_count, -rhs_docid);
      }
    );

    search_results_output << current_query << ':';
    for (auto [docid, hitcount] : Head(search_results, 5)) {
      search_results_output << " {"
        << "docid: " << docid << ", "
        << "hitcount: " << hitcount << '}';
    }
    search_results_output << endl;
  }
}

void InvertedIndex::Add(string document) {
  docs.push_back(move(document));

  const size_t docid = docs.size() - 1;
  for (auto& word : SplitIntoWords(docs[docid])) {
    index[move(word)].push_back(docid);
  }
}

const vector<size_t> & InvertedIndex::Lookup(const string& word) const {
  if (auto it = index.find(word); it != index.end()) {
    return it->second;
  } else {
    return empty_vector;
  }
}
