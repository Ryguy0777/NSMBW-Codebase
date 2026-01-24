#!/usr/bin/env python3

# anim.py
# Exporter/importer for animated tile metadata.

import struct
from typing import BinaryIO

HEADER_STRUCT = struct.Struct('>BB')
ANIMENTRY_STRUCT = struct.Struct('>HB?')

VERSION = 1
HEADER_SIZE = HEADER_STRUCT.size
ANIMENTRY_SIZE = ANIMENTRY_STRUCT.size

JSON_HEADER = 'entries'
JSON_TILENUM = 'tilenum'
JSON_REVERSE = 'reverse'
JSON_DELAYS = 'delays'


def decode(data: BinaryIO) -> dict:

    # Initialize JSON file
    jsonfile = {JSON_HEADER: []}

    # Get version and numentries (former is discarded)
    _, numEntries = HEADER_STRUCT.unpack(data.read(HEADER_SIZE))

    # Parse entries
    for _ in range(numEntries):

        # Read the entry data
        delayOffset, tileNum, reverse = ANIMENTRY_STRUCT.unpack(data.read(ANIMENTRY_SIZE))
        newEntry = {JSON_TILENUM: tileNum, JSON_REVERSE: reverse, JSON_DELAYS: []}

        # Save position and go to the delay offset
        posBak = data.tell()
        data.seek(delayOffset, 0)

        # Read delay values until a null byte is encountered
        while True:
            delay = data.read(1)
            if delay == b'\0':
                break
            newEntry[JSON_DELAYS].append(int.from_bytes(delay, 'big'))

        # Restore file position
        data.seek(posBak, 0)

        # Append completed entry
        jsonfile[JSON_HEADER].append(newEntry)

    # Return result
    return jsonfile


def encode(data: dict) -> bytes:

    # Initialize loop
    animEntries = []
    delayEntries = []
    currOffset = HEADER_SIZE + ANIMENTRY_SIZE * len(data[JSON_HEADER])

    # Parse JSON
    for entry in data[JSON_HEADER]:

        # Encode the delay and append null byte
        delayEntry = bytes(entry[JSON_DELAYS]) + b'\0'

        # Look for an equal delay array in order to reuse it and save space
        equalDelayEntryIdx = -1
        for i, delay in enumerate(delayEntries):
            if delay == delayEntry:
                equalDelayEntryIdx = i
                break

        # If not found, append the new entry and update the offset
        if equalDelayEntryIdx == -1:
            finalOffset = currOffset
            delayEntries.append(delayEntry)
            currOffset += len(delayEntry)

        # Else calculate the offset of the existing entry
        else:
            finalOffset = HEADER_SIZE + ANIMENTRY_SIZE * len(data[JSON_HEADER])
            for i in range(equalDelayEntryIdx):
                finalOffset += len(delayEntries[i])

        # Create the anim entry
        animEntries.append(ANIMENTRY_STRUCT.pack(finalOffset, entry[JSON_TILENUM], entry[JSON_REVERSE]))

    # Assemble the data and return it
    binfile = HEADER_STRUCT.pack(VERSION, len(animEntries))
    for entry in animEntries:
        binfile += entry
    for entry in delayEntries:
        binfile += entry
    return binfile
