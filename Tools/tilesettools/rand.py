#!/usr/bin/env python3

# rand.py
# Exporter/importer for tileset randomization data.

import struct
from typing import BinaryIO

HEADER_STRUCT = struct.Struct('>BBxx')
RANDENTRY_STRUCT = struct.Struct('>4BI')
RANDTILE_STRUCT = struct.Struct('>H')

VERSION = 1
HEADER_SIZE = HEADER_STRUCT.size
RANDENTRY_SIZE = RANDENTRY_STRUCT.size
RANDTILE_SIZE = RANDTILE_STRUCT.size

JSON_HEADER = 'entries'
JSON_TILENUM = 'tilenum'
JSON_NORMTYPE = 'normaltype'
JSON_SPECTYPE = 'specialtype'
JSON_VALUEPOOL = 'values'

NORMAL_TYPES = {
    0: 'NONE',
    1: 'HORIZONTAL',
    2: 'VERTICAL',
    3: 'BOTH'
}

SPECIAL_TYPES = {
    0: 'NONE',
    1: 'VDOUBLETOP',
    2: 'VDOUBLEBOTTOM',
    3: 'HDOUBLELEFT',
    4: 'HDOUBLERIGHT',
}

def decode(data: BinaryIO) -> dict:

    # Parse header
    _, numEntries = HEADER_STRUCT.unpack(data.read(HEADER_SIZE))

    # Initialize JSON
    jsonfile = {JSON_HEADER: []}

    # Parse entries
    for i in range(numEntries):

        # Reach the entry and parse it
        data.seek(HEADER_SIZE + RANDENTRY_SIZE * i, 0)
        (tileNum,
        normalType,
        specialType,
        arrayLen,
        tileArrayOffs) = RANDENTRY_STRUCT.unpack(data.read(RANDENTRY_SIZE))

        # Initialize the new entry
        newEntry = {
            JSON_TILENUM: hex(tileNum),
            JSON_NORMTYPE: NORMAL_TYPES[normalType].title(),
            JSON_SPECTYPE: SPECIAL_TYPES[specialType].title(),
            JSON_VALUEPOOL: []
        }

        # Go to the tile array and parse it
        data.seek(tileArrayOffs, 0)
        for _ in range(arrayLen):
            newEntry[JSON_VALUEPOOL].append(hex(*RANDTILE_STRUCT.unpack(data.read(RANDTILE_SIZE))))

        # Append the new entry
        jsonfile[JSON_HEADER].append(newEntry)

    # Return result
    return jsonfile


def encode(data: dict) -> bytes:

    # Initialize lists
    randentries = []
    randtiles = []
    currTileOffs = HEADER_SIZE + RANDENTRY_SIZE * len(data[JSON_HEADER])

    # Parse JSON
    for entry in data[JSON_HEADER]:

        # Reuse tile data if duplicated
        try:
            idx = randtiles.index(entry[JSON_VALUEPOOL])
            finalOffs = HEADER_SIZE + RANDENTRY_SIZE * len(data[JSON_HEADER])
            for i in range(idx):
                finalOffs += len(randtiles[i]) * RANDTILE_SIZE

        # Else append it
        except ValueError:
            finalOffs = currTileOffs
            currTileOffs += len(entry[JSON_VALUEPOOL]) * RANDTILE_SIZE
            randtiles.append(entry[JSON_VALUEPOOL])

        # Encode the entry
        newEntry = RANDENTRY_STRUCT.pack(
            int(entry[JSON_TILENUM], 16),
            list(NORMAL_TYPES.values()).index(entry[JSON_NORMTYPE].upper()),
            list(SPECIAL_TYPES.values()).index(entry[JSON_SPECTYPE].upper()),
            len(entry[JSON_VALUEPOOL]),
            finalOffs)

        # Append it
        randentries.append(newEntry)

    # Assemble the file and return it
    binfile = HEADER_STRUCT.pack(VERSION, len(data[JSON_HEADER]))
    for entry in randentries:
        binfile += entry
    for entry in randtiles:
        for tile in entry:
            binfile += RANDTILE_STRUCT.pack(int(tile, 16))
    return binfile
