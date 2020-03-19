#include "search_server.h"
#include "iterator_range.h"
#include "parse.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <future>

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

using Request = string;
using AnswerForRequest = vector<pair<IndexedDocs::Relevation, size_t>>;
using ThreadResult = vector<pair<Request, AnswerForRequest>>;

ThreadResult AddSearchThread(IndexedDocs & id, vector<string> requests) {
    ThreadResult result;
    for (string & request : requests) {
        auto words = SplitBy(request, ' ');
        sort(words.begin(), words.end());
        words.erase(unique(words.begin(), words.end()), words.end());
        auto search_result = id.GetRelevationDocs(words);
        auto head_search_result = Head(search_result, 5);
        result.emplace_back(move(request), AnswerForRequest(head_search_result.begin(), head_search_result.end()));
    }
    return result;
}


void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {
    vector<string> requests;
    requests.reserve(SEARCH_THREAD_CAPACITY);
    vector<future<ThreadResult>> futures;
    for (string current_query; getline(query_input, current_query); ) {
        if (requests.size() < SEARCH_THREAD_CAPACITY)
            requests.emplace_back(move(current_query));
        else {
            futures.push_back(async(AddSearchThread, ref(index), move(requests)));
            requests.reserve(SEARCH_THREAD_CAPACITY);
        }
    }
    futures.push_back(async(AddSearchThread, ref(index), move(requests)));
    for (auto & f : futures) {
        for (auto & [request, answer] : f.get()) {
            search_results_output << request << ':';
            for (auto [hitcount, docid] : answer) {
              search_results_output << " {"
                << "docid: " << docid << ", "
                << "hitcount: " << hitcount << '}';
            }
            search_results_output << endl;
        }
    }
}


