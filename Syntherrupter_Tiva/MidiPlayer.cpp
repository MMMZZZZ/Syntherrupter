/**
 * Created using Claude Sonnet 4.6
 *
 * MidiPlayer.cpp - Real-time MIDI file player
 *
 * Supports SMF format 0 and format 1 files.
 * File is seeked per-event — no full track buffering in RAM.
 */

#include "MidiPlayer.h"
#include <string.h>


// -----------------------------------------------------------------------
// MIDI event data lengths (bytes after status, excl. status itself)
// Index = (status >> 4) & 0x07  for status 0x80..0xE0
// -----------------------------------------------------------------------
static const uint8_t kMidiDataBytes[8] = {
    2, // 0x8n Note Off
    2, // 0x9n Note On
    2, // 0xAn Poly Aftertouch
    2, // 0xBn Control Change
    1, // 0xCn Program Change
    1, // 0xDn Channel Aftertouch
    2, // 0xEn Pitch Bend
    0, // 0xFn System (handled separately)
};

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------

MidiPlayer::MidiPlayer()
    : _format(0), _numTracks(0), _ticksPerQN(480),
      _state(MIDI_STATE_IDLE), _tempo(MIDI_DEFAULT_TEMPO),
      _globalTick(0), _usAccumulator(0), _lastCallMs(0),
      _activeTrackCount(0), _outHead(0), _outTail(0)
{
    memset(_tracks, 0, sizeof(_tracks));
    memset(_outBuf, 0, sizeof(_outBuf));
}

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------

bool MidiPlayer::open(SDFile file)
{
    close();

    _file = file;
    if (!(_file)) return false;

    if (!_parseHeader())    { close(); return false; }
    if (!_scanTrackOffsets()){ close(); return false; }

    _state = MIDI_STATE_IDLE;
    return true;
}

void MidiPlayer::close()
{
    _file.close();
    _state          = MIDI_STATE_IDLE;
    _globalTick     = 0;
    _usAccumulator  = 0;
    _tempo          = MIDI_DEFAULT_TEMPO;
    _activeTrackCount = 0;
    _outHead        = 0;
    _outTail        = 0;
    memset(_tracks, 0, sizeof(_tracks));
}

void MidiPlayer::play()
{
    if (_state == MIDI_STATE_IDLE || _state == MIDI_STATE_PAUSED) {
        _lastCallMs    = System::getSystemTimeMS();
        _state         = MIDI_STATE_PLAYING;
    }
}

void MidiPlayer::pause()
{
    if (_state == MIDI_STATE_PLAYING)
        _state = MIDI_STATE_PAUSED;
}

void MidiPlayer::stop()
{
    _state         = MIDI_STATE_IDLE;
    _globalTick    = 0;
    _usAccumulator = 0;
    _tempo         = MIDI_DEFAULT_TEMPO;

    // Rewind all tracks
    for (uint8_t i = 0; i < _numTracks; i++) {
        _tracks[i].bytesRead     = 0;
        _tracks[i].nextEventTick = 0;
        _tracks[i].runningStatus = 0;
        _tracks[i].active        = true;
    }

    // Re-read first delta times
    _scanTrackOffsets();

    // All-notes-off on all channels
    for (uint8_t ch = 0; ch < 16; ch++) {
        _writeByte(0xB0 | ch);
        _writeByte(123); // All Notes Off
        _writeByte(0);
    }
}

void MidiPlayer::tick()
{
    if (_state != MIDI_STATE_PLAYING) return;

    // ---- Advance tick counter based on elapsed time ----
    uint32_t now     = System::getSystemTimeMS();
    uint32_t elapsed = now - _lastCallMs;     // ms elapsed since last call
    _lastCallMs      = now;

    // Convert elapsed ms to µs and accumulate
    _usAccumulator += elapsed * 1000UL;

    // How many µs per MIDI tick?
    uint32_t usPerTick = _usPerTick();

    // Advance global tick counter
    while (_usAccumulator >= usPerTick) {
        _usAccumulator -= usPerTick;
        _globalTick++;
    }

    // ---- Process all tracks for events due at or before _globalTick ----
    bool anyActive = false;

    for (uint8_t i = 0; i < _numTracks; i++) {
        if (!_tracks[i].active) continue;
        anyActive = true;

        // Drain all events due at the current tick
        while (_tracks[i].active &&
               _tracks[i].nextEventTick <= _globalTick) {
            if (!_processTrack(i)) {
                _tracks[i].active = false;
            }
        }
    }

    if (!anyActive) {
        _state = MIDI_STATE_DONE;
    }
}

// -----------------------------------------------------------------------
// Output buffer
// -----------------------------------------------------------------------

void MidiPlayer::_writeByte(uint8_t b)
{
    MIDI::otherBuffer.add(b);
}

void MidiPlayer::_writeBytes(const uint8_t* data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) _writeByte(data[i]);
}

// -----------------------------------------------------------------------
// SMF Header parsing
// -----------------------------------------------------------------------

bool MidiPlayer::_parseHeader()
{
    _file.seek(0);

    uint8_t buf[14];
    if (_file.read(buf, 14) != 14) return false;

    // "MThd"
    if (buf[0] != 'M' || buf[1] != 'T' ||
        buf[2] != 'h' || buf[3] != 'd') return false;

    // Chunk length must be 6
    uint32_t chunkLen = ((uint32_t)buf[4]  << 24) |
                        ((uint32_t)buf[5]  << 16) |
                        ((uint32_t)buf[6]  <<  8) |
                         (uint32_t)buf[7];
    if (chunkLen != 6) return false;

    _format     = ((uint16_t)buf[8]  << 8) | buf[9];
    _numTracks  = ((uint16_t)buf[10] << 8) | buf[11];
    uint16_t td = ((uint16_t)buf[12] << 8) | buf[13];

    // Only metrical time (bit 15 = 0) is supported
    if (td & 0x8000) return false;
    _ticksPerQN = td;

    if (_format > 1)              return false; // SMF2 not supported
    if (_numTracks > MIDI_MAX_TRACKS) _numTracks = MIDI_MAX_TRACKS;

    return true;
}

// Scan through the file to find all MTrk chunk offsets
bool MidiPlayer::_scanTrackOffsets()
{
    _activeTrackCount = 0;
    uint32_t pos = 14; // After MThd

    for (uint16_t t = 0; t < _numTracks; t++) {
        if (!_file.seek(pos)) return false;

        uint8_t hdr[8];
        if (_file.read(hdr, 8) != 8) return false;

        if (hdr[0] != 'M' || hdr[1] != 'T' ||
            hdr[2] != 'r' || hdr[3] != 'k') return false;

        uint32_t len = ((uint32_t)hdr[4] << 24) |
                       ((uint32_t)hdr[5] << 16) |
                       ((uint32_t)hdr[6] <<  8) |
                        (uint32_t)hdr[7];

        TrackState& tr   = _tracks[t];
        tr.fileOffset    = pos + 8; // Data starts after 8-byte chunk header
        tr.length        = len;
        tr.bytesRead     = 0;
        tr.runningStatus = 0;
        tr.active        = true;

        // Read the first delta time to populate nextEventTick
        uint32_t delta = 0;
        if (!_readVLQ(t, delta)) return false;
        tr.nextEventTick = delta;

        pos += 8 + len;
        _activeTrackCount++;
    }

    return true;
}

// -----------------------------------------------------------------------
// Track file I/O
// -----------------------------------------------------------------------

bool MidiPlayer::_trackReadByte(uint8_t idx, uint8_t& out)
{
    TrackState& tr = _tracks[idx];
    if (tr.bytesRead >= tr.length) return false;

    uint32_t absPos = tr.fileOffset + tr.bytesRead;
    if (!_file.seek(absPos)) return false;

    int b = _file.read();
    if (b < 0) return false;

    tr.bytesRead++;
    out = (uint8_t)b;
    return true;
}

bool MidiPlayer::_trackReadBytes(uint8_t idx, uint8_t* buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        if (!_trackReadByte(idx, buf[i])) return false;
    }
    return true;
}

bool MidiPlayer::_trackPeekByte(uint8_t idx, uint8_t& out)
{
    TrackState& tr   = _tracks[idx];
    uint32_t savedBR = tr.bytesRead;
    if (!_trackReadByte(idx, out)) return false;
    tr.bytesRead = savedBR; // Restore position
    return true;
}

// -----------------------------------------------------------------------
// Variable-length quantity
// -----------------------------------------------------------------------

bool MidiPlayer::_readVLQ(uint8_t idx, uint32_t& out)
{
    out = 0;
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t b;
        if (!_trackReadByte(idx, b)) return false;
        out = (out << 7) | (b & 0x7F);
        if (!(b & 0x80)) return true;
    }
    return false; // VLQ > 4 bytes — malformed
}

// -----------------------------------------------------------------------
// Event processing
// -----------------------------------------------------------------------

/**
 * Processes the NEXT event in the track:
 *   1. Dispatches the event (writes MIDI bytes or handles meta/sysex).
 *   2. Reads the FOLLOWING delta time and advances nextEventTick.
 * Returns false when the track is exhausted (End of Track meta event
 * or a read error).
 */
bool MidiPlayer::_processTrack(uint8_t idx)
{
    TrackState& tr = _tracks[idx];
    if (!tr.active) return false;

    // Peek at status byte
    uint8_t b;
    if (!_trackPeekByte(idx, b)) return false;

    uint8_t status;

    if (b & 0x80) {
        // New status byte
        _trackReadByte(idx, status);
        tr.runningStatus = (status < 0xF0) ? status : 0; // Running status not valid for system messages
    } else {
        // Running status — reuse previous status
        if (tr.runningStatus == 0) return false; // Invalid
        status = tr.runningStatus;
    }

    // Dispatch
    if (status == 0xFF) {
        _handleMeta(idx);
        // Check if track was marked inactive (End of Track)
        if (!tr.active) return false;
    } else if (status == 0xF0 || status == 0xF7) {
        _handleSysEx(idx, status);
    } else {
        _handleMidiEvent(idx, status);
    }

    // Read next delta time and advance nextEventTick
    uint32_t delta = 0;
    if (!_readVLQ(idx, delta)) {
        tr.active = false;
        return false;
    }
    tr.nextEventTick += delta;

    return true;
}

// -----------------------------------------------------------------------
// Meta event handler
// -----------------------------------------------------------------------

void MidiPlayer::_handleMeta(uint8_t idx)
{
    TrackState& tr = _tracks[idx];

    // Status 0xFF was already consumed — read type byte
    uint8_t metaType;
    if (!_trackReadByte(idx, metaType)) { tr.active = false; return; }

    // Read length VLQ
    uint32_t len = 0;
    if (!_readVLQ(idx, len)) { tr.active = false; return; }

    switch (metaType) {

    case 0x51: { // Set Tempo
        if (len != 3) break;
        uint8_t t[3];
        if (!_trackReadBytes(idx, t, 3)) { tr.active = false; return; }
        _tempo = ((uint32_t)t[0] << 16) |
                 ((uint32_t)t[1] <<  8) |
                  (uint32_t)t[2];
        return; // Early return — no skip needed
    }

    case 0x2F: // End of Track
        // Skip any remaining bytes (usually 0)
        tr.bytesRead += len;
        tr.active     = false;
        return;

    default:
        // All other meta events (text, key sig, time sig, etc.)
        // are silently skipped
        break;
    }

    // Skip data bytes
    tr.bytesRead += len;
}

// -----------------------------------------------------------------------
// SysEx handler
// -----------------------------------------------------------------------

void MidiPlayer::_handleSysEx(uint8_t idx, uint8_t type)
{
    TrackState& tr = _tracks[idx];

    uint32_t len = 0;
    if (!_readVLQ(idx, len)) { tr.active = false; return; }

    // Forward SysEx to output
    _writeByte(type);

    for (uint32_t i = 0; i < len; i++) {
        uint8_t b;
        if (!_trackReadByte(idx, b)) { tr.active = false; return; }
        _writeByte(b);
    }
}

// -----------------------------------------------------------------------
// Channel MIDI event handler
// -----------------------------------------------------------------------

void MidiPlayer::_handleMidiEvent(uint8_t idx, uint8_t status)
{
    TrackState& tr = _tracks[idx];

    // How many data bytes follow?
    uint8_t dataLen = kMidiDataBytes[(status >> 4) & 0x07];

    // Write status byte
    _writeByte(status);

    // Read and write data bytes
    for (uint8_t i = 0; i < dataLen; i++) {
        uint8_t b;
        if (!_trackReadByte(idx, b)) { tr.active = false; return; }

        // Note On with velocity 0 is semantically Note Off — pass through as-is;
        // the receiving device handles this correctly.
        _writeByte(b);
    }
}

// -----------------------------------------------------------------------
// Timing
// -----------------------------------------------------------------------

uint32_t MidiPlayer::_usPerTick() const
{
    // µs per tick = tempo (µs/QN) / ticksPerQN
    if (_ticksPerQN == 0) return 1;
    return _tempo / _ticksPerQN;
}
