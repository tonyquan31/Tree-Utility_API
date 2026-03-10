#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <iomanip>

#define EXPORT_API __declspec(dllexport)

namespace fs = std::filesystem;

bool isHiddenFile(const fs::path& p) {
    std::string filename = p.filename().string();

    if (!filename.empty() && filename[0] == '.') {
        return true;
    }

    DWORD attr = GetFileAttributesW(p.wstring().c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        return (attr & FILE_ATTRIBUTE_HIDDEN) != 0;
    }

    return false;
}

std::string formatSize(uintmax_t size) {
    double s = static_cast<double>(size);
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unitIndex = 0;

    while (s >= 1024 && unitIndex < 4) {
        s /= 1024;
        unitIndex++;
    }

    std::stringstream ss;
    if (unitIndex == 0) {
        ss << size << " " << units[unitIndex];
    }
    else {
        ss << std::fixed << std::setprecision(1) << s << " " << units[unitIndex];
    }
    return ss.str();
}

void Tree(const fs::path& path, std::stringstream& ss, int maxDepth, bool dirsOnly, bool showAll, bool showSize, int currentDepth = 0, const std::string& prefix = "", bool isRoot = true, bool isLast = true)
{
    if (!fs::exists(path)) {
        ss << "Error: Path does not exist: " << path.string() << "\n";
        return;
    }

    std::string sizeStr = "";
    if (showSize && fs::is_regular_file(path)) {
        try {
            sizeStr = " [" + formatSize(fs::file_size(path)) + "]";
        }
        catch (...) {
            sizeStr = " [? KB]";
        }
    }

    if (isRoot) {
        ss << path.filename().string() << (path.filename().empty() ? path.string() : "") << sizeStr << "\n";
    }
    else {
        ss << prefix << (isLast ? "└── " : "├── ") << path.filename().string() << sizeStr << "\n";
    }

    if (maxDepth >= 0 && currentDepth >= maxDepth) {
        return;
    }

    if (fs::is_directory(path)) {
        std::vector<fs::directory_entry> entries;
        try {
            for (const auto& entry : fs::directory_iterator(path)) {

                if (!showAll && isHiddenFile(entry.path())) {
                    continue;
                }

                if (dirsOnly && !entry.is_directory()) {
                    continue;
                }

                entries.push_back(entry);
            }
        }
        catch (const fs::filesystem_error&) {
            ss << prefix << (isRoot ? "" : (isLast ? "    " : "│   ")) << "    [Access Denied]\n";
            return;
        }

        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            return a.path().filename().string() < b.path().filename().string();
            });

        std::string nextPrefix = isRoot ? "" : prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < entries.size(); ++i) {
            bool lastChild = (i == entries.size() - 1);
            Tree(entries[i].path(), ss, maxDepth, dirsOnly, showAll, showSize, currentDepth + 1, nextPrefix, false, lastChild);
        }
    }
}

// API (C-style)
extern "C" {
    EXPORT_API const char* GenerateTreeAPI(const char* dirPath, int maxDepth = -1, bool dirsOnly = false, bool showAll = false, bool showSize = false) {
        std::stringstream ss;
        Tree(fs::path(dirPath), ss, maxDepth, dirsOnly, showAll, showSize);

        std::string result = ss.str();

        char* output = new char[result.length() + 1];
        strcpy_s(output, result.length() + 1, result.c_str());

        return output;
    }

    EXPORT_API void FreeTreeMemoryAPI(const char* ptr) {
        if (ptr != nullptr) {
            delete[] ptr;
        }
    }
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-help" || arg == "--help" || arg == "-h") {
            std::cout << "Directory Tree Printer Utility\n\n";
            std::cout << "Usage: Tree.exe [path] [options]\n\n";
            std::cout << "Options:\n";
            std::cout << "  -L <level>     Limit the depth level of the printed tree.\n";
            std::cout << "  -d, --dirs     List directories only.\n";
            std::cout << "  -a, --all      All files are printed. By default hidden files are not printed.\n";
            std::cout << "  -s, --size     Print the size of each file.\n";
            std::cout << "  -h, -help      Display this help message and exit.\n\n";
            std::cout << "Examples:\n";
            std::cout << "  Tree.exe C:\\Windows -d\n";
            std::cout << "  Tree.exe -L 2 -a -s E:\\Code\n";
            return 0;
        }
    }

    fs::path targetDir = fs::current_path();
    int maxDepth = -1;
    bool dirsOnly = false;
    bool showAll = false;
	bool showSize = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-L" && i + 1 < argc) {
            try {
                maxDepth = std::stoi(argv[i + 1]);
            }
            catch (...) {
                std::cerr << "Error: Please enter a valid integer behind flag -L\n";
                return 1;
            }
            i++;
        }
        else if (arg == "-d" || arg == "--dirs") {
            dirsOnly = true;
        }
        else if (arg == "-a" || arg == "--all") {
            showAll = true;
        }
        else if (arg == "-s" || arg == "--size") {
            showSize = true;
        }
        else if (arg != "-help" && arg != "--help" && arg != "-h") {
            targetDir = argv[i];
        }
    }

    std::stringstream ss;
    Tree(targetDir, ss, maxDepth, dirsOnly, showAll, showSize);
    std::cout << ss.str();

    return 0;
}