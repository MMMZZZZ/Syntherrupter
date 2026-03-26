/*
 * Files.cpp
 *
 *  Created on: 08.03.2026
 *      Author: Max Zuidberg
 */

#include <Files.h>


SDClass Files::sd;
MidiPlayer Files::player;
bool Files::available;
struct Files::File Files::fileList[Files::FILES_COUNT_MAX];
uint32_t Files::fileCount;
uint32_t Files::playerStatus;


Files::Files()
{
    // TODO Auto-generated constructor stub

}

Files::~Files()
{
    // TODO Auto-generated destructor stub
}


void Files::init()
{
    available = sd.begin();
}

uint32_t Files::loadSorted(
        bool descending,
        bool sortByDate,
        bool sortByFullPath,
        bool includeSubdirs,
        bool includeDirs,
        bool mixFoldersFiles
        )
{
    auto cwd = sd.open("/");
    uint32_t parent = 0;

    while (fileCount < FILES_COUNT_MAX)
    {
        auto f = cwd.openNextFile();

        auto& entry = fileList[fileCount++];
        if (!f.name()[0])
        {
            // Reached end of directory
            break;
        }
        if (f.isDirectory())
        {
            // Only interested in files.
            continue;
        }
        int32_t extensionStart = -1;
        for (int32_t i = 0; i < NAME_LEN_MAX - 1; i++)
        {
            auto c = f.name()[i];
            entry.name[i] = c;
            // null = end of string
            if (!c)
            {
                break;
            }
            if (c == '.')
            {
                extensionStart = i;
            }
        }
        // Copy and paste extension
        for (int32_t i = 0; i < EXT_LEN_MAX - 1; i++)
        {
            auto& c = entry.name[extensionStart + i];
            entry.extension[i] = c;
            if (!c)
            {
                break;
            }
            // c = 0;
        }
        entry.parent = parent;
    }

    return fileCount;
}

