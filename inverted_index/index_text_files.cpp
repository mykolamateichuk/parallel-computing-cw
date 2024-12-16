#include "index_text_files.hpp"

void index_text_files(index_worker& worker, const std::vector<std::string>& files) {
    for (const auto& file_path : files) {
        std::ifstream file(file_path);

        if (file.is_open()) {
            std::ostringstream content;
            content << file.rdbuf();
            worker.update(file_path, content.str());
        } else {
            std::cerr << "Failed to open file: " << file_path << std::endl;
        }
    }
}
