#ifndef PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP
#define PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include "../inverted_index/inverted_index.hpp"

const std::filesystem::path FILES_DIR_PATH = "../files";

void index_text_files(inverted_index_parallel& index, const std::vector<std::string>& file_names);

std::vector<std::string> get_file_names(std::string dir_path);

std::vector<std::pair<std::size_t, std::size_t>> get_bounds(std::size_t file_names_size, std::size_t num_workers);

#endif //PARALLEL_COMPUTING_CW_INDEX_TEXT_FILES_HPP
