#ifndef MIDI_PLAYER_H
#define MIDI_PLAYER_H

/**
 * Created using Claude Sonnet 4.6
 *
 * MidiPlayer.h - Real-time MIDI file player for TM4C1294
 *
 * Reads SMF0 and SMF1 MIDI files from SD card and schedules MIDI events
 * in real-time by writing them into a circular output buffer.
 *
 * Usage:
 *   1. Call MidiPlayer::begin() once to initialize.
 *   2. Call MidiPlayer::open("/song.mid") to load a file.
 *   3. Call MidiPlayer::play() to start playback.
 *   4. Call MidiPlayer::tick() from your main loop (or a timer ISR) as
 *      often as possible — ideally every 1 ms or faster.
 *   5. Consume bytes from the output buffer (e.g. in a UART TX ISR) via
 *      MidiPlayer::readByte() / MidiPlayer::available().
 *
 * Timing:
 *   tick() uses System::getSystemTimeMS() (1 kHz) for coarse scheduling and an
 *   optional microsecond timer for sub-millisecond accuracy.
 *   Define MIDI_PLAYER_USE_TIMER32 to enable Timer32 for µs resolution.
 *
 * Limitations:
 *   - Up to MIDI_MAX_TRACKS tracks (default 32)
 *   - Chunk data is NOT loaded entirely into RAM; the file is seeked
 *     per-track on each tick (suitable for low-RAM embedded targets)
 *   - SysEx events are forwarded as-is
 *   - Meta events are consumed internally (only Tempo is acted upon)
 *   - SMF2 (pattern-based) is not supported
 */

#include <stdbool.h>
#include <stdint.h>
#include "System.h"
#include "SD Card/SD.h"
#include "MIDI.h"

// -----------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------

/** Maximum number of tracks (SMF1). Reduce to save RAM. */
#define MIDI_MAX_TRACKS         32

/** Output circular buffer size in bytes. Must be power of 2. */
#define MIDI_OUT_BUF_SIZE       256

/** Default tempo: 120 BPM = 500000 µs per quarter note */
#define MIDI_DEFAULT_TEMPO      500000UL

// -----------------------------------------------------------------------
// Public types
// -----------------------------------------------------------------------

enum MidiPlayerState {
    MIDI_STATE_IDLE,
    MIDI_STATE_PLAYING,
    MIDI_STATE_PAUSED,
    MIDI_STATE_DONE,
    MIDI_STATE_ERROR
};

// -----------------------------------------------------------------------
// MidiPlayer class
// -----------------------------------------------------------------------

class MidiPlayer {
public:
    MidiPlayer();

    /**
     * Open a MIDI file from the SD card.
     * Returns true on success. Must be called before play().
     */
    bool open(SDFile file);

    /** Close the current file and reset state. */
    void close();

    /** Start or resume playback. */
    void play();

    /** Pause playback (remembers position). */
    void pause();

    /** Stop playback and rewind. */
    void stop();

    /**
     * Must be called as frequently as possible (e.g. every 1 ms).
     * Processes due events and writes MIDI bytes to the output buffer.
     */
    void tick();

    MidiPlayerState state() const { return _state; }

    /** Current playback position in ticks */
    uint32_t positionTicks() const { return _globalTick; }

    /** Current tempo in µs per quarter note */
    uint32_t tempo() const { return _tempo; }

    /** File's ticks per quarter note (from header) */
    uint16_t ticksPerQN() const { return _ticksPerQN; }

private:
    // ---- Track state ----
    struct TrackState {
        uint32_t fileOffset;    // Absolute file offset of track chunk data start
        uint32_t length;        // Track chunk data length in bytes
        uint32_t bytesRead;     // Bytes consumed so far in this track
        uint32_t nextEventTick; // Absolute tick when next event is due
        uint8_t  runningStatus; // Last channel status byte (running status)
        bool     active;        // false = track finished
    };

    // ---- MIDI header info ----
    uint16_t        _format;        // 0 or 1
    uint16_t        _numTracks;
    uint16_t        _ticksPerQN;    // ticks per quarter note (assuming metrical time)

    // ---- Playback state ----
    MidiPlayerState _state;
    uint32_t        _tempo;         // µs per quarter note
    uint32_t        _globalTick;    // Current absolute tick counter

    // µs accumulator: tracks fractional ticks between tick() calls
    uint32_t        _usAccumulator; // accumulated µs since last tick advance
    uint32_t        _lastCallMs;    // System::getSystemTimeMS() at last tick() call

    // ---- Track table ----
    TrackState      _tracks[MIDI_MAX_TRACKS];
    uint8_t         _activeTrackCount;

    // ---- File handle ----
    SDFile          _file;

    // ---- Output circular buffer ----
    uint8_t         _outBuf[MIDI_OUT_BUF_SIZE];
    volatile uint16_t _outHead; // write index
    volatile uint16_t _outTail; // read index

    // ---- Private helpers ----

    // Buffer
    void     _writeByte(uint8_t b);
    void     _writeBytes(const uint8_t* data, uint16_t len);

    // File parsing
    bool     _parseHeader();
    bool     _scanTrackOffsets();

    // Per-track event processing
    // Returns false if track is exhausted or a file error occurred
    bool     _processTrack(uint8_t trackIdx);

    // Variable-length quantity decoder
    // Reads from file, advances track bytesRead, returns decoded value
    // Returns false on error
    bool     _readVLQ(uint8_t trackIdx, uint32_t& out);

    // Low-level file read for a specific track (handles seeking)
    bool     _trackReadByte(uint8_t trackIdx, uint8_t& out);
    bool     _trackReadBytes(uint8_t trackIdx, uint8_t* buf, uint16_t len);
    bool     _trackPeekByte(uint8_t trackIdx, uint8_t& out);

    // Event handlers
    void     _handleMeta(uint8_t trackIdx);
    void     _handleSysEx(uint8_t trackIdx, uint8_t type);
    void     _handleMidiEvent(uint8_t trackIdx, uint8_t status);

    // Tick timing
    uint32_t _usPerTick() const;
};


#endif // MIDI_PLAYER_H
