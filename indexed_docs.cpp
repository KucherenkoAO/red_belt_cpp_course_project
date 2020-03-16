#include "indexed_docs.h"
#include "parse.h"

// Doc_t methods
IndexedDocs::Doc_t::Doc_t(string str) : doc(move(str)) {
    auto words = SplitBy(doc, ' ');
    for (auto sv : words) {
        ++words_in_doc[sv];
    }
}

IndexedDocs::Doc_t::Doc_t(Doc_t && d)
    : doc(move(d.doc)),
      words_in_doc(move(d.words_in_doc))
{}


size_t IndexedDocs::Doc_t::CalculateRelevation(const vector<string_view>& words) const {
    size_t result = 0;
    for (auto word : words) {
        if (auto it = words_in_doc.find(word); it != words_in_doc.end())
            result += it->second;
    }
    return result;
}

// IndexedDocs methods

void IndexedDocs::Add(const string& document) {
  docs.emplace_back(document);
  for (const auto &[word, count] : docs.back().words_in_doc)
      docs_with_word[word].insert(docs.size() - 1);
  int a = 3;
  a++;
}

const set<size_t> & IndexedDocs::DocWithWord(string_view word) const {
    if (auto it = docs_with_word.find(word); it != docs_with_word.end())
        return it->second;
    else
        return zero_set;
}


vector<pair<size_t, size_t>> IndexedDocs::GetRelevationDocs(const vector<string_view> words) {
    set<size_t> id_interesting_docs;
    for (auto word : words) {
        for (auto id : DocWithWord(word)) {
            id_interesting_docs.insert(id);
        }
    }

    auto cmp = [](pair<Relevation, DocId> lhs, pair<Relevation, DocId> rhs) {
        if (lhs.first != rhs.first)
            return lhs.first < rhs.first;
        else
            return lhs.second > rhs.second;
    };

    set<pair<Relevation, DocId>, decltype (cmp)> relevation_docs(cmp);

    for (auto id : id_interesting_docs) {
        relevation_docs.insert({docs[id].CalculateRelevation(words), id});
    }
    return {relevation_docs.rbegin(), relevation_docs.rend()};
}
