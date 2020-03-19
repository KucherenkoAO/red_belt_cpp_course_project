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
        ch.GetAccess().ref_to_value.reserve(100'000 / CHUNKS_COUNT + 1);
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
      docs_with_word[word].insert({cid, pos_in_chunk});
}

const set<IndexedDocs::DocId> & IndexedDocs::DocWithWord(string_view word) const {
    if (auto it = docs_with_word.find(word); it != docs_with_word.end())
        return it->second;
    else
        return zero_set;
}

vector<pair<IndexedDocs::Relevation, size_t>> IndexedDocs::GetRelevationDocs(const vector<string_view> & words) const {
    set<DocId> id_interesting_docs;
    for (auto word : words) {
        for (auto id : DocWithWord(word)) {
            id_interesting_docs.insert(id);
        }
    }

    auto cmp = [](pair<Relevation, size_t> lhs, pair<Relevation, size_t> rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;
        else
            return lhs.second > rhs.second;
    };

    set<pair<Relevation, size_t>, decltype (cmp)> relevation_docs(cmp);

    for (auto id : id_interesting_docs) {
        auto & doc = chunks[id.first].GetAccess().ref_to_value[id.second];
        relevation_docs.insert(
        {doc.CalculateRelevation(words),
         doc.doc_num});
    }
    return {make_move_iterator(relevation_docs.rbegin()), make_move_iterator(relevation_docs.rend())};
}
