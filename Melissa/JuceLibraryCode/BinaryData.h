/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   speaker_svg;
    const int            speaker_svgSize = 357;

    extern const char*   add_svg;
    const int            add_svgSize = 917;

    extern const char*   clear_svg;
    const int            clear_svgSize = 778;

    extern const char*   arrow_left_svg;
    const int            arrow_left_svgSize = 423;

    extern const char*   arrow_right_svg;
    const int            arrow_right_svgSize = 500;

    extern const char*   down_svg;
    const int            down_svgSize = 524;

    extern const char*   loop_onesong_svg;
    const int            loop_onesong_svgSize = 891;

    extern const char*   loop_playlist_svg;
    const int            loop_playlist_svgSize = 1337;

    extern const char*   next_button_svg;
    const int            next_button_svgSize = 429;

    extern const char*   playlist_add_svg;
    const int            playlist_add_svgSize = 583;

    extern const char*   playlist_add_file_svg;
    const int            playlist_add_file_svgSize = 1155;

    extern const char*   playlist_add_playing_svg;
    const int            playlist_add_playing_svgSize = 1442;

    extern const char*   playlist_edit_svg;
    const int            playlist_edit_svgSize = 687;

    extern const char*   playlist_remove_svg;
    const int            playlist_remove_svgSize = 865;

    extern const char*   prev_button_svg;
    const int            prev_button_svgSize = 426;

    extern const char*   up_svg;
    const int            up_svgSize = 524;

    extern const char*   enUS_txt;
    const int            enUS_txtSize = 9968;

    extern const char*   jaJP_txt;
    const int            jaJP_txtSize = 11250;

    extern const char*   logo_png;
    const int            logo_pngSize = 110193;

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
