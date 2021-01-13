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
    if (++activeNotes > maxVoices)
    {
        removeNote(firstNote);
    }
    return targetNote;
}

void NoteList::removeNote(Note* note)
{
    if (note)
    {
        activeNotes--;

        note->number = 128;
        note->nextChnNote = 0;
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
            note->prevNote->nextNote = note->nextNote;
            note->nextNote->prevNote = note->prevNote;
            note->nextNote           = firstNote;
            note->prevNote           = firstNote->prevNote;
            note->prevNote->nextNote = note;
            note->nextNote->prevNote = note;
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
        if (noteNum == note->number && channel == note->channel)
        {
            return note;
        }
        note = note->nextNote;
    }
    return 0;
};

