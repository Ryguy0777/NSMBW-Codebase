#!/usr/bin/env python3

# profoverride.py
# Exporter/importer for custom profile override information.

import struct
from typing import BinaryIO

HEADER_STRUCT = struct.Struct('>5B3x')
PROFENTRY_STRUCT = struct.Struct('>BxHBBxx5fI')
COLORENTRY_STRUCT = struct.Struct('>I')
SCALEENTRY_STRUCT = struct.Struct('>ff')

VERSION = 1
HEADER_SIZE = HEADER_STRUCT.size
PROFENTRY_SIZE = PROFENTRY_STRUCT.size
COLORENTRY_SIZE = COLORENTRY_STRUCT.size
SCALEENTRY_SIZE = SCALEENTRY_STRUCT.size
DEFAULT_RAIL_COLORS = [0xC8966496, 0xA07878AA, 0xFAC86842, 0xF0AAAAFF]
DEFAULT_RAIL_SRTS = [
    [0.0, 0.0],
    [0.1, 0.0],
    [0.02, -0.45],
    [0.0, 0.0],
    [0.25, -0.5],
    [8.0, -7.0],
    [-8.0, -7.0],
    [-8.0, -7.0],
    [-8.0, -15.0],
    [-8.0, -15.0],
    [-8.0, -7.0],
    [-8.0, -7.0],
    [-16.0, -7.0],
    [-16.0, -7.0],
    [-8.0, -8.0],
    [-16.0, -15.0],
    [-16.0, -15.0],
    [-16.0, -15.0],
    [-16.0, -15.0],
    [-8.0, -7.0],
    [-8.0, -7.0],
    [-8.0, -7.0],
    [-8.0, -7.0],
    [5.0, 1.0],
    [0.0, -5.0],
    [0.0, 5.0],
    [-5.0, 1.0],
    [6.0, -5.0],
    [7.0, 8.0],
    [-7.0, -7.0],
    [-6.0, 7.0],
    [0.0, 0.0],
    [0.0, 0.0],
    [0.0, 0.0],
    [0.0, 0.0],
    [0.0, 0.0],
    [0.0, 0.0]
]

JSON_PROFVER = 'profver'
JSON_HEADER = 'entries'
JSON_TILENUM = 'tilenum'
JSON_PROFID = 'profileid'
JSON_RAILCOLOR = 'railcolor'
JSON_RAILTEXSRT = 'railsrt'
JSON_OFFSET = 'offset'
JSON_SCALE = 'scale'
JSON_SETTINGS = 'settings'


def hexwithpad(val: int) -> str:
    conv = hex(val)[2:]
    return '0x' + '0' * (8 - len(conv)) + conv


def decode(data: BinaryIO) -> dict:

    # Read header
    _, profileIdVersion, numEntries, numColors, _ = HEADER_STRUCT.unpack(data.read(HEADER_SIZE))

    # Get file positions
    colorTableOffs = HEADER_SIZE + numEntries * PROFENTRY_SIZE
    scaleTableOffs = colorTableOffs + numColors * COLORENTRY_SIZE

    # Initialize JSON
    jsonfile = {JSON_PROFVER: profileIdVersion, JSON_HEADER: []}

    # Parse entries
    for i in range(numEntries):

        # Move to position
        data.seek(HEADER_SIZE + i * PROFENTRY_SIZE, 0)

        # Read profentry
        (tileNum,
        profileId,
        railColorIdx,
        railTexSrtIdx,
        xOffs,
        yOffs,
        zOffs,
        xScale,
        yScale,
        settings) = PROFENTRY_STRUCT.unpack(data.read(PROFENTRY_SIZE))

        # Start building the new entry
        newEntry = {
            JSON_TILENUM: tileNum,
            JSON_PROFID: profileId,
            JSON_OFFSET: [xOffs, yOffs, zOffs],
            JSON_SCALE: [xScale, yScale],
            JSON_SETTINGS: hexwithpad(settings)
        }

        # Get the rail colors
        # If the index isn't zero, add the field
        # If the index falls into the defaults, pull the values from there
        if railColorIdx:
            if railColorIdx <= len(DEFAULT_RAIL_COLORS):
                newEntry[JSON_RAILCOLOR] = hexwithpad(DEFAULT_RAIL_COLORS[railColorIdx-1])
            else:
                data.seek(colorTableOffs + (railColorIdx - len(DEFAULT_RAIL_COLORS) - 1) * COLORENTRY_SIZE, 0)
                newEntry[JSON_RAILCOLOR] = hexwithpad(*COLORENTRY_STRUCT.unpack(data.read(COLORENTRY_SIZE)))

        # Repeat for rail texture info
        if railTexSrtIdx:
            if railTexSrtIdx <= len(DEFAULT_RAIL_SRTS):
                newEntry[JSON_RAILTEXSRT] = DEFAULT_RAIL_SRTS[railTexSrtIdx-1]
            else:
                data.seek(scaleTableOffs + (railTexSrtIdx - len(DEFAULT_RAIL_SRTS) - 1) * SCALEENTRY_SIZE, 0)
                newEntry[JSON_RAILTEXSRT] = list(SCALEENTRY_STRUCT.unpack(data.read(SCALEENTRY_SIZE)))

        # Add the entry
        jsonfile[JSON_HEADER].append(newEntry)

    # Return result
    return jsonfile


def encode(data: dict) -> bytes:

    # Initialize temporary containers
    profentries = []
    colors = []
    scales = []

    # Parse file
    for entry in data[JSON_HEADER]:

        # Check if colors were defined
        currColorIdx = 0
        currColor = entry.get(JSON_RAILCOLOR, '')
        if currColor:

            # Convert to int for lookups
            currColor = int(currColor, 16)

            # Check if the color is a default or is already in the custom color list
            try:
                currColorIdx = DEFAULT_RAIL_COLORS.index(currColor) + 1
            except ValueError:
                try:
                    currColorIdx = colors.index(currColor) + len(DEFAULT_RAIL_COLORS) + 1
                except ValueError:
                    colors.append(currColor)
                    currColorIdx = len(colors) + len(DEFAULT_RAIL_COLORS)

        # Check if scales were defined
        currScaleIdx = 0
        currScale = entry.get(JSON_RAILTEXSRT, [])
        if currScale:

            # Check if the scale is a default or is already in the custom scale list
            try:
                currScaleIdx = DEFAULT_RAIL_SRTS.index(currScale) + 1
            except ValueError:
                try:
                    currScaleIdx = scales.index(currScale) + len(DEFAULT_RAIL_SRTS) + 1
                except ValueError:
                    scales.append(currScale)
                    currScaleIdx = len(scales) + len(DEFAULT_RAIL_SRTS)

        # Pack the entry
        newEntry = PROFENTRY_STRUCT.pack(
        entry[JSON_TILENUM],
        entry[JSON_PROFID],
        currColorIdx,
        currScaleIdx,
        *entry[JSON_OFFSET],
        *entry[JSON_SCALE],
        int(entry[JSON_SETTINGS], 16)
        )

        # Add it to the list
        profentries.append(newEntry)

    # Assemble the file and return it
    binfile = HEADER_STRUCT.pack(VERSION,
        data[JSON_PROFVER],
        len(profentries),
        len(colors),
        len(scales))

    for entry in profentries:
        binfile += entry
    for entry in colors:
        binfile += COLORENTRY_STRUCT.pack(entry)
    for entry in scales:
        binfile += SCALEENTRY_STRUCT.pack(*entry)
    return binfile
