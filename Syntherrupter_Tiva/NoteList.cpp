/*
 * NoteList.cpp
 *
 *  Created on: 21.08.2020
 *      Author: Max Zuidberg
 */


#include <NoteList.h>


NoteList::NoteList()
{
    // For debugging purposes only.
    for (uint32_t note = 0; note < MAX_NOTES + 1; note++)
    {
        unorderedNotes[note].id = note;
    }
    buildLinks();
}

NoteList::~NoteList()
{
    // Auto-generated destructor stub
}

Note* NoteList::addNote()
{
    Note* targetNote = newNote;
    newNote          = newNote->nextNote;
    if (++activeNotes > MAX_NOTES)
    {
        removeNote(firstNote);
    }
    return targetNote;
}

void NoteList::moveToEnd(Note* note)
{
    if (note == newNote->prevNote)
    {
        // Note is already at the end. This case must be caught first!
        return;
    }
    if (note == firstNote)
    {
        // Adjust pointer to firstNote.
        firstNote = note->nextNote;
    }
    moveBefore(note, newNote);
}

void NoteList::removeNote(Note* note)
{
    if (note)
    {
        note->channel->removeNote(note);

        activeNotes--;

        note->number = 128;
        for (uint32_t coil = 0; coil < COIL_COUNT; coil++)
        {
            if (note->assignedTones[coil])
            {
                note->assignedTones[coil]->remove(note);
                note->assignedTones[coil] = 0;
            }
        }

        if (note == firstNote)
        {
            firstNote = firstNote->nextNote;
        }
        else if (note->nextNote == firstNote)
        {
            newNote = note;
        }
        else
        {
            moveBefore(note, firstNote);
        }
    }
}

void NoteList::removeAllNotes()
{
    while (activeNotes)
    {
        removeNote(firstNote);
    }
}

void NoteList::buildLinks()
{
    /*
     * This is a doubly linked list, but a circular one. This makes it easier
     * to handle "overflows" of the list but (seems) to require one additional
     * list element that won't be used/moved. Maybe a bug; haven't really
     * understood the issue yet. TODO!
     */
    for (uint32_t note = 0; note < MAX_NOTES; note++)
    {
        unorderedNotes[note].nextNote = &(unorderedNotes[note + 1]);
        unorderedNotes[note + 1].prevNote = &(unorderedNotes[note]);
    }
    firstNote = &(unorderedNotes[0]);
    firstNote->prevNote = &(unorderedNotes[MAX_NOTES]);
    firstNote->prevNote->nextNote = firstNote;
    newNote  = firstNote;
}

Note* NoteList::getNote(uint32_t channel, uint32_t noteNum)
{
    Note* note = firstNote;
    for (uint32_t i = 0; i < MAX_NOTES; i++)
    {
        if (i >= activeNotes)
        {
            return 0;
        }
        if (noteNum == note->number && channel == note->channel->number)
        {
            return note;
        }
        note = note->nextNote;
    }
    return 0;
};

void NoteList::moveBefore(Note* noteToMove, Note* noteToInsertBefore)
{
    noteToMove->prevNote->nextNote = noteToMove->nextNote;
    noteToMove->nextNote->prevNote = noteToMove->prevNote;
    noteToMove->nextNote           = noteToInsertBefore;
    noteToMove->prevNote           = noteToInsertBefore->prevNote;
    noteToMove->prevNote->nextNote = noteToMove;
    noteToMove->nextNote->prevNote = noteToMove;
}

