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
#include "Channel.h"


class NoteList
{
public:
    NoteList();
    virtual ~NoteList();
    Note* addNote();
    void moveToEnd(Note* note);
    void removeNote(Note* note);
    void removeAllNotes();
    Note* getNote(uint32_t channel, uint32_t noteNum);
    Note* firstNote;
    Note* newNote;
    static constexpr uint32_t MAX_NOTES = 64;
    uint32_t activeNotes = 0;

private:
    void buildLinks();
    void moveBefore(Note* noteToMove, Note* noteToInsertBefore);
    Note unorderedNotes[MAX_NOTES + 1];
};

#endif /* NOTELIST_H_ */
