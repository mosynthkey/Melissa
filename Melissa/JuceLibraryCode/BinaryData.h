/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   add_svg;
    const int            add_svgSize = 1116;

    extern const char*   add_highlighted_svg;
    const int            add_highlighted_svgSize = 1126;

    extern const char*   arrow_left_svg;
    const int            arrow_left_svgSize = 1165;

    extern const char*   arrow_left_highlighted_svg;
    const int            arrow_left_highlighted_svgSize = 1175;

    extern const char*   arrow_right_svg;
    const int            arrow_right_svgSize = 1263;

    extern const char*   arrow_right_highlighted_svg;
    const int            arrow_right_highlighted_svgSize = 1273;

    extern const char*   playlist_add_svg;
    const int            playlist_add_svgSize = 2731;

    extern const char*   playlist_add_file_svg;
    const int            playlist_add_file_svgSize = 2087;

    extern const char*   playlist_add_file_highlighted_svg;
    const int            playlist_add_file_highlighted_svgSize = 2089;

    extern const char*   playlist_add_highlighted_svg;
    const int            playlist_add_highlighted_svgSize = 2733;

    extern const char*   playlist_add_playing_svg;
    const int            playlist_add_playing_svgSize = 2000;

    extern const char*   playlist_add_playing_highlighted_svg;
    const int            playlist_add_playing_highlighted_svgSize = 2002;

    extern const char*   playlist_edit_svg;
    const int            playlist_edit_svgSize = 2979;

    extern const char*   playlist_edit_highlighted_svg;
    const int            playlist_edit_highlighted_svgSize = 2981;

    extern const char*   playlist_remove_svg;
    const int            playlist_remove_svgSize = 3230;

    extern const char*   playlist_remove_highlighted_svg;
    const int            playlist_remove_highlighted_svgSize = 3232;

    extern const char*   enUS_txt;
    const int            enUS_txtSize = 3089;

    extern const char*   jaJP_txt;
    const int            jaJP_txtSize = 4008;

    extern const char*   logo_png;
    const int            logo_pngSize = 121456;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 19;

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
