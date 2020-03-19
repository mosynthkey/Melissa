/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   NotoSansCJKjpRegular_otf;
    const int            NotoSansCJKjpRegular_otfSize = 16427228;

    extern const char*   enUS_txt;
    const int            enUS_txtSize = 593;

    extern const char*   jaJP_txt;
    const int            jaJP_txtSize = 1626;

    extern const char*   icon_png;
    const int            icon_pngSize = 352351;

    extern const char*   logo_png;
    const int            logo_pngSize = 121456;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
