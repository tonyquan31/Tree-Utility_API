#include <iostream>
#include <windows.h>
#include <string>

// 1. Define function pointer types matching the DLL exported symbols.
// Signature: const char* (const char* path, int maxDepth, bool dirsOnly, bool showAll, bool showSize)
typedef const char* (*GenerateTreeFunc)(const char*, int, bool, bool, bool);
typedef void (*FreeTreeMemoryFunc)(const char*);

int main() {
    // 2. Load the dynamic library.
    // Ensure Tree.dll is in the same directory as this executable or provide a full path.
    HMODULE hModule = LoadLibraryA("Tree.dll");
    if (hModule == NULL) {
        std::cerr << "Error: Could not load Tree.dll! (Ensure it is in the current directory)\n";
        std::cerr << "System Error Code: " << GetLastError() << std::endl;
        return 1;
    }

    // 3. Map the functions from the DLL to our function pointers.
    GenerateTreeFunc GenerateTree = (GenerateTreeFunc)GetProcAddress(hModule, "GenerateTreeAPI");
    FreeTreeMemoryFunc FreeMemory = (FreeTreeMemoryFunc)GetProcAddress(hModule, "FreeTreeMemoryAPI");

    if (!GenerateTree || !FreeMemory) {
        std::cerr << "Error: Could not find API functions within the DLL.\n";
        FreeLibrary(hModule);
        return 1;
    }

    // 4. Setup API parameters.
    const char* targetPath = "."; // Use current directory as a test
    int maxDepth = 2;             // Level 2 depth limit
    bool dirsOnly = false;        // Include both files and folders
    bool showAll = false;         // Hide hidden files/folders (e.g., .git, .vs)
    bool showSize = true;          // Display file sizes

    // 5. Execute the API.
    std::cout << "--- Executing Tree API from DLL ---\n";
    const char* treeOutput = GenerateTree(targetPath, maxDepth, dirsOnly, showAll, showSize);

    if (treeOutput != nullptr) {
        // Set console to UTF-8 to correctly render the tree structure symbols (├──, └──).
        SetConsoleOutputCP(CP_UTF8);

        std::cout << "Directory Structure for: " << targetPath << "\n";
        std::cout << "----------------------------------------\n";
        std::cout << treeOutput << std::endl;

        // 6. Memory Management: Call the DLL's free function to prevent memory leaks.
        // This is crucial because memory was allocated inside the DLL using 'new'.
        FreeMemory(treeOutput);
    }
    else {
        std::cerr << "Error: API returned a null pointer.\n";
    }

    // 7. Unload the DLL from memory.
    FreeLibrary(hModule);

    std::cout << "--- Process Finished ---" << std::endl;
    return 0;
}