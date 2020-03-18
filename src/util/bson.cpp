#include "bson.hpp"
#include <sstream>
#include "bson_document.hpp"

using namespace realm;






//
//Document::Document(const Document& d)
//{
//     for (auto const& pair : d.m_values)
//       {
//           m_values.emplace_back(std::make_pair(pair.first, std::move(pair.second)));
//       }
////    if (!d.m_map.empty())
////        m_map.insert(make_move_iterator(std::begin(d.m_map)),
////                     make_move_iterator(std::end(d.m_map)));
////
////    if (!d.m_insert_order.empty())
////        m_insert_order = std::vector<std::pair<std::string, size_t>>(d.m_insert_order.begin(),
////                                                                     d.m_insert_order.end());
//
//}
//
//Document::Document(std::initializer_list<std::pair<std::string, Bson>> pairs):
//    std::vector<std::pair<std::string, Bson>>(pairs) {}
//Document& Document::operator=(const Document& d) {
//    for (auto const& pair : d.m_values)
//    {
//        m_values.emplace_back(std::make_pair(pair.first, std::move(pair.second)));
//    }
////    m_values = std::vector<std::pair<std::string, Bson*>>(d.m_values.begin(), d.m_values.end());
////    if (!d.m_map.empty())
////        m_map.insert(make_move_iterator(std::begin(d.m_map)),
////                     make_move_iterator(std::end(d.m_map)));
////
////    if (!d.m_insert_order.empty())
////        m_insert_order = std::vector<std::pair<std::string, size_t>>(d.m_insert_order.begin(), d.m_insert_order.end());
////
////    return *this;
//}
//
//Document& Document::operator=(Document&& d) {
//     for (auto const& pair : d.m_values)
//       {
//           m_values.emplace_back(std::make_pair(pair.first, std::move(pair.second)));
//       }
////    if (!d.m_map.empty())
////        m_map.insert(make_move_iterator(std::begin(d.m_map)),
////                     make_move_iterator(std::end(d.m_map)));
////
////    if (!d.m_insert_order.empty())
////        m_insert_order = std::vector<std::pair<std::string, size_t>>(d.m_insert_order.begin(), d.m_insert_order.end());
////
////    return *this;
//}

