#ifndef SD_H
#define SD_H

/**
 * Created using Claude Sonnet 4.6
 * SD.h - SD Class for TI TM4C1294 with FatFs over SPI
 *
 * Inspired by the Arduino SD API.
 * Prerequisites:
 *   - FatFs (http://elm-chan.org/fsw/ff/00index_e.html)
 *   - TivaWare / TM4C1294 DriverLib
 *   - diskio.c / mmc_tm4c.c must be implemented separately
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"       // FatFs Header
#include "diskio.h"   // FatFs Disk I/O

// -----------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------
#define FILE_READ   FA_READ
#define FILE_WRITE  (FA_READ | FA_WRITE | FA_OPEN_ALWAYS)

// Max path length
#define SD_MAX_PATH_LEN  256

// -----------------------------------------------------------------------
// SDFile - Represents an open file
// -----------------------------------------------------------------------
class SDFile {
public:
    SDFile();
    ~SDFile();

    // Write
    size_t write(uint8_t data);
    size_t write(const uint8_t* buf, size_t len);
    size_t write(const char* str);

    // Read
    int    read();
    int    read(void* buf, size_t len);
    int    peek();
    int    available();

    // Position
    bool   seek(uint32_t pos);
    uint32_t position();
    uint32_t size();

    // Control
    void   flush();
    void   close();
    bool   isDirectory();

    // Directory Iteration
    SDFile openNextFile(uint8_t mode = FILE_READ);
    void   rewindDirectory();

    // Operator overloading for bool-Check
    operator bool() const { return _isOpen; }

    // Name of the file / directory
    const char* name() const { return _name; }

private:
    FIL     _file;
    DIR     _dir;
    bool    _isOpen;
    bool    _isDir;
    char    _name[FF_LFN_BUF + 1];

    // Only SDClass is allowed to fully initialize objects
    friend class SDClass;
    void _open(const char* path, uint8_t mode, bool isDir);
};

// -----------------------------------------------------------------------
// SDClass - Main class
// -----------------------------------------------------------------------
class SDClass {
public:
    SDClass();

    /**
     * Initialize the SD Card
     * @return true bei Erfolg
     */
    bool begin();

    /**
     * Free ressources (unmounts fatfs volume)
     */
    void end();

    // ---- File Operations ----

    /** Opens a file. mode: FILE_READ or FILE_WRITE */
    SDFile open(const char* path, uint8_t mode = FILE_READ);

    /** Checks if a file or directory exists */
    bool exists(const char* path);

    /** Deletes a file */
    bool remove(const char* path);

    /** Renames a file */
    bool rename(const char* oldPath, const char* newPath);

    // ---- Directory Operations ----

    /** Creates a directory (including parents) */
    bool mkdir(const char* path);

    /** Deletes an (empty) directory */
    bool rmdir(const char* path);

    /** Returns true if the path points to a directory */
    bool isDirectory(const char* path);

    // ---- Info ----

    /** Returns available space in bytes (can take some time to run) */
    uint64_t freeSpace();

    /** Return the total size in bytes */
    uint64_t totalSpace();

    /** Returns true if begin() was successful */
    bool isMounted() const { return _mounted; }

    // Operator-overloading for bool check
    operator bool() const { return _mounted; }

private:
    FATFS   _fs;
    bool    _mounted;

    // Helper function: normalizes path (prepends '/' if necessary)
    void _normalizePath(const char* in, char* out, size_t outLen);
};

// Global instance
extern SDClass SD;

#endif // SD_H
