#include "utils.h"

#include <map>

std::filesystem::path get_documents_path()
{
    wchar_t Folder[1024];
    HRESULT hr = SHGetFolderPathW(0, CSIDL_MYDOCUMENTS, 0, 0, Folder);
    if (SUCCEEDED(hr))
    {
        char str[1024];
        wcstombs(str, Folder, 1023);
        return str;
    }
    else return "";
}

float get_distance(CVector position)
{
    const auto my_pos = FindPlayerPed()->GetPosition();

    const auto x_dif = position.x - my_pos.x;
    const auto y_dif = position.y - my_pos.y;
    const auto z_dif = position.z - my_pos.z;

    return sqrt(x_dif * x_dif + y_dif * y_dif + z_dif * z_dif);
}