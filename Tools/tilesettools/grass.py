#!/usr/bin/env python3

# grass.py
# Exporter/importer for custom grass file information.

import struct
from typing import BinaryIO

HEADER_STRUCT = struct.Struct('>4B')
GRASSENTRY_STRUCT = struct.Struct('>BB')

VERSION = 1
HEADER_SIZE = HEADER_STRUCT.size
GRASSENTRY_SIZE = GRASSENTRY_STRUCT.size

JSON_FLOWERSTYLE = 'flowerfile'
JSON_GRASSSTYLE = 'grassfile'
JSON_HEADER = 'entries'
JSON_TILENUM = 'tilenum'
JSON_FLOWERVAL = 'flowertype'
JSON_GRASSVAL = 'grasstype'


def decode(data: BinaryIO) -> dict:

    # Decode header (version is discarded)
    _, numEntries, flowerStyle, grassStyle = HEADER_STRUCT.unpack(data.read(HEADER_SIZE))

    # Initialize JSON file
    jsonfile = {JSON_FLOWERSTYLE: flowerStyle, JSON_GRASSSTYLE: grassStyle, JSON_HEADER: []}

    # Parse entries
    for _ in range(numEntries):

        # Read the entry data
        tileNum, combinedVal = GRASSENTRY_STRUCT.unpack(data.read(GRASSENTRY_SIZE))
        newEntry = {JSON_TILENUM: tileNum, JSON_FLOWERVAL: combinedVal >> 4, JSON_GRASSVAL: combinedVal & 0xF}

        # Append completed entry
        jsonfile[JSON_HEADER].append(newEntry)

    # Return result
    return jsonfile


def encode(data: dict) -> bytes:

    # Assemble the file and return it
    binfile = HEADER_STRUCT.pack(VERSION, len(data[JSON_HEADER]), data[JSON_FLOWERSTYLE], data[JSON_GRASSSTYLE])
    for entry in data[JSON_HEADER]:
        binfile += GRASSENTRY_STRUCT.pack(entry[JSON_TILENUM], (entry[JSON_FLOWERVAL] & 0xF) << 4 | entry[JSON_GRASSVAL] & 0xF)
    return binfile
