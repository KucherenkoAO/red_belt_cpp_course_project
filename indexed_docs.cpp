#include "indexed_docs.h"
#include "parse.h"
#include "test_runner.h"
#include <memory>

// Doc_t methods
Doc_t::Doc_t(string str, size_t num) : doc(move(str)), doc_num(num) {}

Doc_t::Doc_t(Doc_t && d)
    : doc(move(d.doc)),
      doc_num(d.doc_num),
      words_in_doc(move(d.words_in_doc))
{}

void Doc_t::CountWordsInDoc() {
    auto words = SplitBy(doc, ' ');
    for (auto sv : words) {
        ++words_in_doc[sv];
    }
}

size_t Doc_t::CalculateRelevation(const vector<string_view>& words) const {
    size_t result = 0;
    for (auto word : words) {
        if (auto it = words_in_doc.find(word); it != words_in_doc.end()) {
            result += it->second;
        }
    }
    return result;
}

// IndexedDocs methods

IndexedDocs::IndexedDocs()
    : chunks(CHUNKS_COUNT)
{
    for (auto & ch : chunks)
        ch.GetAccess().ref_to_value.reserve(50'000 / CHUNKS_COUNT + 1);
}


IndexedDocs & IndexedDocs::operator = (IndexedDocs && id) {
    if (&id != this) {
        CHUNKS_COUNT = id.CHUNKS_COUNT;
        chunks = move(id.chunks);
        docs_with_word = move(id.docs_with_word);
    }
    return *this;
}


void IndexedDocs::Add(string document, size_t num) {
  ChunkId cid = num % CHUNKS_COUNT;
  auto & chunk = chunks[cid];
  Doc_t * doc;
  int pos_in_chunk;
  {
      auto synhro_chunk = chunk.GetAccess();
      synhro_chunk.ref_to_value.emplace_back(move(document), num);
      pos_in_chunk = synhro_chunk.ref_to_value.end() - synhro_chunk.ref_to_value.begin() - 1;
      doc = & synhro_chunk.ref_to_value.back();
  }
  doc->CountWordsInDoc();
  lock_guard lg(docs_with_word_mutex);
  for (const auto &[word, count] : doc->words_in_doc)
      docs_with_word[word].push_back({{cid, pos_in_chunk}, count});
}

const vector<pair<IndexedDocs::DocId, size_t>> & IndexedDocs::DocWithWord(string_view word) const {
    if (auto it = docs_with_word.find(word); it != docs_with_word.end())
        return it->second;
    else
        return zero_pair;
}

vector<pair<IndexedDocs::Relevation, size_t>> IndexedDocs::GetRelevationDocs(const vector<string_view> & words) const {
    map<DocId, size_t> id_interesting_docs;
    for (auto word : words) {
        for (auto [docid, count_word] : DocWithWord(word)) {
            id_interesting_docs[docid] += count_word;
        }
    }

    vector<pair<Relevation, size_t>> relevation_docs;
    for (auto [docid, relevation] : id_interesting_docs)
        relevation_docs.push_back({relevation, chunks[docid.first].GetAccess().ref_to_value[docid.second].doc_num});

    sort(relevation_docs.begin(), relevation_docs.end(), [](pair<Relevation, size_t> & lhs, pair<Relevation, size_t> & rhs) {
        if (lhs.first != rhs.first)
            return lhs.first > rhs.first;
        else
            return lhs.second < rhs.second;
    });

    return relevation_docs;
}
