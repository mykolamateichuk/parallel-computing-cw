#ifndef PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP
#define PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP

#include <iostream>
#include <fstream>
#include "index_worker.hpp"

void index_text_files(index_worker& worker, const std::vector<std::string>& files);

#endif //PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP
