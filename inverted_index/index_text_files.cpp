#include "index_text_files.hpp"

void index_text_files(inverted_index_parallel& index, const std::vector<std::string>& file_names) {
    for (const auto& file_name : file_names) {
        std::ifstream file(file_name);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_name << "\n";
            continue;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        index.add(file_name, content);
    }
}

std::vector<std::string> get_file_names(std::string dir_path) {
    std::vector<std::string> file_names;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) file_names.push_back(entry.path());
    }

    return file_names;
}


std::vector<std::pair<std::size_t, std::size_t>> get_bounds(std::size_t file_names_size, std::size_t num_workers) {
    std::size_t files_per_worker = file_names_size / num_workers;
    std::size_t remainder = file_names_size - files_per_worker * num_workers;

    std::vector<std::pair<std::size_t, std::size_t>> bounds;

    std::size_t start = 0;
    std::size_t end = -1;
    for (std::size_t worker = 0; worker < num_workers; worker++) {
        start = end + 1;
        end = start + files_per_worker - 1;
        if (worker == num_workers - 1) end += remainder;

        bounds.push_back(std::pair(start, end));
    }

    return bounds;
}
