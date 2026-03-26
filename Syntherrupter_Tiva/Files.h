/*
 * Files.h
 *
 *  Created on: 08.03.2026
 *      Author: Max Zuidberg
 */

#ifndef FILES_H_
#define FILES_H_


#include "stdbool.h"
#include "stdint.h"
#include "InterrupterConfig.h"
#include "System.h"
#include "SD Card/SD.h"
#include "MidiPlayer.h"



class Files
{
public:
    static constexpr uint32_t FILES_COUNT_MAX = 200;
    static constexpr uint32_t NAME_LEN_MAX = 64;
    static constexpr uint32_t EXT_LEN_MAX = 6;
    struct File {
        uint32_t parent;
        char name[NAME_LEN_MAX];
        char extension[EXT_LEN_MAX];
    };

    Files();
    virtual ~Files();
    static void init();
    static uint32_t loadSorted(bool descending = false, bool sortByDate = false, bool sortByFullPath = false, bool includeSubdirs = false, bool includeDirs = false, bool mixFoldersFiles = false);
    static uint32_t getFileCount()
    {
        return fileCount;
    };
    static const char* getFileName(uint32_t fileNum)
    {
        if (fileNum < fileCount)
        {
            return fileList[fileNum].name;
        }
        else
        {
            return 0;
        }
    };
    static void open(uint32_t fileNum)
    {
        if (fileNum < fileCount)
        {
            auto f = sd.open(fileList[fileNum].name);
            player.open(f);
        }
    };
    static void play()
    {
        player.play();
    };
    static void pause()
    {
        player.pause();
    };
    static void stop()
    {
        player.stop();
    };
    static void update()
    {
        switch (playerStatus)
        {
        case 1:
            play();
            break;
        case 2:
            pause();
            break;
        case 3:
            stop();
            break;
        case 4:
        {
            uint32_t num = 0;
            stop();
            open(num);
            break;
        }
        }
        playerStatus = 0;
        player.tick();
    };
private:
    static SDClass sd;
    static MidiPlayer player;
    static bool available;
    static struct File fileList[FILES_COUNT_MAX];
    static uint32_t fileCount;
    static uint32_t playerStatus;
};


#endif /* FILES_H_ */
