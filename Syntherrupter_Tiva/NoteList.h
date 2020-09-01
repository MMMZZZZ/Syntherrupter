/*
 * NoteList.h
 *
 *  Created on: 21.08.2020
 *      Author: Max Zuidberg
 */

#ifndef NOTELIST_H_
#define NOTELIST_H_


#include "InterrupterConfig.h"
#include "Note.h"


class NoteList
{
public:
    NoteList();
    virtual ~NoteList();
    Note* addNote();
    void removeNote(Note* note);
    void removeAllNotes();
    Note* getNote(uint32_t channel, uint32_t noteNum);
    void setMaxVoices(uint32_t maxVoices)
    {
        if (maxVoices > MAX_NOTES)
        {
            maxVoices = MAX_NOTES;
        }
        this->maxVoices = maxVoices;
        buildLinks();
    };
    Note* firstNote;
    Note* newNote;
    uint32_t activeNotes = 0;
    static constexpr uint32_t MAX_NOTES = 64;

private:
    void buildLinks();
    float maxOntimeUS    = 10;
    float maxDuty        = 0.01f;
    bool limiterActive   = false;
    uint32_t maxVoices   = 8;
    Note unorderedNotes[MAX_NOTES + 1];
};

#endif /* NOTELIST_H_ */
