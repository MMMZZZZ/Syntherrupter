/**
 * Created using Claude Sonnet 4.6
 * SD.cpp - SD-Class for TI TM4C1294 using FatFs and an SD card over SPI
 */

#include "SD.h"
#include <string.h>
#include <stdio.h>


// =======================================================================
// SDFile - Implementation
// =======================================================================

SDFile::SDFile()
    : _isOpen(false), _isDir(false)
{
    _name[0] = '\0';
    memset(&_file, 0, sizeof(_file));
    memset(&_dir,  0, sizeof(_dir));
}

SDFile::~SDFile()
{
    // Do close automatically
    close();
}

void SDFile::_open(const char* path, uint8_t mode, bool isDir)
{
    _isDir  = isDir;
    _isOpen = true;

    // Extract file name from path
    const char* lastSlash = strrchr(path, '/');
    const char* base      = lastSlash ? lastSlash + 1 : path;
    strncpy(_name, base, FF_LFN_BUF);
    _name[FF_LFN_BUF] = '\0';
}

// ---- Write ----

size_t SDFile::write(uint8_t data)
{
    return write(&data, 1);
}

size_t SDFile::write(const uint8_t* buf, size_t len)
{
    if (!_isOpen || _isDir) return 0;

    UINT written = 0;
    f_write(&_file, buf, (UINT)len, &written);
    return (size_t)written;
}

size_t SDFile::write(const char* str)
{
    if (!str) return 0;
    return write((const uint8_t*)str, strlen(str));
}

// ---- Read ----

int SDFile::read()
{
    if (!_isOpen || _isDir) return -1;

    uint8_t byte;
    UINT    bytesRead = 0;
    FRESULT res = f_read(&_file, &byte, 1, &bytesRead);
    if (res != FR_OK || bytesRead == 0) return -1;
    return (int)byte;
}

int SDFile::read(void* buf, size_t len)
{
    if (!_isOpen || _isDir) return -1;

    UINT    bytesRead = 0;
    FRESULT res = f_read(&_file, buf, (UINT)len, &bytesRead);
    if (res != FR_OK) return -1;
    return (int)bytesRead;
}

int SDFile::peek()
{
    if (!_isOpen || _isDir) return -1;

    FSIZE_t pos = f_tell(&_file);
    int     val = read();
    if (val >= 0) f_lseek(&_file, pos);
    return val;
}

int SDFile::available()
{
    if (!_isOpen || _isDir) return 0;
    return (int)(f_size(&_file) - f_tell(&_file));
}

// ---- Position ----

bool SDFile::seek(uint32_t pos)
{
    if (!_isOpen || _isDir) return false;
    return (f_lseek(&_file, (FSIZE_t)pos) == FR_OK);
}

uint32_t SDFile::position()
{
    if (!_isOpen || _isDir) return 0;
    return (uint32_t)f_tell(&_file);
}

uint32_t SDFile::size()
{
    if (!_isOpen || _isDir) return 0;
    return (uint32_t)f_size(&_file);
}

// ---- Control ----

void SDFile::flush()
{
    if (_isOpen && !_isDir) f_sync(&_file);
}

void SDFile::close()
{
    if (!_isOpen) return;

    if (_isDir) {
        f_closedir(&_dir);
    } else {
        f_close(&_file);
    }
    _isOpen = false;
}

bool SDFile::isDirectory()
{
    return _isDir;
}

// ---- Directory Iteration ----

SDFile SDFile::openNextFile(uint8_t mode)
{
    SDFile entry;
    if (!_isOpen || !_isDir) return entry;

    FILINFO fno;
    FRESULT res = f_readdir(&_dir, &fno);

    if (res != FR_OK || fno.fname[0] == '\0') {
        // No more entries
        return entry;
    }

    // Skip . / .. entries
    if (fno.fname[0] == '.') {
        return openNextFile(mode); // Recursion
    }

    bool isDir = (fno.fattrib & AM_DIR) != 0;

    if (isDir) {
        // Open directory
        // Path should be complete – simplified using _name
        char fullPath[SD_MAX_PATH_LEN];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", _name, fno.fname);

        if (f_opendir(&entry._dir, fullPath) == FR_OK) {
            entry._isDir  = true;
            entry._isOpen = true;
            strncpy(entry._name, fno.fname, FF_LFN_BUF);
            entry._name[FF_LFN_BUF] = '\0';
        }
    } else {
        // Open file – path should be complete
        char fullPath[SD_MAX_PATH_LEN];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", _name, fno.fname);

        if (f_open(&entry._file, fullPath, mode) == FR_OK) {
            entry._isDir  = false;
            entry._isOpen = true;
            strncpy(entry._name, fno.fname, FF_LFN_BUF);
            entry._name[FF_LFN_BUF] = '\0';
        }
    }

    return entry;
}

void SDFile::rewindDirectory()
{
    if (_isOpen && _isDir) f_rewinddir(&_dir);
}

// =======================================================================
// SDClass - Implementation
// =======================================================================

SDClass::SDClass()
    : _mounted(false)
{
    memset(&_fs, 0, sizeof(_fs));
}

bool SDClass::begin()
{
    FRESULT res = f_mount(&_fs, "", 1); // "" = logic drive 0, mount immediately
    if (res != FR_OK) {
        _mounted = false;
        return false;
    }
    _mounted = true;
    return true;
}

void SDClass::end()
{
    if (_mounted) {
        f_mount(NULL, "", 0); // Unmount
        _mounted = false;
    }
}

SDFile SDClass::open(const char* path, uint8_t mode)
{
    SDFile f;
    if (!_mounted) return f;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    // Test if it's a directory
    // Note: The root directory doesn't have a name and thus the test fails
    FILINFO fno;
    FRESULT res = f_stat(normPath, &fno);

    if (res == FR_OK && (fno.fattrib & AM_DIR)
           || (res == FR_INVALID_NAME && normPath[0] == '/' && normPath[1] == 0)) {
        // Open directory
        if (f_opendir(&f._dir, normPath) == FR_OK) {
            f._open(normPath, mode, true);
        }
    } else {
        // Open file
        if (f_open(&f._file, normPath, mode) == FR_OK) {
            f._open(normPath, mode, false);
        }
    }

    return f;
}

bool SDClass::exists(const char* path)
{
    if (!_mounted) return false;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    FILINFO fno;
    return (f_stat(normPath, &fno) == FR_OK);
}

bool SDClass::remove(const char* path)
{
    if (!_mounted) return false;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    return (f_unlink(normPath) == FR_OK);
}

bool SDClass::rename(const char* oldPath, const char* newPath)
{
    if (!_mounted) return false;

    char normOld[SD_MAX_PATH_LEN];
    char normNew[SD_MAX_PATH_LEN];
    _normalizePath(oldPath, normOld, sizeof(normOld));
    _normalizePath(newPath, normNew, sizeof(normNew));

    return (f_rename(normOld, normNew) == FR_OK);
}

bool SDClass::mkdir(const char* path)
{
    if (!_mounted) return false;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    // Create directories recursively
    char tmp[SD_MAX_PATH_LEN];
    strncpy(tmp, normPath, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    // Skip leading '/'
    char* p = tmp;
    if (*p == '/') p++;

    while (*p) {
        if (*p == '/') {
            *p = '\0';
            FRESULT res = f_mkdir(tmp);
            if (res != FR_OK && res != FR_EXIST) return false;
            *p = '/';
        }
        p++;
    }

    FRESULT res = f_mkdir(tmp);
    return (res == FR_OK || res == FR_EXIST);
}

bool SDClass::rmdir(const char* path)
{
    if (!_mounted) return false;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    return (f_rmdir(normPath) == FR_OK);
}

bool SDClass::isDirectory(const char* path)
{
    if (!_mounted) return false;

    char normPath[SD_MAX_PATH_LEN];
    _normalizePath(path, normPath, sizeof(normPath));

    FILINFO fno;
    if (f_stat(normPath, &fno) != FR_OK) return false;
    return (fno.fattrib & AM_DIR) != 0;
}

uint64_t SDClass::freeSpace()
{
    if (!_mounted) return 0;

    DWORD   clust = 0;
    FATFS*  fs    = nullptr;
    if (f_getfree("", &clust, &fs) != FR_OK) return 0;

    // Free bytes = free clusters * sectors/cluster * bytes/sector
    return (uint64_t)clust * fs->csize * 512ULL;
}

uint64_t SDClass::totalSpace()
{
    if (!_mounted) return 0;

    // Total size = (total clusters) * sectors/cluster * bytes/sector
    // FatFs stores the total number of clusters in n_fatent - 2
    DWORD   clust = 0;
    FATFS*  fs    = nullptr;
    if (f_getfree("", &clust, &fs) != FR_OK) return 0;

    uint32_t totalClusters = fs->n_fatent - 2;
    return (uint64_t)totalClusters * fs->csize * 512ULL;
}

// ---- Auxiliary functions ----

void SDClass::_normalizePath(const char* in, char* out, size_t outLen)
{
    if (!in || outLen == 0) return;

    if (in[0] != '/') {
        out[0] = '/';
        strncpy(out + 1, in, outLen - 2);
        out[outLen - 1] = '\0';
    } else {
        strncpy(out, in, outLen - 1);
        out[outLen - 1] = '\0';
    }
}
