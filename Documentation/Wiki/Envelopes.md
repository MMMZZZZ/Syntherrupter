# Build-in Envelopes

[Back to the wiki main page](README.md#readme)

Syntherrupter comes with a bunch of envelopes included in its firmware. Envelopes allow you to modify the sound (soft & long, sharp & short, ...). I made some [forum posts](README.md#external-content) that go into greater detail.

The sound can be selected by the MIDI command "Program Change". In your MIDI software you can select it by changing the instrument of the channel. Program 0 is f.ex. called "Accoustic Grand Piano". Some MIDI softwares only list the name, others list the program number, too. Syntherrupter doesn't care about the name, since its programs don't match those labels. The meaning of each program for Syntherrupter is listed below.

* Program 0 and all unlisted: No envelope. Constant ontime (except for other effects like modulation).
* Program 1: Roughly like a piano. Attack peaks to 1 (= not exceeding the given ontime)
* Program 2: Sloooow rise, slow fall. Good for soft background, but too slow for shorter notes. Attack peaks to 1 (= not exceeding the given ontime)
* Program 3: Like program 1, but with a small step, to make shorter notes more audible. Attack peaks to 1 (= not exceeding the given ontime)
* Program 4: Twice as fast as program 1. Attack peaks to 1 (= not exceeding the given ontime)
* Program 5: Forced Staccato. All notes are always short. Attack peaks to 1 (= not exceeding the given ontime)
* Program 6: Forced Legato. All notes are hold for quite some time. Attack peaks to 1 (= not exceeding the given ontime)
* Program 7: Like program 2, but with a faster release. Attack peaks to 1 (= not exceeding the given ontime)
* Program 8: Roughly like a piano. Attack peaks to 2 (= doublingt he given ontime for a few milliseconds)
* Program 9: Forced Staccato with slight sustain. Attack peaks to 3 (= tripling the given ontime for a few milliseconds)
* Program 10: My best approximation of an actual piano. Attack peaks to 2 (= doubling the given ontime for a few milliseconds)
* Program 11-19: Same as programs 1-9 but with exponential instead of linear curves between the points. No other changes made, so nothing special *yet*.

If you wonder why you would want to exceed the given ontime, it is an "efficient" way to get louder notes even with high voice count without tripping your circuit breaker. Since the ontime drops pretty fast after the attack you can consider the ontime on the display more like an average ontime. Note that all these envelope settings do not allow to exceed your [coil limits](/Documentation/Wiki/UI/Coil%20Limits.md#readme). 

The programs 1-63 can be modified by the user using the [Envelope Editor](/Documentation/Wiki/UI/Envelope.md#readme). The programs 20-39 are stored in EEPROM. Unless modified by the user, they are like program 0, no envelope (constant ontime). All other programs will be reset on startup.
