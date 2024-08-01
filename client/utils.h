#include <Windows.h>
#include <iostream>
#include <shlobj.h>
#include <filesystem>

#pragma comment(lib, "shell32.lib")
#include "plugin.h"
#pragma once

extern std::filesystem::path get_documents_path();

extern float get_distance(CVector position);
