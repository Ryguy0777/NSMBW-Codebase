#!/usr/bin/env python

import archive
import lz77
import json
import os, os.path
import shutil
import struct
import sys
import threading
import time
from xml.etree import ElementTree as etree

from ctypes import create_string_buffer

try:
    from PyQt5 import QtCore, QtGui, QtWidgets
except ImportError:
    from PySide2 import QtCore, QtGui, QtWidgets
Qt = QtCore.Qt

#try:
#    from PIL import Image
#except ImportError:
#    print("You have to install Pillow!")
#    os._exit(0)

from widgets.grass_widget import FlowerGrassWidget
from widgets.prof_widget import ProfileOverrideWidget
from widgets.rand_widget import RandTileWidget
from widgets.anim_widget import AnimTileWidget

try:
    from tools import grass
    from tools import profoverride
    from tools import rand
    from tools import anim
    del grass
    del profoverride
    del rand
    del anim
except ImportError:
    app = QtWidgets.QApplication(sys.argv)

    msg = QtWidgets.QMessageBox()
    msg.setIcon(QtWidgets.QMessageBox.Critical)
    msg.setText("Tileset Tools not found!")
    msg.setInformativeText('In order to use Puzzle Next, you must put Tileset Tools in the /tools folder.<br><br>See README.md for more information.')

    msg.setWindowTitle("Puzzle Next")
    msg.setWindowIcon(QtGui.QIcon("Icons/about.png"))
    msg.exec_()


try:
    import nsmblib
    HaveNSMBLib = True
except ImportError:
    HaveNSMBLib = False

if hasattr(QtCore, 'pyqtSlot'): # PyQt
    QtCoreSlot = QtCore.pyqtSlot
    QtCoreSignal = QtCore.pyqtSignal
else: # PySide2
    QtCoreSlot = QtCore.Slot
    QtCoreSignal = QtCore.Signal


########################################################
# To Do:
#
#   - Object Editor
#       - Moving objects around
#
#   - Make UI simpler for Pop
#   - fix up conflicts with different types of parameters
#   - C speed saving
#   - quick settings for applying to mulitple slopes
#
########################################################


Tileset = None


def module_path():
    """
    This will get us the program's directory, even if we are frozen
    using PyInstaller
    """

    if hasattr(sys, 'frozen') and hasattr(sys, '_MEIPASS'):  # PyInstaller
        if sys.platform == 'darwin':  # macOS
            # sys.executable is /x/y/z/puzzle.app/Contents/MacOS/puzzle
            # We need to return /x/y/z/puzzle.app/Contents/Resources/

            macos = os.path.dirname(sys.executable)
            if os.path.basename(macos) != 'MacOS':
                return None

            return os.path.join(os.path.dirname(macos), 'Resources')

    if __name__ == '__main__':
        return os.path.dirname(os.path.abspath(sys.argv[0]))

    return None


#############################################################################################
########################## Tileset Class and Tile/Object Subclasses #########################

class TilesetClass():
    '''Contains Tileset data. Inits itself to a blank tileset.
    Methods: addTile, removeTile, addObject, removeObject, clear'''

    class Tile():
        def __init__(self, image, noalpha, bytelist):
            '''Tile Constructor'''

            self.image = image
            self.noalpha = noalpha
            self.byte0 = bytelist[0]
            self.byte1 = bytelist[1]
            self.byte2 = bytelist[2]
            self.byte3 = bytelist[3]
            self.byte4 = bytelist[4]
            self.byte5 = bytelist[5]
            self.byte6 = bytelist[6]
            self.byte7 = bytelist[7]


    class Object():

        def __init__(self, height, width, uslope, lslope, tilelist):
            '''Tile Constructor'''

            self.upperslope = uslope
            self.lowerslope = lslope

            assert (width, height) != 0

            self.height = height
            self.width = width

            self.tiles = tilelist

            self.determineRepetition()

            if self.repeatX or self.repeatY:
                height = len(self.tiles)
                if self.height != height:
                    print("WARNING: Object height mismatches with the number of rows in the object.")
                    self.height = height

                width = max(len(self.tiles[y]) for y in range(self.height))
                if self.width  != width:
                    print("WARNING: Object width mismatches with the maximum number of columns in the object.")
                    self.width = width

                if self.repeatX:
                    self.determineRepetitionFinalize()

            else:
                # Fix a bug from a previous version of Puzzle
                # where the actual width and height would
                # mismatch with the number of tiles for the object

                self.fillMissingTiles()

            self.tilingMethodIdx = self.determineTilingMethod()


        def determineRepetition(self):
            self.repeatX = []
            self.repeatY = []

            if self.upperslope[0] != 0:
                return

            #### Find X Repetition ####
            # You can have different X repetitions between rows, so we have to account for that

            for y in range(self.height):
                repeatXBn = -1
                repeatXEd = -1

                for x in range(len(self.tiles[y])):
                    if self.tiles[y][x][0] & 1 and repeatXBn == -1:
                        repeatXBn = x

                    elif not self.tiles[y][x][0] & 1 and repeatXBn != -1:
                        repeatXEd = x
                        break

                if repeatXBn != -1:
                    if repeatXEd == -1:
                        repeatXEd = len(self.tiles[y])

                    self.repeatX.append((y, repeatXBn, repeatXEd))

            #### Find Y Repetition ####

            repeatYBn = -1
            repeatYEd = -1

            for y in range(self.height):
                if len(self.tiles[y]) and self.tiles[y][0][0] & 2:
                    if repeatYBn == -1:
                        repeatYBn = y

                elif repeatYBn != -1:
                    repeatYEd = y
                    break

            if repeatYBn != -1:
                if repeatYEd == -1:
                    repeatYEd = self.height

                self.repeatY = [repeatYBn, repeatYEd]


        def determineRepetitionFinalize(self):
            if self.repeatX:
                # If any X repetition is present, fill in rows which didn't have X repetition set
                ## Should never happen, unless the tileset is broken
                ## Additionally, sort the list
                repeatX = []
                for y in range(self.height):
                    for row, start, end in self.repeatX:
                        if y == row:
                            repeatX.append([start, end])
                            break

                    else:
                        # Get the start and end X offsets for the row
                        start = 0
                        end = len(self.tiles[y])

                        repeatX.append([start, end])

                self.repeatX = repeatX


        def fillMissingTiles(self):
            realH = len(self.tiles)
            while realH > self.height:
                del self.tiles[-1]
                realH -= 1

            for row in self.tiles:
                realW = len(row)
                while realW > self.width:
                    del row[-1]
                    realW -= 1

            for row in self.tiles:
                realW = len(row)
                while realW < self.width:
                    row.append((0, 0, 0))
                    realW += 1

            while realH < self.height:
                self.tiles.append([(0, 0, 0) for _ in range(self.width)])
                realH += 1


        def createRepetitionX(self):
            self.repeatX = []

            for y in range(self.height):
                for x in range(len(self.tiles[y])):
                    self.tiles[y][x] = (self.tiles[y][x][0] | 1, self.tiles[y][x][1], self.tiles[y][x][2])

                self.repeatX.append([0, len(self.tiles[y])])


        def createRepetitionY(self, y1, y2):
            self.clearRepetitionY()

            for y in range(y1, y2):
                for x in range(len(self.tiles[y])):
                    self.tiles[y][x] = (self.tiles[y][x][0] | 2, self.tiles[y][x][1], self.tiles[y][x][2])

            self.repeatY = [y1, y2]


        def clearRepetitionX(self):
            self.fillMissingTiles()

            for y in range(self.height):
                for x in range(self.width):
                    self.tiles[y][x] = (self.tiles[y][x][0] & ~1, self.tiles[y][x][1], self.tiles[y][x][2])

            self.repeatX = []


        def clearRepetitionY(self):
            for y in range(self.height):
                for x in range(len(self.tiles[y])):
                    self.tiles[y][x] = (self.tiles[y][x][0] & ~2, self.tiles[y][x][1], self.tiles[y][x][2])

            self.repeatY = []


        def clearRepetitionXY(self):
            self.clearRepetitionX()
            self.clearRepetitionY()


        def determineTilingMethod(self):
            if self.upperslope[0] == 0x93:
                return 7

            elif self.upperslope[0] == 0x92:
                return 6

            elif self.upperslope[0] == 0x91:
                return 5

            elif self.upperslope[0] == 0x90:
                return 4

            elif self.repeatX and self.repeatY:
                return 3

            elif self.repeatY:
                return 2

            elif self.repeatX:
                return 1

            return 0


    def __init__(self):
        '''Constructor'''

        self.tiles = []
        self.objects = []
        self.animdata = {}
        self.unknownFiles = {}

        self.slot = 0
        self.placeNullChecked = False


    def addTile(self, image, noalpha, bytelist = (0, 0, 0, 0, 0, 0, 0, 0)):
        '''Adds an tile class to the tile list with the passed image or parameters'''

        self.tiles.append(self.Tile(image, noalpha, bytelist))


    def getUsedTiles(self):
        usedTiles = []

        for object in self.objects:
            for i in range(len(object.tiles)):
                for tile in object.tiles[i]:
                    if not tile[2] & 3 and Tileset.slot:  # Pa0 tile 0 used in another slot, don't count it
                        continue

                    if tile[1] not in usedTiles:
                        usedTiles.append(tile[1])

        return usedTiles


    def addObject(self, height = 1, width = 1,  uslope = [0, 0], lslope = [0, 0], tilelist = None, new = False):
        '''Adds a new object'''

        global Tileset

        if new:
            tilelist = [[(0, 0, Tileset.slot)]]

        self.objects.append(self.Object(height, width, uslope, lslope, tilelist))


    def removeObject(self, index):
        '''Removes an Object by Index number. Don't use this much, because we want objects to preserve their ID.'''

        self.objects.pop(index)


    def clear(self):
        '''Clears the tileset for a new file'''

        self.tiles = []
        self.objects = []
        self.animdata = {}
        self.unknownFiles = {}


#############################################################################################
######################### Palette for painting behaviours to tiles ##########################


class paletteWidget(QtWidgets.QWidget):

    def __init__(self, window):
        super(paletteWidget, self).__init__(window)


        # Core Types Radio Buttons and Tooltips
        self.coreType = QtWidgets.QGroupBox()
        self.coreType.setTitle('Core Type:')
        self.coreWidgets = []
        coreLayout = QtWidgets.QVBoxLayout()
        rowA = QtWidgets.QHBoxLayout()
        rowB = QtWidgets.QHBoxLayout()
        rowC = QtWidgets.QHBoxLayout()
        rowD = QtWidgets.QHBoxLayout()
        rowE = QtWidgets.QHBoxLayout()
        rowF = QtWidgets.QHBoxLayout()

        path = 'Icons/'

        self.coreTypes = [['Default', QtGui.QIcon(path + 'Core/Default.png'), 'The standard type for tiles.\n\nAny regular terrain or backgrounds\nshould be of generic type. It has no\n collision properties.'],
                     ['Slope', QtGui.QIcon(path + 'Core/Slope.png'), 'Defines a sloped tile\n\nSloped tiles have sloped collisions,\nwhich Mario can slide on.\n\nNote: Do NOT set slopes to have solid collision.'],
                     ['Reverse Slope', QtGui.QIcon(path + 'Core/RSlope.png'), 'Defines an upside-down slope.\n\nSloped tiles have sloped collisions,\nwhich Mario can slide on.\n\nNote: Do NOT set slopes to have solid collision.'],
                     ['Partial Block', QtGui.QIcon(path + 'Partial/Full.png'), 'Used for blocks with partial collisions.\n\nVery useful for Mini-Mario secret\nareas, but also for providing a more\naccurate collision map for your tiles.'],
                     ['Coin', QtGui.QIcon(path + 'Core/Coin.png'), 'Creates a coin.\n\nCoins have no solid collision,\nand when touched will disappear\nand increment the coin counter.'],
                     ['Explodable Block', QtGui.QIcon(path + 'Core/Explode.png'), 'Specifies blocks which can explode.\n\nThese blocks will shatter into componenent\npieces when hit by a bom-omb or meteor.\nThe pieces themselves may be hardcoded\nand must be included in the tileset.\nBehaviour may be sporadic.'],
                     ['Climable Grid', QtGui.QIcon(path + 'Core/Climb.png'), 'Creates terrain that can be climbed on.\n\nClimable terrain cannot be walked on.\nWhen Mario is overtop of a climable\ntile and the player presses up,\nMario will enter a climbing state.'],
                     ['Spike', QtGui.QIcon(path + 'Core/Spike.png'), 'Dangerous Spikey spikes.\n\nSpike tiles will damage Mario one hit\nwhen they are touched.'],
                     ['Pipe', QtGui.QIcon(path + 'Core/Pipe.png'), "Denotes a pipe tile.\n\nPipe tiles are specified according to\nthe part of the pipe. It's important\nto specify the right parts or\nentrances will not function correctly."],
                     ['Rails', QtGui.QIcon(path + 'Core/Rails.png'), 'Used for all types of rails.\n\nPlease note that Pa3_rail.arc is hardcoded\nto replace rails with 3D models.'],
                     ['Conveyor Belt', QtGui.QIcon(path + 'Core/Conveyor.png'), 'Defines moving tiles.\n\nMoving tiles will move Mario in one\ndirection or another. Parameters are\nlargely unknown at this time.'],
                     ['Question Block', QtGui.QIcon(path + 'Core/Qblock.png'), 'Creates question blocks.']]

        i = 0
        for item in range(len(self.coreTypes)):
            self.coreWidgets.append(QtWidgets.QRadioButton())
            if i == 0:
                self.coreWidgets[item].setText('Default')
            else:
                self.coreWidgets[item].setIcon(self.coreTypes[item][1])
            self.coreWidgets[item].setIconSize(QtCore.QSize(24, 24))
            self.coreWidgets[item].setToolTip(self.coreTypes[item][2])
            self.coreWidgets[item].clicked.connect(self.swapParams)
            if i < 2:
                rowA.addWidget(self.coreWidgets[item])
            elif i < 4:
                rowB.addWidget(self.coreWidgets[item])
            elif i < 6:
                rowC.addWidget(self.coreWidgets[item])
            elif i < 8:
                rowD.addWidget(self.coreWidgets[item])
            elif i < 10:
                rowE.addWidget(self.coreWidgets[item])
            else:
                rowF.addWidget(self.coreWidgets[item])
            i += 1

        coreLayout.addLayout(rowA)
        coreLayout.addLayout(rowB)
        coreLayout.addLayout(rowC)
        coreLayout.addLayout(rowD)
        coreLayout.addLayout(rowE)
        coreLayout.addLayout(rowF)
        self.coreType.setLayout(coreLayout)


        # Properties Buttons. I hope this works well!
        self.propertyGroup = QtWidgets.QGroupBox()
        self.propertyGroup.setTitle('Properties:')
        propertyLayout = QtWidgets.QVBoxLayout()
        self.propertyWidgets = []
        propertyList = [['Solid', QtGui.QIcon(path + 'Prop/Solid.png'), 'Tiles you can walk on.\n\nThe tiles we be a solid basic square\nthrough which Mario can not pass.'],
                        ['Block', QtGui.QIcon(path + 'Prop/Break.png'), 'This denotes breakable tiles such\nas brick blocks. It is likely that these\nare subject to the same issues as\nexplodable blocks. They emit a coin\nwhen hit.'],
                        ['Falling Block', QtGui.QIcon(path + 'Prop/Fall.png'), 'Sets the block to fall after a set period. The\nblock is sadly replaced with a donut lift model.'],
                        ['Ledge', QtGui.QIcon(path + 'Prop/Ledge.png'), 'A ledge tile with unique properties.\n\nLedges can be shimmied along or\nhung from, but not walked along\nas with normal terrain. Must have the\nledge terrain type set as well.'],
                        ['Meltable', QtGui.QIcon(path + 'Prop/Melt.png'), 'Supposedly allows melting the tile?']]

        for item in range(len(propertyList)):
            self.propertyWidgets.append(QtWidgets.QCheckBox(propertyList[item][0]))
            self.propertyWidgets[item].setIcon(propertyList[item][1])
            self.propertyWidgets[item].setIconSize(QtCore.QSize(24, 24))
            self.propertyWidgets[item].setToolTip(propertyList[item][2])
            propertyLayout.addWidget(self.propertyWidgets[item])


        self.PassThrough = QtWidgets.QRadioButton('Pass-Through')
        self.PassDown = QtWidgets.QRadioButton('Pass-Down')
        self.PassNone = QtWidgets.QRadioButton('No Passing')

        self.PassThrough.setIcon(QtGui.QIcon(path + 'Prop/Pup.png'))
        self.PassDown.setIcon(QtGui.QIcon(path + 'Prop/Pdown.png'))
        self.PassNone.setIcon(QtGui.QIcon(path + 'Prop/Pnone.png'))

        self.PassThrough.setIconSize(QtCore.QSize(24, 24))
        self.PassDown.setIconSize(QtCore.QSize(24, 24))
        self.PassNone.setIconSize(QtCore.QSize(24, 24))

        self.PassThrough.setToolTip('Allows Mario to jump through the bottom\nof the tile and land on the top.')
        self.PassDown.setToolTip("Allows Mario to fall through the tile but\nbe able to jump up through it. Doesn't seem to actually do anything, though?")
        self.PassNone.setToolTip('Default setting')

        propertyLayout.addWidget(self.PassNone)
        propertyLayout.addWidget(self.PassThrough)
        propertyLayout.addWidget(self.PassDown)

        self.propertyGroup.setLayout(propertyLayout)



        # Terrain Type ComboBox
        self.terrainType = QtWidgets.QComboBox()
        self.terrainLabel = QtWidgets.QLabel('Terrain Type')

        self.terrainTypes = [['Default', QtGui.QIcon(path + 'Core/Default.png')],
                        ['Ice', QtGui.QIcon(path + 'Terrain/Ice.png')],
                        ['Snow', QtGui.QIcon(path + 'Terrain/Snow.png')],
                        ['Quicksand', QtGui.QIcon(path + 'Terrain/Quicksand.png')],
                        ['Conveyor Belt Right', QtGui.QIcon(path + 'Core/Conveyor.png')],
                        ['Conveyor Belt Left', QtGui.QIcon(path + 'Core/Conveyor.png')],
                        ['Horiz. Climbing Rope', QtGui.QIcon(path + 'Terrain/Rope.png')],
                        ['Anti Wall Jumps', QtGui.QIcon(path + 'Terrain/Spike.png')],
                        ['Ledge', QtGui.QIcon(path + 'Terrain/Ledge.png')],
                        ['Ladder', QtGui.QIcon(path + 'Terrain/Ladder.png')],
                        ['Staircase', QtGui.QIcon(path + 'Terrain/Stairs.png')],
                        ['Carpet', QtGui.QIcon(path + 'Terrain/Carpet.png')],
                        ['Dusty', QtGui.QIcon(path + 'Terrain/Dust.png')],
                        ['Grass', QtGui.QIcon(path + 'Terrain/Grass.png')],
                        ['Muffled', QtGui.QIcon(path + 'Unknown.png')],
                        ['Beach Sand', QtGui.QIcon(path + 'Terrain/Sand.png')]]

        for item in range(len(self.terrainTypes)):
            self.terrainType.addItem(self.terrainTypes[item][1], self.terrainTypes[item][0])
            self.terrainType.setIconSize(QtCore.QSize(24, 24))
        self.terrainType.setToolTip('Set the various types of terrain.'
                                    '<ul>'
                                    '<li><b>Default:</b><br>'
                                    'Terrain with no particular properties.</li>'
                                    '<li><b>Ice:</b><br>'
                                    'Will be slippery.</li>'
                                    '<li><b>Snow:</b><br>'
                                    'Will emit puffs of snow and snow noises.</li>'
                                    '<li><b>Quicksand:</b><br>'
                                    'Will slowly swallow Mario. Required for creating the quicksand effect.</li>'
                                    '<li><b>Conveyor Belt Right:</b><br>'
                                    'Mario moves rightwards.</li>'
                                    '<li><b>Conveyor Belt Left:</b><br>'
                                    'Mario moves leftwards.</li>'
                                    '<li><b>Horiz. Rope:</b><br>'
                                    'Must be solid to function. Mario will move hand-over-hand along the rope.</li>'
                                    '<li><b>Anti Wall Jumps:</b><br>'
                                    'Mario cannot wall-jump off of the tile.</li>'
                                    '<li><b>Ledge:</b><br>'
                                    'Must have ledge property set as well.</li>'
                                    '<li><b>Ladder:</b><br>'
                                    'Acts as a pole. Mario will face right or left as he climbs.</li>'
                                    '<li><b>Staircase:</b><br>'
                                    'Does not allow Mario to slide.</li>'
                                    '<li><b>Carpet:</b><br>'
                                    'Will muffle footstep noises.</li>'
                                    '<li><b>Dusty:</b><br>'
                                    'Will emit puffs of dust.</li>'
                                    '<li><b>Muffled:</b><br>'
                                    'Mostly muffles footstep noises.</li>'
                                    '<li><b>Grass:</b><br>'
                                    'Will emit grass-like footstep noises.</li>'
                                    '<li><b>Beach Sand:</b><br>'
                                    "Will create sand tufts around Mario's feet.</li>"
                                    '</ul>'
                                   )



        # Parameters ComboBox
        self.parameters = QtWidgets.QComboBox()
        self.parameterLabel = QtWidgets.QLabel('Parameters')
        self.parameters.addItem('None')


        GenericParams = [['None', QtGui.QIcon(path + 'Core/Default.png')],
                         ['Beanstalk Stop', QtGui.QIcon(path + '/Generic/Beanstopper.png')],
                         ['Dash Coin', QtGui.QIcon(path + 'Generic/Outline.png')],
                         ['Battle Coin', QtGui.QIcon(path + 'Generic/Outline.png')],
                         ['Red Block Outline A', QtGui.QIcon(path + 'Generic/RedBlock.png')],
                         ['Red Block Outline B', QtGui.QIcon(path + 'Generic/RedBlock.png')],
                         ['Cave Entrance Right', QtGui.QIcon(path + 'Generic/Cave-Right.png')],
                         ['Cave Entrance Left', QtGui.QIcon(path + 'Generic/Cave-Left.png')],
                         ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                         ['Layer 0 Pit', QtGui.QIcon(path + 'Unknown.png')]]

        RailParams = [['None', QtGui.QIcon(path + 'Core/Default.png')],
                      ['Rail: Upslope', QtGui.QIcon(path + '')],
                      ['Rail: Downslope', QtGui.QIcon(path + '')],
                      ['Rail: 90 degree Corner Fill', QtGui.QIcon(path + '')],
                      ['Rail: 90 degree Corner', QtGui.QIcon(path + '')],
                      ['Rail: Horizontal Rail', QtGui.QIcon(path + '')],
                      ['Rail: Vertical Rail', QtGui.QIcon(path + '')],
                      ['Rail: Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Rail: Gentle Upslope 2', QtGui.QIcon(path + '')],
                      ['Rail: Gentle Upslope 1', QtGui.QIcon(path + '')],
                      ['Rail: Gentle Downslope 2', QtGui.QIcon(path + '')],
                      ['Rail: Gentle Downslope 1', QtGui.QIcon(path + '')],
                      ['Rail: Steep Upslope 2', QtGui.QIcon(path + '')],
                      ['Rail: Steep Upslope 1', QtGui.QIcon(path + '')],
                      ['Rail: Steep Downslope 2', QtGui.QIcon(path + '')],
                      ['Rail: Steep Downslope 1', QtGui.QIcon(path + '')],
                      ['Rail: One Panel Circle', QtGui.QIcon(path + '')],
                      ['Rail: 2x2 Circle Upper Right', QtGui.QIcon(path + '')],
                      ['Rail: 2x2 Circle Upper Left', QtGui.QIcon(path + '')],
                      ['Rail: 2x2 Circle Lower Right', QtGui.QIcon(path + '')],
                      ['Rail: 2x2 Circle Lower Left', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Top Left Corner', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Top Left', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Top Right', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Top Right Corner', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Upper Left Side', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Upper Right Side', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Lower Left Side', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Lower Right Side', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Bottom Left Corner', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Bottom Left', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Bottom Right', QtGui.QIcon(path + '')],
                      ['Rail: 4x4 Circle Bottom Right Corner', QtGui.QIcon(path + '')],
                      ['Rail: Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Rail: End Stop', QtGui.QIcon(path + '')]]

        ClimableGridParams = [['None', QtGui.QIcon(path + 'Core/Default.png')],
                             ['Free Move', QtGui.QIcon(path + 'Climb/Center.png')],
                             ['Upper Left Corner', QtGui.QIcon(path + 'Climb/UpperLeft.png')],
                             ['Top', QtGui.QIcon(path + 'Climb/Top.png')],
                             ['Upper Right Corner', QtGui.QIcon(path + 'Climb/UpperRight.png')],
                             ['Left Side', QtGui.QIcon(path + 'Climb/Left.png')],
                             ['Center', QtGui.QIcon(path + 'Climb/Center.png')],
                             ['Right Side', QtGui.QIcon(path + 'Climb/Right.png')],
                             ['Lower Left Corner', QtGui.QIcon(path + 'Climb/LowerLeft.png')],
                             ['Bottom', QtGui.QIcon(path + 'Climb/Bottom.png')],
                             ['Lower Right Corner', QtGui.QIcon(path + 'Climb/LowerRight.png')]]


        CoinParams = [['Generic Coin', QtGui.QIcon(path + 'QBlock/Coin.png')],
                     ['Coin', QtGui.QIcon(path + 'Unknown.png')],
                     ['Nothing', QtGui.QIcon(path + 'Unknown.png')],
                     ['Coin', QtGui.QIcon(path + 'Unknown.png')],
                     ['Pow Block Coin', QtGui.QIcon(path + 'Coin/POW.png')]]

        ExplodableBlockParams = [['None', QtGui.QIcon(path + 'Core/Default.png')],
                                ['Stone Block', QtGui.QIcon(path + 'Explode/Stone.png')],
                                ['Wooden Block', QtGui.QIcon(path + 'Explode/Wooden.png')],
                                ['Red Block', QtGui.QIcon(path + 'Explode/Red.png')],
                                ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                                ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                                ['Unknown', QtGui.QIcon(path + 'Unknown.png')]]

        PipeParams = [['Vert. Top Entrance Left', QtGui.QIcon(path + 'Pipes/')],
                      ['Vert. Top Entrance Right', QtGui.QIcon(path + '')],
                      ['Vert. Bottom Entrance Left', QtGui.QIcon(path + '')],
                      ['Vert. Bottom Entrance Right', QtGui.QIcon(path + '')],
                      ['Vert. Center Left', QtGui.QIcon(path + '')],
                      ['Vert. Center Right', QtGui.QIcon(path + '')],
                      ['Vert. On Top Junction Left', QtGui.QIcon(path + '')],
                      ['Vert. On Top Junction Right', QtGui.QIcon(path + '')],
                      ['Horiz. Left Entrance Top', QtGui.QIcon(path + '')],
                      ['Horiz. Left Entrance Bottom', QtGui.QIcon(path + '')],
                      ['Horiz. Right Entrance Top', QtGui.QIcon(path + '')],
                      ['Horiz. Right Entrance Bottom', QtGui.QIcon(path + '')],
                      ['Horiz. Center Left', QtGui.QIcon(path + '')],
                      ['Horiz. Center Right', QtGui.QIcon(path + '')],
                      ['Horiz. On Top Junction Top', QtGui.QIcon(path + '')],
                      ['Horiz. On Top Junction Bottom', QtGui.QIcon(path + '')],
                      ['Vert. Mini Pipe Top', QtGui.QIcon(path + '')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Vert. Mini Pipe Bottom', QtGui.QIcon(path + '')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Vert. On Top Mini-Junction', QtGui.QIcon(path + '')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Horiz. Mini Pipe Left', QtGui.QIcon(path + '')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Horiz. Mini Pipe Right', QtGui.QIcon(path + '')],
                      ['Unknown', QtGui.QIcon(path + 'Unknown.png')],
                      ['Vert. Mini Pipe Center', QtGui.QIcon(path + '')],
                      ['Horiz. Mini Pipe Center', QtGui.QIcon(path + '')],
                      ['Horiz. On Top Mini-Junction', QtGui.QIcon(path + '')],
                      ['Block Covered Corner', QtGui.QIcon(path + '')]]

        PartialBlockParams = [['None', QtGui.QIcon(path + 'Core/Default.png')],
                              ['Upper Left', QtGui.QIcon(path + 'Partial/UpLeft.png')],
                              ['Upper Right', QtGui.QIcon(path + 'Partial/UpRight.png')],
                              ['Top Half', QtGui.QIcon(path + 'Partial/TopHalf.png')],
                              ['Lower Left', QtGui.QIcon(path + 'Partial/LowLeft.png')],
                              ['Left Half', QtGui.QIcon(path + 'Partial/LeftHalf.png')],
                              ['Diagonal Downwards', QtGui.QIcon(path + 'Partial/DiagDn.png')],
                              ['Upper Left 3/4', QtGui.QIcon(path + 'Partial/UpLeft3-4.png')],
                              ['Lower Right', QtGui.QIcon(path + 'Partial/LowRight.png')],
                              ['Diagonal Downwards', QtGui.QIcon(path + 'Partial/DiagDn.png')],
                              ['Right Half', QtGui.QIcon(path + 'Partial/RightHalf.png')],
                              ['Upper Right 3/4', QtGui.QIcon(path + 'Partial/UpRig3-4.png')],
                              ['Lower Half', QtGui.QIcon(path + 'Partial/LowHalf.png')],
                              ['Lower Left 3/4', QtGui.QIcon(path + 'Partial/LowLeft3-4.png')],
                              ['Lower Right 3/4', QtGui.QIcon(path + 'Partial/LowRight3-4.png')],
                              ['Full Brick', QtGui.QIcon(path + 'Partial/Full.png')]]

        SlopeParams = [['Steep Upslope', QtGui.QIcon(path + 'Slope/steepslopeleft.png')],
                       ['Steep Downslope', QtGui.QIcon(path + 'Slope/steepsloperight.png')],
                       ['Upslope 1', QtGui.QIcon(path + 'Slope/slopeleft.png')],
                       ['Upslope 2', QtGui.QIcon(path + 'Slope/slope3left.png')],
                       ['Downslope 1', QtGui.QIcon(path + 'Slope/slope3right.png')],
                       ['Downslope 2', QtGui.QIcon(path + 'Slope/sloperight.png')],
                       ['Steep Upslope 1', QtGui.QIcon(path + 'Slope/vsteepup1.png')],
                       ['Steep Upslope 2', QtGui.QIcon(path + 'Slope/vsteepup2.png')],
                       ['Steep Downslope 1', QtGui.QIcon(path + 'Slope/vsteepdown1.png')],
                       ['Steep Downslope 2', QtGui.QIcon(path + 'Slope/vsteepdown2.png')],
                       ['Slope Edge (solid)', QtGui.QIcon(path + 'Slope/edge.png')],
                       ['Gentle Upslope 1', QtGui.QIcon(path + 'Slope/gentleupslope1.png')],
                       ['Gentle Upslope 2', QtGui.QIcon(path + 'Slope/gentleupslope2.png')],
                       ['Gentle Upslope 3', QtGui.QIcon(path + 'Slope/gentleupslope3.png')],
                       ['Gentle Upslope 4', QtGui.QIcon(path + 'Slope/gentleupslope4.png')],
                       ['Gentle Downslope 1', QtGui.QIcon(path + 'Slope/gentledownslope1.png')],
                       ['Gentle Downslope 2', QtGui.QIcon(path + 'Slope/gentledownslope2.png')],
                       ['Gentle Downslope 3', QtGui.QIcon(path + 'Slope/gentledownslope3.png')],
                       ['Gentle Downslope 4', QtGui.QIcon(path + 'Slope/gentledownslope4.png')]]

        ReverseSlopeParams = [['Steep Downslope', QtGui.QIcon(path + 'Slope/Rsteepslopeleft.png')],
                              ['Steep Upslope', QtGui.QIcon(path + 'Slope/Rsteepsloperight.png')],
                              ['Downslope 1', QtGui.QIcon(path + 'Slope/Rslopeleft.png')],
                              ['Downslope 2', QtGui.QIcon(path + 'Slope/Rslope3left.png')],
                              ['Upslope 1', QtGui.QIcon(path + 'Slope/Rslope3right.png')],
                              ['Upslope 2', QtGui.QIcon(path + 'Slope/Rsloperight.png')],
                              ['Steep Downslope 1', QtGui.QIcon(path + 'Slope/Rvsteepdown1.png')],
                              ['Steep Downslope 2', QtGui.QIcon(path + 'Slope/Rvsteepdown2.png')],
                              ['Steep Upslope 1', QtGui.QIcon(path + 'Slope/Rvsteepup1.png')],
                              ['Steep Upslope 2', QtGui.QIcon(path + 'Slope/Rvsteepup2.png')],
                              ['Slope Edge (solid)', QtGui.QIcon(path + 'Slope/edge.png')],
                              ['Gentle Downslope 1', QtGui.QIcon(path + 'Slope/Rgentledownslope1.png')],
                              ['Gentle Downslope 2', QtGui.QIcon(path + 'Slope/Rgentledownslope2.png')],
                              ['Gentle Downslope 3', QtGui.QIcon(path + 'Slope/Rgentledownslope3.png')],
                              ['Gentle Downslope 4', QtGui.QIcon(path + 'Slope/Rgentledownslope4.png')],
                              ['Gentle Upslope 1', QtGui.QIcon(path + 'Slope/Rgentleupslope1.png')],
                              ['Gentle Upslope 2', QtGui.QIcon(path + 'Slope/Rgentleupslope2.png')],
                              ['Gentle Upslope 3', QtGui.QIcon(path + 'Slope/Rgentleupslope3.png')],
                              ['Gentle Upslope 4', QtGui.QIcon(path + 'Slope/Rgentleupslope4.png')]]

        SpikeParams = [['Double Left Spikes', QtGui.QIcon(path + 'Spike/Left.png')],
                       ['Double Right Spikes', QtGui.QIcon(path + 'Spike/Right.png')],
                       ['Double Upwards Spikes', QtGui.QIcon(path + 'Spike/Up.png')],
                       ['Double Downwards Spikes', QtGui.QIcon(path + 'Spike/Down.png')],
                       ['Long Spike Down 1', QtGui.QIcon(path + 'Spike/LongDown1.png')],
                       ['Long Spike Down 2', QtGui.QIcon(path + 'Spike/LongDown2.png')],
                       ['Single Downwards Spike', QtGui.QIcon(path + 'Spike/SingDown.png')],
                       ['Spike Block', QtGui.QIcon(path + 'Unknown.png')]]

        ConveyorBeltParams = [['Slow', QtGui.QIcon(path + 'Unknown.png')],
                              ['Fast', QtGui.QIcon(path + 'Unknown.png')]]

        QBlockParams = [['Fire Flower', QtGui.QIcon(path + 'Qblock/Fire.png')],
                       ['Star', QtGui.QIcon(path + 'Qblock/Star.png')],
                       ['Coin', QtGui.QIcon(path + 'Qblock/Coin.png')],
                       ['Vine', QtGui.QIcon(path + 'Qblock/Vine.png')],
                       ['1-Up', QtGui.QIcon(path + 'Qblock/1up.png')],
                       ['Mini Mushroom', QtGui.QIcon(path + 'Qblock/Mini.png')],
                       ['Propeller Suit', QtGui.QIcon(path + 'Qblock/Prop.png')],
                       ['Penguin Suit', QtGui.QIcon(path + 'Qblock/Peng.png')],
                       ['Ice Flower', QtGui.QIcon(path + 'Qblock/IceF.png')]]


        self.ParameterList = [GenericParams,
                              SlopeParams,
                              ReverseSlopeParams,
                              PartialBlockParams,
                              CoinParams,
                              ExplodableBlockParams,
                              ClimableGridParams,
                              SpikeParams,
                              PipeParams,
                              RailParams,
                              ConveyorBeltParams,
                              QBlockParams]


        layout = QtWidgets.QGridLayout()
        layout.addWidget(self.coreType, 0, 1)
        layout.addWidget(self.propertyGroup, 0, 0, 3, 1)
        layout.addWidget(self.terrainType, 2, 1)
        layout.addWidget(self.parameters, 1, 1)
        self.setLayout(layout)


    def swapParams(self):
        for item in range(len(self.ParameterList)):
            if self.coreWidgets[item].isChecked():
                self.parameters.clear()
                for option in self.ParameterList[item]:
                    self.parameters.addItem(option[1], option[0])



#############################################################################################
######################### InfoBox Custom Widget to display info to ##########################


class InfoBox(QtWidgets.QWidget):
    def __init__(self, window):
        super(InfoBox, self).__init__(window)

        # InfoBox
        superLayout = QtWidgets.QGridLayout()
        infoLayout = QtWidgets.QFormLayout()

        self.imageBox = QtWidgets.QGroupBox()
        imageLayout = QtWidgets.QHBoxLayout()

        pix = QtGui.QPixmap(24, 24)
        pix.fill(Qt.transparent)

        self.coreImage = QtWidgets.QLabel()
        self.coreImage.setPixmap(pix)
        self.terrainImage = QtWidgets.QLabel()
        self.terrainImage.setPixmap(pix)
        self.parameterImage = QtWidgets.QLabel()
        self.parameterImage.setPixmap(pix)


        def updateAllTiles():
            for i in range(256):
                window.tileDisplay.update(window.tileDisplay.model().index(i, 0))
        self.collisionOverlay = QtWidgets.QCheckBox('Overlay Collision')
        self.collisionOverlay.clicked.connect(updateAllTiles)

        self.toggleAlpha = QtWidgets.QCheckBox('Toggle Alpha')
        self.toggleAlpha.clicked.connect(window.toggleAlpha)

        class QScreenshot(QtWidgets.QSplashScreen):
            def __init__(self, screenshot):
                super().__init__(screenshot)

        def showScreenshot():
            self.image = window.tileDisplay.grab()
            self.image = self.image.scaled(self.image.width()*2, self.image.height()*2)
            self.screenshotWindow = QScreenshot(self.image)
            self.screenshotWindow.show()

        self.screenshotButton = QtWidgets.QPushButton('View enlarged screenshot')
        self.screenshotButton.released.connect(showScreenshot)

        self.coreInfo = QtWidgets.QLabel()
        self.terrainInfo = QtWidgets.QLabel()
        self.paramInfo = QtWidgets.QLabel()
        self.propertyBox = QtWidgets.QGroupBox()
        self.propertyInfo = QtWidgets.QLabel('Properties:\nNone             \n\n\n\n\n')
        self.propertyInfo.setWordWrap(True)
        self.propertyInfo.setSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Minimum)

        Font = self.font()
        Font.setPointSize(9)

        self.coreInfo.setFont(Font)
        self.propertyInfo.setFont(Font)
        self.terrainInfo.setFont(Font)
        self.paramInfo.setFont(Font)

        self.hexdata = QtWidgets.QLabel('Hex Data: 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00')
        self.hexdata.setFont(Font)
        self.numInfo = QtWidgets.QLabel('Slot: 0 Row: 0 Column: 0')
        self.numInfo.setFont(Font)

        coreLayout = QtWidgets.QVBoxLayout()
        terrLayout = QtWidgets.QVBoxLayout()
        paramLayout = QtWidgets.QVBoxLayout()

        coreLayout.setGeometry(QtCore.QRect(0,0,40,40))
        terrLayout.setGeometry(QtCore.QRect(0,0,40,40))
        paramLayout.setGeometry(QtCore.QRect(0,0,40,40))


        label = QtWidgets.QLabel('Core')
        label.setFont(Font)
        coreLayout.addWidget(label, 0, Qt.AlignCenter)

        label = QtWidgets.QLabel('Terrain')
        label.setFont(Font)
        terrLayout.addWidget(label, 0, Qt.AlignCenter)

        label = QtWidgets.QLabel('Parameters')
        label.setFont(Font)
        paramLayout.addWidget(label, 0, Qt.AlignCenter)

        coreLayout.addWidget(self.coreImage, 0, Qt.AlignCenter)
        terrLayout.addWidget(self.terrainImage, 0, Qt.AlignCenter)
        paramLayout.addWidget(self.parameterImage, 0, Qt.AlignCenter)

        coreLayout.addWidget(self.coreInfo, 0, Qt.AlignCenter)
        terrLayout.addWidget(self.terrainInfo, 0, Qt.AlignCenter)
        paramLayout.addWidget(self.paramInfo, 0, Qt.AlignCenter)

        imageLayout.setContentsMargins(0,4,4,4)
        imageLayout.addLayout(coreLayout)
        imageLayout.addStretch()
        imageLayout.addLayout(terrLayout)
        imageLayout.addStretch()
        imageLayout.addLayout(paramLayout)

        self.imageBox.setLayout(imageLayout)

        infoLayout.setContentsMargins(0,4,4,4)
        infoLayout.addRow(self.propertyInfo)
        self.propertyBox.setLayout(infoLayout)

        self.setMinimumWidth(800)
        #self.setMaximumWidth(1000)

        superLayout.addWidget(self.imageBox, 0, 2, 1, 2)
        superLayout.addWidget(self.propertyBox, 0, 0, 1, 2)
        superLayout.addWidget(self.hexdata, 1, 0, 1, 4, Qt.AlignCenter)
        superLayout.addWidget(self.numInfo, 2, 0, 1, 4, Qt.AlignCenter)
        superLayout.addWidget(self.collisionOverlay, 3, 0, 1, 1)
        superLayout.addWidget(self.toggleAlpha, 3, 1, 1, 2, Qt.AlignCenter)
        superLayout.addWidget(self.screenshotButton, 3, 3, 1, 1, Qt.AlignRight)
        self.setLayout(superLayout)


#############################################################################################
##################### Framesheet List Widget and Model Setup with Painter #######################


class framesheetList(QtWidgets.QListView):

    def __init__(self, parent=None):
        super(framesheetList, self).__init__(parent)

        self.setViewMode(QtWidgets.QListView.IconMode)
        self.setHeight()
        self.setMovement(QtWidgets.QListView.Static)
        self.setBackgroundRole(QtGui.QPalette.BrightText)
        self.setWrapping(False)
        self.setMinimumHeight(512)
        self.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)

    def setHeight(self):
        height = getFramesheetGridSize()
        self.setIconSize(QtCore.QSize(32, height))
        self.setGridSize(QtCore.QSize(200,height+50))


def getFramesheetGridSize():
    global Tileset
    max = 0
    for key in list(Tileset.animdata.keys()):
        t = len(Tileset.animdata[key])//64
        if t > max:
            max = t
    return max



def SetupFramesheetModel(self, animdata):
    global Tileset
    self.framesheetList.setHeight()
    self.framesheetmodel.clear()
    self.frames = {}
    frames = []

    count = 0
    for key in list(animdata.keys()):
        height = len(animdata[key])//64

        image = QtGui.QImage(32, height, QtGui.QImage.Format_ARGB32)
        frame = QtGui.QImage(32, 32, QtGui.QImage.Format_ARGB32)

        bytes = animdata[key]
        bits = ''.join(format(byte, '08b') for byte in bytes)

        Xoffset = 0
        Yoffset = 0
        XBlock = 0
        YBlock = 0

        frames = []

        for i in range(0, len(bits), 16):
            color = RGB4A3LUT[int(bits[i:i+16], 2)]

            image.setPixel(Xoffset+XBlock, Yoffset+YBlock, color)
            frame.setPixel(Xoffset+XBlock, (Yoffset+YBlock)%32, color)

            XBlock += 1
            if XBlock >= 4:
                XBlock = 0
                YBlock += 1
                if YBlock >= 4:
                    YBlock = 0
                    Xoffset += 4
                    if Xoffset >= 32:
                        Xoffset = 0
                        Yoffset += 4
                        if (Yoffset+YBlock)%32 == 0 and not (Yoffset+YBlock) == 0:
                            frames.append(QtGui.QPixmap.fromImage(frame))
                            frame = QtGui.QImage(32, 32, QtGui.QImage.Format_ARGB32)

        self.frames[key] = frames

        tex = QtGui.QPixmap.fromImage(image)

        #fn = QtWidgets.QFileDialog.getSaveFileName(self, 'Choose a new filename', '', '.png (*.png)')[0]
        #tex.save(fn)


        item = QtGui.QStandardItem(QtGui.QIcon(tex), '{0}'.format(key[7:-4]))
        item.setEditable(False)
        self.framesheetmodel.appendRow(item)

        count += 1


def RGB4A3FramesheetEncode(tex):
    shorts = []
    colorCache = {}

    for Yoffset in range(0, tex.height(), 4):
        for Xoffset in range(0,32,4):
            for ypixel in range(Yoffset, Yoffset + 4):
                for xpixel in range(Xoffset, Xoffset + 4):

                    pixel = tex.pixel(xpixel, ypixel)

                    a = pixel >> 24
                    r = (pixel >> 16) & 0xFF
                    g = (pixel >> 8) & 0xFF
                    b = pixel & 0xFF

                    if pixel in colorCache:
                        rgba = colorCache[pixel]

                    else:
                        if a < 238: # RGB4A3
                            alpha = ((a + 18) << 1) // 73
                            red = (r + 8) // 17
                            green = (g + 8) // 17
                            blue = (b + 8) // 17

                            # 0aaarrrrggggbbbb
                            rgba = blue | (green << 4) | (red << 8) | (alpha << 12)

                        else: # RGB555
                            red = ((r + 4) << 2) // 33
                            green = ((g + 4) << 2) // 33
                            blue = ((b + 4) << 2) // 33

                            # 1rrrrrgggggbbbbb
                            rgba = blue | (green << 5) | (red << 10) | (0x8000)

                        colorCache[pixel] = rgba

                    shorts.append(rgba)
    return struct.pack('>{0}H'.format(len(shorts)), *shorts)


class framesheetOverlord(QtWidgets.QWidget):

    def __init__(self):
        super(framesheetOverlord, self).__init__()

        self.addFramesheet = QtWidgets.QPushButton('Add')
        self.removeFramesheet = QtWidgets.QPushButton('Remove')
        self.replaceFramesheet = QtWidgets.QPushButton('Replace')
        self.renameFramesheet = QtWidgets.QPushButton('Rename')


        # Connections
        self.addFramesheet.released.connect(self.addFs)
        self.removeFramesheet.released.connect(self.removeFs)
        self.replaceFramesheet.released.connect(self.replaceFs)
        self.renameFramesheet.released.connect(self.renameFs)


        # Layout
        layout = QtWidgets.QGridLayout()

        layout.addWidget(self.addFramesheet, 0, 6, 1, 1)
        layout.addWidget(self.removeFramesheet, 0, 7, 1, 1)
        layout.addWidget(self.replaceFramesheet, 0, 8, 1, 1)
        layout.addWidget(self.renameFramesheet, 0, 9, 1, 1)

        self.setLayout(layout)


    def openFs(self, forAdding = True):
        '''Opens an framesheet from png.'''

        path = QtWidgets.QFileDialog.getOpenFileName(self, "Open framesheet", '', "Image Files (*.png)")[0]
        if not path: return (None, None)

        framesheet = QtGui.QImage()
        if not framesheet.load(path):
            QtWidgets.QMessageBox.warning(self, "Open framesheet",
                    "The framesheet file could not be loaded.",
                    QtWidgets.QMessageBox.Cancel)
            return (None, None)

        if forAdding and framesheet.width() != 32 or framesheet.height() % 32 != 0:
            QtWidgets.QMessageBox.warning(self, "Open framesheet",
                    "The framesheet has incorrect dimensions. "
                    "Needed sizes: 32 pixel width and a multiple of 32 height.",
                    QtWidgets.QMessageBox.Cancel)
            return (None, None)

        return (framesheet, path)


    def addFs(self):
        global Tileset

        framesheet, path = self.openFs()
        if framesheet is None: return
        #print(framesheet)
        base = os.path.basename(path)
        name = os.path.splitext(base)[0]


        while self.nameIsTaken(name):
            newName, ok = QtWidgets.QInputDialog.getText(self, 'Rename framesheet', 'The framesheet name is already taken!\nEnter a new name:')

            if not ok:
                return

            if not newName or " " in newName:
                QtWidgets.QMessageBox.warning(self, "Rename framesheet",
                        "The file name cannot be empty or contain any spaces.",
                        QtWidgets.QMessageBox.Cancel)
                continue

            name = newName

        image = framesheet.convertToFormat(QtGui.QImage.Format_ARGB32)
        data = RGB4A3FramesheetEncode(image)
        Tileset.animdata["BG_tex/{0}.bin".format(name)] = data

        frames = []
        for y in range(0, len(data)//2048):
            frame = framesheet.copy(0, 32*y, 32, 32)
            frames.append(QtGui.QPixmap.fromImage(frame))
        window.frames["BG_tex/{0}.bin".format(name)] = frames

        window.framesheetmodel.appendRow(QtGui.QStandardItem(QtGui.QIcon(QtGui.QPixmap.fromImage(framesheet)), '{0}'.format(name)))
        index = window.framesheetList.currentIndex()
        window.framesheetList.setCurrentIndex(index)
        #self.setObject(index)

        window.framesheetList.setHeight()
        window.framesheetList.update()
        self.update()


    def removeFs(self):
        if not Tileset.animdata:
            return

        index = window.framesheetList.currentIndex()

        if index.row() == -1:
            return

        name = window.framesheetmodel.itemFromIndex(index).text()
        Tileset.animdata.pop("BG_tex/{0}.bin".format(name), None)
        window.frames.pop("BG_tex/{0}.bin".format(name), None)

        window.framesheetmodel.removeRow(index.row())

        index = window.framesheetList.currentIndex()
        if not index.row() == -1:
            window.framesheetList.setCurrentIndex(index)
            #self.setObject(index)

        window.framesheetList.update()
        self.update()


    def replaceFs(self):
        index = window.framesheetList.currentIndex()

        if index.row() == -1:
            return

        name = window.framesheetmodel.itemFromIndex(index).text()
        iconSize = window.framesheetmodel.itemFromIndex(index).icon().availableSizes()[0]
        framesheet, temp = self.openFs(False)

        if framesheet is None: return

        if not framesheet.width() == iconSize.width() or not framesheet.height() == iconSize.height():
            QtWidgets.QMessageBox.warning(self, "Open framesheet",
                    "The framesheet has incorrect dimensions. "
                    "Needed sizes: {0} pixel width and {1} pixel height.".format(iconSize.width(), iconSize.height()),
                    QtWidgets.QMessageBox.Cancel)
            return

        image = framesheet.convertToFormat(QtGui.QImage.Format_ARGB32);
        data = RGB4A3FramesheetEncode(image)
        Tileset.animdata["BG_tex/{0}.bin".format(name)] = data

        frames = []
        for y in range(0, len(data)//2048):
            frame = framesheet.copy(0, 32*y, 32, 32)
            frames.append(frame)
        window.frames["BG_tex/{0}.bin".format(name)] = frames

        window.framesheetmodel.itemFromIndex(index).setIcon(QtGui.QIcon(QtGui.QPixmap.fromImage(framesheet)))

        window.framesheetList.update()
        self.update()


    def renameFs(self):
        index = window.framesheetList.currentIndex()

        if index.row() == -1:
            return

        oldName = window.framesheetmodel.itemFromIndex(index).text()

        name, ok = QtWidgets.QInputDialog.getText(self, 'Rename framesheet', 'Enter the new name:')
        if not ok or name == oldName:
            return

        if not name or " " in name:
            QtWidgets.QMessageBox.warning(self, "Rename framesheet",
                    "The file name cannot be empty or contain any spaces.",
                    QtWidgets.QMessageBox.Cancel)
            return

        if self.nameIsTaken(name):
            QtWidgets.QMessageBox.warning(self, "Rename framesheet",
                    "The file name is already taken by another framesheet.",
                    QtWidgets.QMessageBox.Cancel)
            return

        window.framesheetmodel.itemFromIndex(index).setText(name)

        Tileset.animdata["BG_tex/{0}.bin".format(name)] = Tileset.animdata.pop("BG_tex/{0}.bin".format(oldName))

        window.frames["BG_tex/{0}.bin".format(name)] = window.frames.pop("BG_tex/{0}.bin".format(oldName))

        window.framesheetList.update()
        self.update()


    def nameIsTaken(self, name):
        return "BG_tex/{0}.bin".format(name) in list(Tileset.animdata.keys())


#############################################################################################
##################### Object List Widget and Model Setup with Painter #######################


class objectList(QtWidgets.QListView):

    def __init__(self, parent=None):
        super(objectList, self).__init__(parent)

        self.noneIdx = self.currentIndex()

        self.setIconSize(QtCore.QSize(10000, 10000))
        self.setUniformItemSizes(False)
        self.setBackgroundRole(QtGui.QPalette.BrightText)
        self.setWrapping(False)
        self.setMinimumWidth(300)
        self.setMaximumWidth(300)

    def setHeight(self):
        height = getObjectMaxSize()
        self.setIconSize(QtCore.QSize(32, height))
        #self.setGridSize(QtCore.QSize(200,height+50))

    def clearCurrentIndex(self):
        self.setCurrentIndex(self.noneIdx)


def getObjectMaxSize():
    global Tileset
    max = 0
    for key in list(Tileset.animdata.keys()):
        t = len(Tileset.animdata[key])//64
        if t > max:
            max = t
    return max


def SetupObjectModel(self, objects, tiles):
    global Tileset
    self.clear()

    count = 0
    for object in objects:
        tex = QtGui.QPixmap(object.width * 24, object.height * 24)
        tex.fill(Qt.transparent)
        painter = QtGui.QPainter(tex)

        Xoffset = 0
        Yoffset = 0

        for i in range(len(object.tiles)):
            for tile in object.tiles[i]:
                if Tileset.slot == (tile[2] & 3):
                    painter.drawImage(Xoffset, Yoffset, tiles[tile[1]].image)
                Xoffset += 24
            Xoffset = 0
            Yoffset += 24

        painter.end()

        item = QtGui.QStandardItem(QtGui.QIcon(tex), 'Object {0}'.format(count))
        item.setEditable(False)
        self.appendRow(item)

        count += 1


#############################################################################################
######################## List Widget with custom painter/MouseEvent #########################


class displayWidget(QtWidgets.QListView):

    mouseMoved = QtCoreSignal(int, int)
    rightClicked = QtCoreSignal(int, int)

    def __init__(self, parent=None):
        super(displayWidget, self).__init__(parent)

        self.setMinimumWidth(403)
        self.setMaximumWidth(403)
        self.setMinimumHeight(403)
        self.setMaximumHeight(403)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setDragEnabled(True)
        self.setViewMode(QtWidgets.QListView.IconMode)
        self.setIconSize(QtCore.QSize(24,24))
        self.setGridSize(QtCore.QSize(25,25))
        self.setMovement(QtWidgets.QListView.Static)
        self.setAcceptDrops(False)
        self.setDropIndicatorShown(True)
        self.setResizeMode(QtWidgets.QListView.Adjust)
        self.setUniformItemSizes(True)
        self.setBackgroundRole(QtGui.QPalette.BrightText)
        self.setMouseTracking(True)
        self.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)

        self.setItemDelegate(self.TileItemDelegate())


    def mouseMoveEvent(self, event):
        QtWidgets.QWidget.mouseMoveEvent(self, event)
        self.mouseMoved.emit(event.x(), event.y())


    def contextMenuEvent(self, event):
        self.rightClicked.emit(event.x(), event.y())
        super(displayWidget, self).contextMenuEvent(event)


    class TileItemDelegate(QtWidgets.QAbstractItemDelegate):
        """Handles tiles and their rendering"""

        def __init__(self):
            """Initialises the delegate"""
            QtWidgets.QAbstractItemDelegate.__init__(self)

        def paint(self, painter, option, index):
            """Paints an object"""

            global Tileset
            p = index.model().data(index, Qt.DecorationRole)
            painter.drawPixmap(option.rect.x(), option.rect.y(), p.pixmap(24,24))

            x = option.rect.x()
            y = option.rect.y()


            # Collision Overlays
            info = window.infoDisplay
            if index.row() >= len(Tileset.tiles): return
            curTile = Tileset.tiles[index.row()]

            if info.collisionOverlay.isChecked():
                path = os.path.dirname(os.path.abspath(sys.argv[0])) + '/Icons/'

                # Sets the colour based on terrain type
                if curTile.byte2 & 16:      # Red
                    colour = QtGui.QColor(255, 0, 0, 120)
                elif curTile.byte5 == 1:    # Ice
                    colour = QtGui.QColor(0, 0, 255, 120)
                elif curTile.byte5 == 2:    # Snow
                    colour = QtGui.QColor(0, 0, 255, 120)
                elif curTile.byte5 == 3:    # Quicksand
                    colour = QtGui.QColor(128,64,0, 120)
                elif curTile.byte5 == 4:    # Conveyor
                    colour = QtGui.QColor(128,128,128, 120)
                elif curTile.byte5 == 5:    # Conveyor
                    colour = QtGui.QColor(128,128,128, 120)
                elif curTile.byte5 == 6:    # Rope
                    colour = QtGui.QColor(128,0,255, 120)
                elif curTile.byte5 == 7:    # Half Spike
                    colour = QtGui.QColor(128,0,255, 120)
                elif curTile.byte5 == 8:    # Ledge
                    colour = QtGui.QColor(128,0,255, 120)
                elif curTile.byte5 == 9:    # Ladder
                    colour = QtGui.QColor(128,0,255, 120)
                elif curTile.byte5 == 10:    # Staircase
                    colour = QtGui.QColor(255, 0, 0, 120)
                elif curTile.byte5 == 11:    # Carpet
                    colour = QtGui.QColor(255, 0, 0, 120)
                elif curTile.byte5 == 12:    # Dust
                    colour = QtGui.QColor(128,64,0, 120)
                elif curTile.byte5 == 13:    # Grass
                    colour = QtGui.QColor(0, 255, 0, 120)
                elif curTile.byte5 == 14:    # Unknown
                    colour = QtGui.QColor(255, 0, 0, 120)
                elif curTile.byte5 == 15:    # Beach Sand
                    colour = QtGui.QColor(128, 64, 0, 120)
                else:                       # Brown?
                    colour = QtGui.QColor(64, 30, 0, 120)


                # Sets Brush style for fills
                if curTile.byte2 & 4:        # Climbing Grid
                    style = Qt.DiagCrossPattern
                elif curTile.byte3 & 16:     # Breakable
                    style = Qt.VerPattern
                else:
                    style = Qt.SolidPattern


                brush = QtGui.QBrush(colour, style)
                painter.setBrush(brush)
                painter.setRenderHint(QtGui.QPainter.Antialiasing)


                # Paints shape based on other junk
                if curTile.byte3 & 32: # Slope
                    if curTile.byte7 == 0:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 1:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 2:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 12)]))
                    elif curTile.byte7 == 3:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 24)]))
                    elif curTile.byte7 == 4:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24)]))
                    elif curTile.byte7 == 5:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 10:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 11:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 18),
                                                            QtCore.QPoint(x + 24, y + 24)]))
                    elif curTile.byte7 == 12:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 18),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 13:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 6),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 14:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x, y + 6),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 15:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 6),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 16:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 6),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 17:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y + 18),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 18:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 18),
                                                            QtCore.QPoint(x, y + 24)]))

                elif curTile.byte3 & 64: # Reverse Slope
                    if curTile.byte7 == 0:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 1:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 2:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y + 12)]))
                    elif curTile.byte7 == 3:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 4:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12)]))
                    elif curTile.byte7 == 5:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 10:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y)]))
                    elif curTile.byte7 == 11:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 6)]))
                    elif curTile.byte7 == 12:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 6)]))
                    elif curTile.byte7 == 13:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 18),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 14:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 18)]))
                    elif curTile.byte7 == 15:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 18),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 16:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 18)]))
                    elif curTile.byte7 == 17:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 6),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 18:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x, y + 6)]))

                elif curTile.byte2 & 8: # Partial
                    if curTile.byte7 == 1:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 2:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 12, y + 12)]))
                    elif curTile.byte7 == 3:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 4:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 5:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 6:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 7:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 8:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24)]))
                    elif curTile.byte7 == 9:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x + 12, y)]))
                    elif curTile.byte7 == 10:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24)]))
                    elif curTile.byte7 == 11:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x, y + 12)]))
                    elif curTile.byte7 == 12:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 13:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 14:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 12, y + 12),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y + 24)]))
                    elif curTile.byte7 == 15:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 24)]))

                elif curTile.byte2 & 0x40: # Solid-on-bottom
                    painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                        QtCore.QPoint(x + 24, y + 24),
                                                        QtCore.QPoint(x + 24, y + 18),
                                                        QtCore.QPoint(x, y + 18)]))

                    painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 15, y),
                                                        QtCore.QPoint(x + 15, y + 12),
                                                        QtCore.QPoint(x + 18, y + 12),
                                                        QtCore.QPoint(x + 12, y + 17),
                                                        QtCore.QPoint(x + 6, y + 12),
                                                        QtCore.QPoint(x + 9, y + 12),
                                                        QtCore.QPoint(x + 9, y)]))

                elif curTile.byte2 & 0x80: # Solid-on-top
                    painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                        QtCore.QPoint(x + 24, y),
                                                        QtCore.QPoint(x + 24, y + 6),
                                                        QtCore.QPoint(x, y + 6)]))

                    painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 15, y + 24),
                                                        QtCore.QPoint(x + 15, y + 12),
                                                        QtCore.QPoint(x + 18, y + 12),
                                                        QtCore.QPoint(x + 12, y + 7),
                                                        QtCore.QPoint(x + 6, y + 12),
                                                        QtCore.QPoint(x + 9, y + 12),
                                                        QtCore.QPoint(x + 9, y + 24)]))

                elif curTile.byte2 & 16: # Spikes
                    if curTile.byte7 == 0:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x, y + 6)]))
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 24, y + 12),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x, y + 18)]))
                    if curTile.byte7 == 1:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x + 24, y + 6)]))
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 12),
                                                            QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 24, y + 18)]))
                    if curTile.byte7 == 2:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y + 24),
                                                            QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x + 6, y)]))
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 12, y + 24),
                                                            QtCore.QPoint(x + 24, y + 24),
                                                            QtCore.QPoint(x + 18, y)]))
                    if curTile.byte7 == 3:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 6, y + 24)]))
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 12, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 18, y + 24)]))
                    if curTile.byte7 == 4:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 18, y + 24),
                                                            QtCore.QPoint(x + 6, y + 24)]))
                    if curTile.byte7 == 5:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x + 6, y),
                                                            QtCore.QPoint(x + 18, y),
                                                            QtCore.QPoint(x + 12, y + 24)]))
                    if curTile.byte7 == 6:
                        painter.drawPolygon(QtGui.QPolygon([QtCore.QPoint(x, y),
                                                            QtCore.QPoint(x + 24, y),
                                                            QtCore.QPoint(x + 12, y + 24)]))

                elif curTile.byte3 & 2: # Coin
                    if curTile.byte7 == 0:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Coin/Coin.png'))
                    if curTile.byte7 == 4:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Coin/POW.png'))

                elif curTile.byte3 & 8: # Exploder
                    if curTile.byte7 == 1:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Explode/Stone.png'))
                    if curTile.byte7 == 2:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Explode/Wood.png'))
                    if curTile.byte7 == 3:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Explode/Red.png'))

                elif curTile.byte1 & 2: # Falling
                    painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'Prop/Fall.png'))

                elif curTile.byte3 & 4: # QBlock
                    if curTile.byte7 == 0:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/FireF.png'))
                    if curTile.byte7 == 1:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Star.png'))
                    if curTile.byte7 == 2:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Coin.png'))
                    if curTile.byte7 == 3:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Vine.png'))
                    if curTile.byte7 == 4:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/1up.png'))
                    if curTile.byte7 == 5:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Mini.png'))
                    if curTile.byte7 == 6:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Prop.png'))
                    if curTile.byte7 == 7:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/Peng.png'))
                    if curTile.byte7 == 8:
                        painter.drawPixmap(option.rect, QtGui.QPixmap(path + 'QBlock/IceF.png'))

                elif curTile.byte3 & 1: # Solid
                    painter.drawRect(option.rect)

                else: # No fill
                    pass


            # Highlight stuff.
            colour = option.palette.highlight().color()
            colour.setAlpha(80)

            if option.state & QtWidgets.QStyle.State_Selected:
                painter.fillRect(option.rect, colour)


        def sizeHint(self, option, index):
            """Returns the size for the object"""
            return QtCore.QSize(24,24)



#############################################################################################
############################ Tile widget for drag n'drop Objects ############################


class RepeatXModifiers(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.setVisible(False)

        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.setSpacing(0)
        self.layout.setContentsMargins(0,0,0,0)

        self.spinboxes = []
        self.buttons = []

        self.updating = False


    def update(self):
        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        if not object.repeatX:
            return

        self.updating = True

        assert len(self.spinboxes) == len(self.buttons)

        height = object.height
        numRows = len(self.spinboxes)

        if numRows < height:
            for i in range(numRows, height):
                layout = QtWidgets.QHBoxLayout()
                layout.setSpacing(0)
                layout.setContentsMargins(0,0,0,0)

                spinbox1 = QtWidgets.QSpinBox()
                spinbox1.setFixedSize(50, 24)
                spinbox1.valueChanged.connect(lambda val, i=i: self.startValChanged(val, i))
                layout.addWidget(spinbox1)

                spinbox2 = QtWidgets.QSpinBox()
                spinbox2.setFixedSize(50, 24)
                spinbox2.valueChanged.connect(lambda val, i=i: self.endValChanged(val, i))
                layout.addWidget(spinbox2)

                button1 = QtWidgets.QPushButton('+')
                button1.setFixedSize(24, 24)
                button1.released.connect(lambda i=i: self.addTile(i))
                layout.addWidget(button1)

                button2 = QtWidgets.QPushButton('-')
                button2.setFixedSize(24, 24)
                button2.released.connect(lambda i=i: self.removeTile(i))
                layout.addWidget(button2)

                self.layout.addLayout(layout)
                self.spinboxes.append((spinbox1, spinbox2))
                self.buttons.append((button1, button2))

        elif height < numRows:
            for i in reversed(range(height, numRows)):
                layout = self.layout.itemAt(i).layout()
                self.layout.removeItem(layout)

                spinbox1, spinbox2 = self.spinboxes[i]
                layout.removeWidget(spinbox1)
                layout.removeWidget(spinbox2)

                spinbox1.setParent(None)
                spinbox2.setParent(None)

                del self.spinboxes[i]

                button1, button2 = self.buttons[i]
                layout.removeWidget(button1)
                layout.removeWidget(button2)

                button1.setParent(None)
                button2.setParent(None)

                del self.buttons[i]

        for y in range(height):
            spinbox1, spinbox2 = self.spinboxes[y]

            spinbox1.setRange(0, object.repeatX[y][1]-1)
            spinbox2.setRange(object.repeatX[y][0]+1, len(object.tiles[y]))

            spinbox1.setValue(object.repeatX[y][0])
            spinbox2.setValue(object.repeatX[y][1])

        self.updating = False
        self.setFixedHeight(height * 24)


    def startValChanged(self, val, y):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        object.repeatX[y][0] = val

        for x in range(len(object.tiles[y])):
            if x >= val and x < object.repeatX[y][1]:
                object.tiles[y][x] = (object.tiles[y][x][0] | 1, object.tiles[y][x][1], object.tiles[y][x][2])

            else:
                object.tiles[y][x] = (object.tiles[y][x][0] & ~1, object.tiles[y][x][1], object.tiles[y][x][2])

        spinbox1, spinbox2 = self.spinboxes[y]
        spinbox1.setRange(0, object.repeatX[y][1]-1)
        spinbox2.setRange(val+1, len(object.tiles[y]))

        window.tileWidget.tiles.update()


    def endValChanged(self, val, y):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        object.repeatX[y][1] = val

        for x in range(len(object.tiles[y])):
            if x >= object.repeatX[y][0] and x < val:
                object.tiles[y][x] = (object.tiles[y][x][0] | 1, object.tiles[y][x][1], object.tiles[y][x][2])

            else:
                object.tiles[y][x] = (object.tiles[y][x][0] & ~1, object.tiles[y][x][1], object.tiles[y][x][2])

        spinbox1, spinbox2 = self.spinboxes[y]
        spinbox1.setRange(0, val-1)
        spinbox2.setRange(object.repeatX[y][0]+1, len(object.tiles[y]))

        window.tileWidget.tiles.update()


    def addTile(self, y):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        pix = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
        pix.fill(QtGui.QColor(0,0,0,0))

        window.tileWidget.tiles.tiles[y].append(pix)

        object = Tileset.objects[index]
        if object.repeatY and y >= object.repeatY[0] and y < object.repeatY[1]:
            object.tiles[y].append((2, 0, 0))

        else:
            object.tiles[y].append((0, 0, 0))

        object.width = max(len(object.tiles[y]), object.width)

        self.update()

        window.tileWidget.tiles.size[0] = object.width
        window.tileWidget.tiles.setMinimumSize(window.tileWidget.tiles.size[0]*24 + 12, window.tileWidget.tiles.size[1]*24 + 12)

        window.tileWidget.tiles.update()
        window.tileWidget.tiles.updateList()


    def removeTile(self, y):
        if self.updating:
            return

        if window.tileWidget.tiles.size[0] == 1:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]

        row = window.tileWidget.tiles.tiles[y]
        if len(row) > 1:
            row.pop()
        else:
            return
                
        row = object.tiles[y]
        if len(row) > 1:
            row.pop()
        else:
            return

        start, end = object.repeatX[y]
        end = min(end, len(row))
        start = min(start, end - 1)

        if [start, end] != object.repeatX[y]:
            object.repeatX[y] = [start, end]
            for x in range(len(row)):
                if x >= start and x < end:
                    row[x] = (row[x][0] | 1, row[x][1], row[x][2])

                else:
                    row[x] = (row[x][0] & ~1, row[x][1], row[x][2])

        object.width = max(len(row) for row in object.tiles)

        self.update()

        window.tileWidget.tiles.size[0] = object.width
        window.tileWidget.tiles.setMinimumSize(window.tileWidget.tiles.size[0]*24 + 12, window.tileWidget.tiles.size[1]*24 + 12)

        window.tileWidget.tiles.update()
        window.tileWidget.tiles.updateList()


class RepeatYModifiers(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.setVisible(False)

        layout = QtWidgets.QHBoxLayout(self)
        layout.setSpacing(0)
        layout.setContentsMargins(0,0,0,0)

        spinbox1 = QtWidgets.QSpinBox()
        spinbox1.setFixedSize(50, 24)
        spinbox1.valueChanged.connect(self.startValChanged)
        layout.addWidget(spinbox1)

        spinbox2 = QtWidgets.QSpinBox()
        spinbox2.setFixedSize(50, 24)
        spinbox2.valueChanged.connect(self.endValChanged)
        layout.addWidget(spinbox2)

        self.spinboxes = (spinbox1, spinbox2)
        self.updating = False

        self.setFixedWidth(102)


    def update(self):
        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        if not object.repeatY:
            return

        self.updating = True

        spinbox1, spinbox2 = self.spinboxes
        spinbox1.setRange(0, object.repeatY[1]-1)
        spinbox2.setRange(object.repeatY[0]+1, object.height)

        spinbox1.setValue(object.repeatY[0])
        spinbox2.setValue(object.repeatY[1])

        self.updating = False


    def startValChanged(self, val):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        object.createRepetitionY(val, object.repeatY[1])

        spinbox1, spinbox2 = self.spinboxes
        spinbox1.setRange(0, object.repeatY[1]-1)
        spinbox2.setRange(object.repeatY[0]+1, object.height)

        window.tileWidget.tiles.update()


    def endValChanged(self, val):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        object.createRepetitionY(object.repeatY[0], val)

        spinbox1, spinbox2 = self.spinboxes
        spinbox1.setRange(0, object.repeatY[1]-1)
        spinbox2.setRange(object.repeatY[0]+1, object.height)

        window.tileWidget.tiles.update()


class SlopeLineModifier(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        self.setVisible(False)

        layout = QtWidgets.QHBoxLayout(self)
        layout.setSpacing(0)
        layout.setContentsMargins(0,0,0,0)

        self.spinbox = QtWidgets.QSpinBox()
        self.spinbox.setFixedSize(50, 24)
        self.spinbox.valueChanged.connect(self.valChanged)
        layout.addWidget(self.spinbox)

        self.updating = False

        self.setFixedWidth(51)


    def update(self):
        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        if object.upperslope[0] == 0:
            return

        self.updating = True

        self.spinbox.setRange(1, object.height)
        self.spinbox.setValue(object.upperslope[1])

        self.updating = False


    def valChanged(self, val):
        if self.updating:
            return

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        if object.height == 1:
            object.upperslope[1] = 1
            object.lowerslope = [0, 0]

        else:
            object.upperslope[1] = val
            object.lowerslope = [0x84, object.height - val]

        tiles = window.tileWidget.tiles

        if object.upperslope[0] & 2:
            tiles.slope = -object.upperslope[1]

        else:
            tiles.slope = object.upperslope[1]

        tiles.update()


class tileOverlord(QtWidgets.QWidget):

    def __init__(self):
        super(tileOverlord, self).__init__()

        # Setup Widgets
        self.tiles = tileWidget()

        self.addObject = QtWidgets.QPushButton('Add')
        self.removeObject = QtWidgets.QPushButton('Remove')

        global Tileset

        self.placeNull = QtWidgets.QPushButton('Place Null Tile')
        self.placeNull.setCheckable(True)
        self.placeNull.setChecked(Tileset.placeNullChecked)
        self.placeNull.setToolTip('When enabled, clicking on the tiles of an object will remove that tile instead of placing the currently selected one.')

        self.addRow = QtWidgets.QPushButton('Add Row')
        self.removeRow = QtWidgets.QPushButton('Remove Row')

        self.addColumn = QtWidgets.QPushButton('Add Column')
        self.removeColumn = QtWidgets.QPushButton('Remove Column')

        self.tilingMethod = QtWidgets.QComboBox()
        self.tilesetType = QtWidgets.QLabel('Type: Pa%d' % Tileset.slot)

        self.tilingMethod.addItems(['No Repetition',
                                    'Repeat X',
                                    'Repeat Y',
                                    'Repeat X and Y',
                                    'Upward slope',
                                    'Downward slope',
                                    'Downward reverse slope',
                                    'Upward reverse slope'])

        self.info = QtWidgets.QLabel("You can right click on the tiles of a object to make additional changes to it's behavior!")
        self.info.setWordWrap(True)
        self.info.setAlignment(Qt.AlignCenter)

        # Connections
        self.addObject.released.connect(self.addObj)
        self.removeObject.released.connect(self.removeObj)
        self.placeNull.toggled.connect(self.doPlaceNull)
        self.addRow.released.connect(self.addRowHandler)
        self.removeRow.released.connect(self.removeRowHandler)
        self.addColumn.released.connect(self.addColumnHandler)
        self.removeColumn.released.connect(self.removeColumnHandler)

        self.tilingMethod.currentIndexChanged.connect(self.setTiling)


        # Layout
        self.repeatX = RepeatXModifiers()
        repeatXLyt = QtWidgets.QVBoxLayout()
        repeatXLyt.addWidget(self.repeatX)

        self.repeatY = RepeatYModifiers()
        repeatYLyt = QtWidgets.QHBoxLayout()
        repeatYLyt.addWidget(self.repeatY)

        self.slopeLine = SlopeLineModifier()
        slopeLineLyt = QtWidgets.QVBoxLayout()
        slopeLineLyt.addWidget(self.slopeLine)

        tilesLyt = QtWidgets.QGridLayout()
        tilesLyt.setSpacing(0)
        tilesLyt.setContentsMargins(0,0,0,0)

        tilesLyt.addWidget(self.tiles, 0, 0, 3, 4)
        tilesLyt.addLayout(repeatXLyt, 0, 4, 3, 1)
        tilesLyt.addLayout(repeatYLyt, 3, 0, 1, 4)
        tilesLyt.addLayout(slopeLineLyt, 0, 5, 3, 1)

        layout = QtWidgets.QGridLayout()

        layout.addWidget(self.tilesetType, 0, 0, 1, 1)
        layout.addWidget(self.tilingMethod, 0, 1, 1, 4)

        layout.addWidget(self.addObject, 0, 5, 1, 1)
        layout.addWidget(self.removeObject, 0, 6, 1, 1)

        layout.setRowMinimumHeight(1, 40)

        layout.setRowStretch(1, 1)
        layout.setRowStretch(2, 5)
        layout.setRowStretch(5, 5)

        layout.addLayout(tilesLyt, 2, 1, 4, 6)
        layout.addWidget(self.info, 6, 0, 1, 7)

        layout.addWidget(self.placeNull, 1, 0, 1, 1)

        layout.addWidget(self.addColumn, 2, 0, 1, 1)
        layout.addWidget(self.removeColumn, 3, 0, 1, 1)
        layout.addWidget(self.addRow, 1, 1, 1, 1)
        layout.addWidget(self.removeRow, 1, 2, 1, 1)

        self.setLayout(layout)




    def addObj(self):
        global Tileset

        Tileset.addObject(new=True)

        pix = QtGui.QPixmap(24, 24)
        pix.fill(Qt.transparent)

        painter = QtGui.QPainter(pix)
        painter.drawImage(0, 0, Tileset.tiles[0].image)
        painter.end()
        del painter

        count = len(Tileset.objects)
        item = QtGui.QStandardItem(QtGui.QIcon(pix), 'Object {0}'.format(count-1))
        item.setEditable(False)
        window.objmodel.appendRow(item)
        index = window.objectList.currentIndex()
        window.objectList.setCurrentIndex(index)
        self.setObject(index)

        window.objectList.update()
        self.update()


    def removeObj(self):
        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        Tileset.removeObject(index)
        window.objmodel.removeRow(index)
        self.tiles.clear()

        SetupObjectModel(window.objmodel, Tileset.objects, Tileset.tiles)

        window.objectList.update()
        self.update()


    def doPlaceNull(self, checked):
        global Tileset
        Tileset.placeNullChecked = checked


    def setObject(self, index):
        global Tileset

        self.tiles.object = index.row()
        if self.tiles.object < 0 or self.tiles.object >= len(Tileset.objects):
            return

        object = Tileset.objects[index.row()]

        self.tilingMethod.setCurrentIndex(object.determineTilingMethod())
        self.tiles.setObject(object)


    @QtCoreSlot(int)
    def setTiling(self, listindex):
        if listindex == 0:  # No Repetition
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(False)

        elif listindex == 1:  # Repeat X
            self.repeatX.setVisible(True)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(False)

        elif listindex == 2:  # Repeat Y
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(True)
            self.slopeLine.setVisible(False)

        elif listindex == 3:  # Repeat X and Y
            self.repeatX.setVisible(True)
            self.repeatY.setVisible(True)
            self.slopeLine.setVisible(False)

        elif listindex == 4:  # Upward Slope
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(True)

        elif listindex == 5:  # Downward Slope
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(True)

        elif listindex == 6:  # Upward Reverse Slope
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(True)

        elif listindex == 7:  # Downward Reverse Slope
            self.repeatX.setVisible(False)
            self.repeatY.setVisible(False)
            self.slopeLine.setVisible(True)

        global Tileset

        index = window.objectList.currentIndex().row()
        if index < 0 or index >= len(Tileset.objects):
            return

        object = Tileset.objects[index]
        if object.tilingMethodIdx == listindex:
            return

        object.tilingMethodIdx = listindex
        self.tiles.slope = 0

        if listindex == 0:  # No Repetition
            object.clearRepetitionXY()

            object.upperslope = [0, 0]
            object.lowerslope = [0, 0]

        elif listindex == 1:  # Repeat X
            object.clearRepetitionY()

            if not object.repeatX:
                object.createRepetitionX()
                self.repeatX.update()

            object.upperslope = [0, 0]
            object.lowerslope = [0, 0]

        elif listindex == 2:  # Repeat Y
            object.clearRepetitionX()

            if not object.repeatY:
                object.createRepetitionY(0, object.height)
                self.repeatY.update()

            object.upperslope = [0, 0]
            object.lowerslope = [0, 0]

        elif listindex == 3:  # Repeat X and Y
            if not object.repeatX:
                object.createRepetitionX()
                self.repeatX.update()

            if not object.repeatY:
                object.createRepetitionY(0, object.height)
                self.repeatY.update()

            object.upperslope = [0, 0]
            object.lowerslope = [0, 0]

        elif listindex == 4:  # Upward Slope
            object.clearRepetitionXY()

            if object.upperslope[0] != 0x90:
                object.upperslope = [0x90, 1]

                if object.height == 1:
                    object.lowerslope = [0, 0]

                else:
                    object.lowerslope = [0x84, object.height - 1]

            self.tiles.slope = object.upperslope[1]
            self.slopeLine.update()

        elif listindex == 5:  # Downward Slope
            object.clearRepetitionXY()

            if object.upperslope[0] != 0x91:
                object.upperslope = [0x91, 1]

                if object.height == 1:
                    object.lowerslope = [0, 0]

                else:
                    object.lowerslope = [0x84, object.height - 1]

            self.tiles.slope = object.upperslope[1]
            self.slopeLine.update()

        elif listindex == 6:  # Upward Reverse Slope
            object.clearRepetitionXY()

            if object.upperslope[0] != 0x92:
                object.upperslope = [0x92, 1]

                if object.height == 1:
                    object.lowerslope = [0, 0]

                else:
                    object.lowerslope = [0x84, object.height - 1]

            self.tiles.slope = -object.upperslope[1]
            self.slopeLine.update()

        elif listindex == 7:  # Downward Reverse Slope
            object.clearRepetitionXY()

            if object.upperslope[0] != 0x93:
                object.upperslope = [0x93, 1]

                if object.height == 1:
                    object.lowerslope = [0, 0]

                else:
                    object.lowerslope = [0x84, object.height - 1]

            self.tiles.slope = -object.upperslope[1]
            self.slopeLine.update()

        self.tiles.update()


    def addRowHandler(self):
        index = window.objectList.currentIndex()
        self.tiles.object = index.row()

        if self.tiles.object < 0 or self.tiles.object >= len(Tileset.objects):
            return

        self.tiles.addRow()


    def removeRowHandler(self):
        index = window.objectList.currentIndex()
        self.tiles.object = index.row()

        if self.tiles.object < 0 or self.tiles.object >= len(Tileset.objects):
            return

        self.tiles.removeRow()


    def addColumnHandler(self):
        index = window.objectList.currentIndex()
        self.tiles.object = index.row()

        if self.tiles.object < 0 or self.tiles.object >= len(Tileset.objects):
            return

        self.tiles.addColumn()


    def removeColumnHandler(self):
        index = window.objectList.currentIndex()
        self.tiles.object = index.row()

        if self.tiles.object < 0 or self.tiles.object >= len(Tileset.objects):
            return

        self.tiles.removeColumn()


class tileWidget(QtWidgets.QWidget):

    def __init__(self):
        super(tileWidget, self).__init__()

        self.tiles = []

        self.size = [1, 1]
        self.setMinimumSize(36, 36)  # (24, 24) + padding

        self.slope = 0

        self.highlightedRect = QtCore.QRect()

        self.setAcceptDrops(True)
        self.object = -1


    def clear(self):
        self.tiles = []
        self.size = [1, 1] # [width, height]

        self.slope = 0
        self.highlightedRect = QtCore.QRect()

        self.update()


    def addColumn(self):
        global Tileset

        if self.size[0] >= 24:
            return

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        self.size[0] += 1
        self.setMinimumSize(self.size[0]*24 + 12, self.size[1]*24 + 12)

        curObj = Tileset.objects[self.object]
        curObj.width += 1

        pix = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
        pix.fill(QtGui.QColor(0,0,0,0))

        for row in self.tiles:
            row.append(pix)

        if curObj.repeatY:
            for y, row in enumerate(curObj.tiles):
                if y >= curObj.repeatY[0] and y < curObj.repeatY[1]:
                    row.append((2, 0, 0))

                else:
                    row.append((0, 0, 0))

        else:
            for row in curObj.tiles:
                row.append((0, 0, 0))

        self.update()
        self.updateList()

        window.tileWidget.repeatX.update()


    def removeColumn(self):
        global Tileset

        if self.size[0] == 1:
            return

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        self.size[0] -= 1
        self.setMinimumSize(self.size[0]*24 + 12, self.size[1]*24 + 12)

        curObj = Tileset.objects[self.object]
        curObj.width -= 1

        for row in self.tiles:
            if len(row) > 1:
                row.pop()

        for row in curObj.tiles:
            if len(row) > 1:
                row.pop()

        if curObj.repeatX:
            for y, row in enumerate(curObj.tiles):
                start, end = curObj.repeatX[y]
                end = min(end, len(row))
                start = min(start, end - 1)

                if [start, end] != curObj.repeatX[y]:
                    curObj.repeatX[y] = [start, end]
                    for x in range(len(row)):
                        if x >= start and x < end:
                            row[x] = (row[x][0] | 1, row[x][1], row[x][2])

                        else:
                            row[x] = (row[x][0] & ~1, row[x][1], row[x][2])

        self.update()
        self.updateList()

        window.tileWidget.repeatX.update()


    def addRow(self):
        global Tileset

        if self.size[1] >= 24:
            return

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        self.size[1] += 1
        self.setMinimumSize(self.size[0]*24 + 12, self.size[1]*24 + 12)

        curObj = Tileset.objects[self.object]
        curObj.height += 1

        pix = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
        pix.fill(QtGui.QColor(0,0,0,0))

        self.tiles.append([pix for _ in range(curObj.width)])

        if curObj.repeatX:
            curObj.tiles.append([(1, 0, 0) for _ in range(curObj.width)])
            curObj.repeatX.append([0, curObj.width])

        else:
            curObj.tiles.append([(0, 0, 0) for _ in range(curObj.width)])

        if curObj.upperslope[0] != 0:
            curObj.lowerslope = [0x84, curObj.lowerslope[1] + 1]

        self.update()
        self.updateList()

        window.tileWidget.repeatX.update()
        window.tileWidget.repeatY.update()
        window.tileWidget.slopeLine.update()


    def removeRow(self):
        global Tileset

        if self.size[1] == 1:
            return

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        self.tiles.pop()

        self.size[1] -= 1
        self.setMinimumSize(self.size[0]*24 + 12, self.size[1]*24 + 12)

        curObj = Tileset.objects[self.object]
        curObj.tiles = list(curObj.tiles)
        curObj.height -= 1

        curObj.tiles.pop()

        if curObj.repeatX:
            curObj.repeatX.pop()

        if curObj.repeatY:
            start, end = curObj.repeatY
            end = min(end, curObj.height)
            start = min(start, end - 1)

            if [start, end] != curObj.repeatY:
                curObj.createRepetitionY(start, end)

        if curObj.upperslope[0] != 0:
            if curObj.upperslope[1] > curObj.height or curObj.height == 1:
                curObj.upperslope[1] = curObj.height
                curObj.lowerslope = [0, 0]

                if curObj.upperslope[0] & 2:
                    self.slope = -curObj.upperslope[1]
                else:
                    self.slope = curObj.upperslope[1]

            else:
                curObj.lowerslope = [0x84, curObj.lowerslope[1] - 1]

        self.update()
        self.updateList()

        window.tileWidget.repeatX.update()
        window.tileWidget.repeatY.update()
        window.tileWidget.slopeLine.update()


    def setObject(self, object):
        self.clear()

        global Tileset

        self.size = [object.width, object.height]
        self.setMinimumSize(self.size[0]*24 + 12, self.size[1]*24 + 12)

        if not object.upperslope[1] == 0:
            if object.upperslope[0] & 2:
                self.slope = -object.upperslope[1]
            else:
                self.slope = object.upperslope[1]

        x = 0
        y = 0
        for row in object.tiles:
            self.tiles.append([])
            for tile in row:
                if Tileset.slot == (tile[2] & 3):
                    self.tiles[-1].append(Tileset.tiles[tile[1]].image)
                else:
                    img = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
                    img.fill(QtCore.Qt.transparent)
                    self.tiles[-1].append(img)
                x += 1
            y += 1
            x = 0


        self.object = window.objectList.currentIndex().row()
        self.update()
        self.updateList()

        window.tileWidget.repeatX.update()
        window.tileWidget.repeatY.update()
        window.tileWidget.slopeLine.update()


    def contextMenuEvent(self, event):
        index = window.objectList.currentIndex()
        self.object = index.row()

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        centerPoint = self.contentsRect().center()

        upperLeftX = centerPoint.x() - self.size[0]*12
        upperLeftY = centerPoint.y() - self.size[1]*12

        x = int((event.x() - upperLeftX) / 24)
        y = int((event.y() - upperLeftY) / 24)

        object = Tileset.objects[self.object]

        if y < 0 or y >= object.height or x < 0 or x >= len(object.tiles[y]):
            return

        self.contX = x
        self.contY = y

        TileMenu = QtWidgets.QMenu(self)

        TileMenu.addAction('Set tile...', self.setTile)
        TileMenu.addAction('Set item...', self.setItem)
        TileMenu.addAction('Set raw data...', self.setRaw)

        TileMenu.exec_(event.globalPos())

    def mouseMoveEvent(self, event):
        if app.mouseButtons() == QtCore.Qt.LeftButton:
            self.mousePressEvent(event)

    def mousePressEvent(self, event):
        global Tileset

        if event.button() == 2:
            return

        index = window.objectList.currentIndex()
        self.object = index.row()

        if self.object < 0 or self.object >= len(Tileset.objects):
            return

        if Tileset.placeNullChecked:
            centerPoint = self.contentsRect().center()

            upperLeftX = centerPoint.x() - self.size[0]*12
            upperLeftY = centerPoint.y() - self.size[1]*12

            lowerRightX = centerPoint.x() + self.size[0]*12
            lowerRightY = centerPoint.y() + self.size[1]*12


            x = int((event.x() - upperLeftX)/24)
            y = int((event.y() - upperLeftY)/24)

            if event.x() < upperLeftX or event.y() < upperLeftY or event.x() > lowerRightX or event.y() > lowerRightY:
                return

            if Tileset.slot == 0:
                try:
                    self.tiles[y][x] = Tileset.tiles[0].image
                    Tileset.objects[self.object].tiles[y][x] = (Tileset.objects[self.object].tiles[y][x][0], 0, 0)
                except IndexError:
                    pass

            else:
                img = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
                img.fill(QtCore.Qt.transparent)

                try:
                    self.tiles[y][x] = img
                    Tileset.objects[self.object].tiles[y][x] = (Tileset.objects[self.object].tiles[y][x][0], 0, 0)
                except IndexError:
                    pass

        else:
            if window.tileDisplay.selectedIndexes() == []:
                return

            currentSelected = window.tileDisplay.selectedIndexes()

            ix = 0
            iy = 0
            for modelItem in currentSelected:
                # Update yourself!
                centerPoint = self.contentsRect().center()

                tile = modelItem.row()
                upperLeftX = centerPoint.x() - self.size[0]*12
                upperLeftY = centerPoint.y() - self.size[1]*12

                lowerRightX = centerPoint.x() + self.size[0]*12
                lowerRightY = centerPoint.y() + self.size[1]*12


                x = int((event.x() - upperLeftX)/24 + ix)
                y = int((event.y() - upperLeftY)/24 + iy)

                if event.x() < upperLeftX or event.y() < upperLeftY or event.x() > lowerRightX or event.y() > lowerRightY:
                    return

                try:
                    self.tiles[y][x] = Tileset.tiles[tile].image
                    Tileset.objects[self.object].tiles[y][x] = (Tileset.objects[self.object].tiles[y][x][0], tile, Tileset.slot)
                except IndexError:
                    pass

                ix += 1
                if self.size[0]-1 < ix:
                    ix = 0
                    iy += 1
                if iy > self.size[1]-1:
                    break


        self.update()

        self.updateList()


    def updateList(self):
        # Update the list >.>
        object = window.objmodel.itemFromIndex(window.objectList.currentIndex())
        if not object: return


        tex = QtGui.QPixmap(self.size[0] * 24, self.size[1] * 24)
        tex.fill(Qt.transparent)
        painter = QtGui.QPainter(tex)

        Xoffset = 0
        Yoffset = 0

        for y, row in enumerate(self.tiles):
            for x, tile in enumerate(row):
                painter.drawImage(x*24, y*24, tile)

        painter.end()

        object.setIcon(QtGui.QIcon(tex))

        window.objectList.update()



    def setTile(self):
        global Tileset

        dlg = self.setTileDialog()
        if dlg.exec_() == QtWidgets.QDialog.Accepted:
            # Do stuff
            tile = dlg.tile.value()
            tileset = dlg.tileset.currentIndex()

            x = self.contX
            y = self.contY

            if tileset != Tileset.slot:
                tex = QtGui.QPixmap(self.size[0] * 24, self.size[1] * 24)
                tex.fill(Qt.transparent)

                self.tiles[y][x] = tex

            else:
                self.tiles[y][x] = Tileset.tiles[tile].image

            Tileset.objects[self.object].tiles[y][x] = (Tileset.objects[self.object].tiles[y][x][0], tile, tileset)

            self.update()
            self.updateList()


    class setTileDialog(QtWidgets.QDialog):

        def __init__(self):
            QtWidgets.QDialog.__init__(self)

            self.setWindowTitle('Set tiles')

            self.tileset = QtWidgets.QComboBox()
            self.tileset.addItems(['Pa0', 'Pa1', 'Pa2', 'Pa3'])

            self.tile = QtWidgets.QSpinBox()
            self.tile.setRange(0, 255)

            self.buttons = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel)
            self.buttons.accepted.connect(self.accept)
            self.buttons.rejected.connect(self.reject)

            self.layout = QtWidgets.QGridLayout()
            self.layout.addWidget(QtWidgets.QLabel('Tileset:'), 0,0,1,1, Qt.AlignLeft)
            self.layout.addWidget(QtWidgets.QLabel('Tile:'), 0,3,1,1, Qt.AlignLeft)
            self.layout.addWidget(self.tileset, 1, 0, 1, 2)
            self.layout.addWidget(self.tile, 1, 3, 1, 3)
            self.layout.addWidget(self.buttons, 2, 3)
            self.setLayout(self.layout)


    def setItem(self):
        global Tileset

        x = self.contX
        y = self.contY

        obj = Tileset.objects[self.object].tiles[y][x]

        dlg = self.setItemDialog(obj[2] >> 2)
        if dlg.exec_() == QtWidgets.QDialog.Accepted:
            # Do stuff
            item = dlg.item.currentIndex()

            Tileset.objects[self.object].tiles[y][x] = (obj[0], obj[1], (obj[2] & 3) | (item << 2))

            self.update()
            self.updateList()


    def setRaw(self):
        global Tileset

        x = self.contX
        y = self.contY

        obj = Tileset.objects[self.object].tiles[y][x]

        dlg = self.setRawDialog(obj)
        if dlg.exec_() == QtWidgets.QDialog.Accepted:
            # Do stuff
            repeating = dlg.repeating.value()
            tilenum = dlg.tilenum.value()
            slot = dlg.slot.value()
            item = dlg.item.value()

            Tileset.objects[self.object].tiles[y][x] = (repeating, tilenum, (slot & 3) | (item << 2))

            window.tileWidget.setObject(window.objectList.currentIndex())
            window.tileWidget.update()
            self.update()
            self.updateList()


    class setItemDialog(QtWidgets.QDialog):

        def __init__(self, initialIndex=0):
            QtWidgets.QDialog.__init__(self)

            self.setWindowTitle('Set item')

            self.item = QtWidgets.QComboBox()
            self.item.addItems([
                'Item specified in tile behavior',
                'Fire Flower',
                'Star',
                'Coin',
                'Vine',
                'Spring',
                'Mini Mushroom',
                'Propeller Mushroom',
                'Penguin Suit',
                'Yoshi',
                'Ice Flower',
                'Unknown (11)',
                'Unknown (12)',
                'Unknown (13)',
                'Unknown (14)'])
            self.item.setCurrentIndex(initialIndex)

            self.buttons = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel)
            self.buttons.accepted.connect(self.accept)
            self.buttons.rejected.connect(self.reject)

            self.layout = QtWidgets.QHBoxLayout()
            self.vlayout = QtWidgets.QVBoxLayout()
            self.layout.addWidget(QtWidgets.QLabel('Item:'))
            self.layout.addWidget(self.item)
            self.vlayout.addLayout(self.layout)
            self.vlayout.addWidget(self.buttons)
            self.setLayout(self.vlayout)


    class setRawDialog(QtWidgets.QDialog):

        def __init__(self, values):
            QtWidgets.QDialog.__init__(self)

            self.setWindowTitle('Set raw bytes')

            self.repeating  = QtWidgets.QSpinBox()
            self.repeating.setRange(0, 255)
            self.repeating.setValue(values[0])
            
            self.tilenum = QtWidgets.QSpinBox()
            self.tilenum.setRange(0, 255)
            self.tilenum.setValue(values[1])
            
            self.slot = QtWidgets.QSpinBox()
            self.slot.setRange(0, 3)
            self.slot.setValue(values[2] & 3)
            
            self.item = QtWidgets.QSpinBox()
            self.item.setRange(0, 63)
            self.item.setValue((values[2] >> 2) & 63)

            self.buttons = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel)
            self.buttons.accepted.connect(self.accept)
            self.buttons.rejected.connect(self.reject)

            self.layout = QtWidgets.QHBoxLayout()
            self.vlayout = QtWidgets.QVBoxLayout()
            self.layout.addWidget(QtWidgets.QLabel('Repeating:'))
            self.layout.addWidget(self.repeating)
            self.layout.addWidget(QtWidgets.QLabel('Tilenum:'))
            self.layout.addWidget(self.tilenum)
            self.layout.addWidget(QtWidgets.QLabel('Slot:'))
            self.layout.addWidget(self.slot)
            self.layout.addWidget(QtWidgets.QLabel('Item:'))
            self.layout.addWidget(self.item)
            self.vlayout.addLayout(self.layout)
            self.vlayout.addWidget(self.buttons)
            self.setLayout(self.vlayout)


    def paintEvent(self, event):
        painter = QtGui.QPainter()
        painter.begin(self)

        centerPoint = self.contentsRect().center()
        upperLeftX = centerPoint.x() - self.size[0]*12
        lowerRightX = centerPoint.x() + self.size[0]*12

        upperLeftY = centerPoint.y() - self.size[1]*12
        lowerRightY = centerPoint.y() + self.size[1]*12

        index = window.objectList.currentIndex()
        self.object = index.row()

        if self.object < 0 or self.object >= len(Tileset.objects):
            painter.end()
            return

        object = Tileset.objects[self.object]
        for y, row in enumerate(object.tiles):
            painter.fillRect(upperLeftX, upperLeftY + y * 24, len(row) * 24, 24, QtGui.QColor(175, 175, 175))

        for y, row in enumerate(self.tiles):
            for x, pix in enumerate(row):
                painter.drawImage(upperLeftX + (x * 24), upperLeftY + (y * 24), pix)

        if object.upperslope[0] & 0x80:
            pen = QtGui.QPen()
            pen.setStyle(Qt.DashLine)
            pen.setWidth(2)
            pen.setColor(QtGui.QColor(0, 255, 255))
            painter.setPen(QtGui.QPen(pen))

            slope = self.slope
            if slope < 0:
                slope += self.size[1]

            painter.drawLine(upperLeftX, upperLeftY + (slope * 24), lowerRightX, upperLeftY + (slope * 24))

            font = painter.font()
            font.setPixelSize(8)
            font.setFamily('Monaco')
            painter.setFont(font)

            if self.slope > 0:
                painter.drawText(upperLeftX+1, upperLeftY+10, 'Main')
                painter.drawText(upperLeftX+1, upperLeftY + (slope * 24) + 9, 'Sub')

            else:
                painter.drawText(upperLeftX+1, upperLeftY + self.size[1]*24 - 4, 'Main')
                painter.drawText(upperLeftX+1, upperLeftY + (slope * 24) - 3, 'Sub')

        if 0 <= self.object < len(Tileset.objects):
            object = Tileset.objects[self.object]
            if object.repeatX:
                pen = QtGui.QPen()
                pen.setStyle(Qt.DashLine)
                pen.setWidth(2)
                pen.setColor(QtGui.QColor(0, 255, 255))
                painter.setPen(QtGui.QPen(pen))

                for y in range(object.height):
                    startX, endX = object.repeatX[y]
                    painter.drawLine(upperLeftX + startX * 24, upperLeftY + y * 24, upperLeftX + startX * 24, upperLeftY + y * 24 + 24)
                    painter.drawLine(upperLeftX +   endX * 24, upperLeftY + y * 24, upperLeftX +   endX * 24, upperLeftY + y * 24 + 24)

            if object.repeatY:
                pen = QtGui.QPen()
                pen.setStyle(Qt.DashLine)
                pen.setWidth(2)
                pen.setColor(QtGui.QColor(255, 0, 255))
                painter.setPen(QtGui.QPen(pen))

                painter.drawLine(upperLeftX, upperLeftY + object.repeatY[0] * 24, lowerRightX, upperLeftY + object.repeatY[0] * 24)
                painter.drawLine(upperLeftX, upperLeftY + object.repeatY[1] * 24, lowerRightX, upperLeftY + object.repeatY[1] * 24)

        painter.end()


#############################################################################################
############################ Subclassed one dimension Item Model ############################


class PiecesModel(QtCore.QAbstractListModel):
    def __init__(self, parent=None):
        super(PiecesModel, self).__init__(parent)

        self.pixmaps = []

    def supportedDragActions(self):
        super().supportedDragActions()
        return Qt.CopyAction | Qt.MoveAction | Qt.LinkAction

    def data(self, index, role=Qt.DisplayRole):
        if not index.isValid():
            return None

        if role == Qt.DecorationRole:
            return QtGui.QIcon(self.pixmaps[index.row()])

        if role == Qt.UserRole:
            return self.pixmaps[index.row()]

        return None

    def addPieces(self, pixmap):
        row = len(self.pixmaps)

        self.beginInsertRows(QtCore.QModelIndex(), row, row)
        self.pixmaps.insert(row, pixmap)
        self.endInsertRows()

    def flags(self,index):
        if index.isValid():
            return (Qt.ItemIsEnabled | Qt.ItemIsSelectable |
                    Qt.ItemIsDragEnabled)

    def clear(self):
        row = len(self.pixmaps)

        del self.pixmaps[:]


    def mimeTypes(self):
        return ['image/x-tile-piece']


    def mimeData(self, indexes):
        mimeData = QtCore.QMimeData()
        encodedData = QtCore.QByteArray()

        stream = QtCore.QDataStream(encodedData, QtCore.QIODevice.WriteOnly)

        for index in indexes:
            if index.isValid():
                pixmap = QtGui.QPixmap(self.data(index, Qt.UserRole))
                stream << pixmap

        mimeData.setData('image/x-tile-piece', encodedData)
        return mimeData


    def rowCount(self, parent):
        if parent.isValid():
            return 0
        else:
            return len(self.pixmaps)

    def supportedDragActions(self):
        return Qt.CopyAction | Qt.MoveAction



#############################################################################################
################## Python-based RGB5a3 Decoding code from my BRFNT program ##################


RGB4A3LUT = []
RGB4A3LUT_NoAlpha = []
def PrepareRGB4A3LUTs():
    global RGB4A3LUT, RGB4A3LUT_NoAlpha

    RGB4A3LUT = [None] * 0x10000
    RGB4A3LUT_NoAlpha = [None] * 0x10000
    for LUT, hasA in [(RGB4A3LUT, True), (RGB4A3LUT_NoAlpha, False)]:

        # RGB4A3
        for d in range(0x8000):
            if hasA:
                alpha = d >> 12
                alpha = alpha << 5 | alpha << 2 | alpha >> 1
            else:
                alpha = 0xFF
            red = ((d >> 8) & 0xF) * 17
            green = ((d >> 4) & 0xF) * 17
            blue = (d & 0xF) * 17
            LUT[d] = blue | (green << 8) | (red << 16) | (alpha << 24)

        # RGB555
        for d in range(0x8000):
            red = d >> 10
            red = red << 3 | red >> 2
            green = (d >> 5) & 0x1F
            green = green << 3 | green >> 2
            blue = d & 0x1F
            blue = blue << 3 | blue >> 2
            LUT[d + 0x8000] = blue | (green << 8) | (red << 16) | 0xFF000000

PrepareRGB4A3LUTs()


def RGB4A3Decode(tex, useAlpha=True):
    tx = 0; ty = 0
    iter = tex.__iter__()
    dest = [0] * 262144

    LUT = RGB4A3LUT if useAlpha else RGB4A3LUT_NoAlpha

    # Loop over all texels (of which there are 16384)
    for i in range(16384):
        temp1 = (i // 256) % 8
        if temp1 == 0 or temp1 == 7:
            # Skip every row of texels that is a multiple of 8 or (a
            # multiple of 8) - 1
            # Unrolled loop for performance.
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
            next(iter); next(iter); next(iter); next(iter)
        else:
            temp2 = i % 8
            if temp2 == 0 or temp2 == 7:
                # Skip every column of texels that is a multiple of 8
                # or (a multiple of 8) - 1
                # Unrolled loop for performance.
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
                next(iter); next(iter); next(iter); next(iter)
            else:
                # Actually render this texel
                for y in range(ty, ty+4):
                    for x in range(tx, tx+4):
                        dest[x + y * 1024] = LUT[next(iter) << 8 | next(iter)]

        # Move on to the next texel
        tx += 4
        if tx >= 1024: tx = 0; ty += 4

    # Convert the list of ARGB color values into a bytes object, and
    # then convert that into a QImage
    return QtGui.QImage(struct.pack('<262144I', *dest), 1024, 256, QtGui.QImage.Format_ARGB32)


def color_transparent_pixels_around_edges_24_24(data: bytearray) -> None:
    """
    THIS FUNCTION IS AUTO-GENERATED SOURCE CODE!
    See make_color_transparent_pixels_around_edges.py.
    Find fully-transparent pixels that border non-fully-transparent
    pixels, and set their RGB channels to the average of those of their
    neighboring non-fully-transparent pixels. This solves the
    longstanding "black outlines around tile edges" bug.
    "data" should be BGRA8 bytes for an image of size 24x24.
    """
    if len(data) != 0x900:
        raise ValueError(f'expected 0x900 bytes, got {len(data):#x}')

    # Redefine some globals as locals for faster lookups
    sum_loc, len_loc, range_loc = sum, len, range

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Top-left corner

    if not data[0x3]:
        neighbors = []

        # (x + 1, y)
        if data[0x7]:
            neighbors.append(data[0x4 : 0x7])

        # (x + 1, y + 1)
        if data[0x67]:
            neighbors.append(data[0x64 : 0x67])

        # (x, y + 1)
        if data[0x63]:
            neighbors.append(data[0x60 : 0x63])

        if neighbors:
            ln = len_loc(neighbors)
            data[0x0] = sum_loc(n[0] for n in neighbors) // ln
            data[0x1] = sum_loc(n[1] for n in neighbors) // ln
            data[0x2] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Top-right corner

    if not data[0x5f]:
        neighbors = []

        # (x, y + 1)
        if data[0xbf]:
            neighbors.append(data[0xbc : 0xbf])

        # (x - 1, y + 1)
        if data[0xbb]:
            neighbors.append(data[0xb8 : 0xbb])

        # (x - 1, y)
        if data[0x5b]:
            neighbors.append(data[0x58 : 0x5b])

        if neighbors:
            ln = len_loc(neighbors)
            data[0x5c] = sum_loc(n[0] for n in neighbors) // ln
            data[0x5d] = sum_loc(n[1] for n in neighbors) // ln
            data[0x5e] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Bottom-left corner

    if not data[0x8a3]:
        neighbors = []

        # (x, y - 1)
        if data[0x843]:
            neighbors.append(data[0x840 : 0x843])

        # (x + 1, y - 1)
        if data[0x847]:
            neighbors.append(data[0x844 : 0x847])

        # (x + 1, y)
        if data[0x8a7]:
            neighbors.append(data[0x8a4 : 0x8a7])

        if neighbors:
            ln = len_loc(neighbors)
            data[0x8a0] = sum_loc(n[0] for n in neighbors) // ln
            data[0x8a1] = sum_loc(n[1] for n in neighbors) // ln
            data[0x8a2] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Bottom-right corner

    if not data[0x8ff]:
        neighbors = []

        # (x - 1, y)
        if data[0x8fb]:
            neighbors.append(data[0x8f8 : 0x8fb])

        # (x - 1, y - 1)
        if data[0x89b]:
            neighbors.append(data[0x898 : 0x89b])

        # (x, y - 1)
        if data[0x89f]:
            neighbors.append(data[0x89c : 0x89f])

        if neighbors:
            ln = len_loc(neighbors)
            data[0x8fc] = sum_loc(n[0] for n in neighbors) // ln
            data[0x8fd] = sum_loc(n[1] for n in neighbors) // ln
            data[0x8fe] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Top edge, except corners

    for offset in range_loc(0x7, 0x5f, 0x4):
        if not data[offset]:
            neighbors = []

            # (x + 1, y)
            offset += 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y + 1)
            offset += 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x, y + 1)
            offset -= 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y + 1)
            offset -= 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y)
            offset -= 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            if neighbors:
                ln = len_loc(neighbors)
                data[offset + 1] = sum_loc(n[0] for n in neighbors) // ln
                data[offset + 2] = sum_loc(n[1] for n in neighbors) // ln
                data[offset + 3] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Bottom edge, except corners

    for offset in range_loc(0x8a7, 0x8ff, 0x4):
        if not data[offset]:
            neighbors = []

            # (x - 1, y)
            offset -= 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y - 1)
            offset -= 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x, y - 1)
            offset += 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y - 1)
            offset += 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y)
            offset += 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            if neighbors:
                ln = len_loc(neighbors)
                data[offset - 7] = sum_loc(n[0] for n in neighbors) // ln
                data[offset - 6] = sum_loc(n[1] for n in neighbors) // ln
                data[offset - 5] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Left edge, except corners

    for offset in range_loc(0x63, 0x8a3, 0x60):
        if not data[offset]:
            neighbors = []

            # (x, y - 1)
            offset -= 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y - 1)
            offset += 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y)
            offset += 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x + 1, y + 1)
            offset += 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x, y + 1)
            offset -= 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            if neighbors:
                offset -= 0x63
                ln = len_loc(neighbors)
                data[offset] = sum_loc(n[0] for n in neighbors) // ln
                data[offset + 1] = sum_loc(n[1] for n in neighbors) // ln
                data[offset + 2] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Right edge, except corners

    for offset in range_loc(0xbf, 0x8ff, 0x60):
        if not data[offset]:
            neighbors = []

            # (x, y + 1)
            offset += 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y + 1)
            offset -= 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y)
            offset -= 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x - 1, y - 1)
            offset -= 0x60
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            # (x, y - 1)
            offset += 0x4
            if data[offset]:
                neighbors.append(data[offset - 3 : offset])

            if neighbors:
                offset += 0x5d
                ln = len_loc(neighbors)
                data[offset] = sum_loc(n[0] for n in neighbors) // ln
                data[offset + 1] = sum_loc(n[1] for n in neighbors) // ln
                data[offset + 2] = sum_loc(n[2] for n in neighbors) // ln

    # ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    # Main body

    for row_start_offs in range_loc(0x67, 0x8a7, 0x60):
        for offset in range_loc(row_start_offs, row_start_offs + 0x58, 0x4):
            if not data[offset]:  # (fully transparent pixel)
                neighbors = []

                # (x, y - 1)
                offset -= 0x60
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x + 1, y - 1)
                offset += 0x4
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x + 1, y)
                offset += 0x60
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x + 1, y + 1)
                offset += 0x60
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x, y + 1)
                offset -= 0x4
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x - 1, y + 1)
                offset -= 0x4
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x - 1, y)
                offset -= 0x60
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                # (x - 1, y - 1)
                offset -= 0x60
                if data[offset]:
                    neighbors.append(data[offset - 3 : offset])

                if neighbors:
                    offset += 0x61
                    ln = len_loc(neighbors)
                    data[offset] = sum_loc(n[0] for n in neighbors) // ln
                    data[offset + 1] = sum_loc(n[1] for n in neighbors) // ln
                    data[offset + 2] = sum_loc(n[2] for n in neighbors) // ln


def RGB4A3Encode(tex):
    assert len(tex) == (1024 * 256 * 4)

    shorts = []
    colorCache = {}
    for ytile in range(0, 256, 4):
        for xtile in range(0, 1024, 4):
            for ypixel in range(ytile, ytile + 4):
                for xpixel in range(xtile, xtile + 4):

                    pixel = tex[ypixel * 4096 + xpixel * 4 : ypixel * 4096 + (xpixel + 1) * 4]

                    if pixel in colorCache:
                        rgba = colorCache[pixel]

                    else:
                        b, g, r, a = pixel

                        # See encodingTests.py for verification that these
                        # channel conversion formulas are 100% correct

                        # Note: we can't do
                        # if a < 19:
                        #     rgba = 0
                        # for speed, because that causes an issue with
                        # texture filtering that results in graphics
                        # having faint black borders in-game

                        if a < 238:  # RGB4A3
                            a = ((a + 18) << 1) // 73
                            r = (r + 8) // 17
                            g = (g + 8) // 17
                            b = (b + 8) // 17

                            # 0aaarrrrggggbbbb
                            rgba = (a << 12) | (r << 8) | (g << 4) | b

                        else:  # RGB555
                            r = ((r + 4) << 2) // 33
                            g = ((g + 4) << 2) // 33
                            b = ((b + 4) << 2) // 33

                            # 1rrrrrgggggbbbbb
                            rgba = 0x8000 | (r << 10) | (g << 5) | b

                        colorCache[pixel] = rgba

                    shorts.append(rgba)

                    if xtile % 32 == 0 or xtile % 32 == 28:
                        shorts.append(rgba)
                        shorts.append(rgba)
                        shorts.append(rgba)
                        break
                if ytile % 32 == 0 or ytile % 32 == 28:
                    shorts.extend(shorts[-4:])
                    shorts.extend(shorts[-8:])
                    break

    return struct.pack('>262144H', *shorts)


#############################################################################################
############ Main Window Class. Takes care of menu functions and widget creation ############


class MainWindow(QtWidgets.QMainWindow):
    MaxRecentFiles = 9

    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

        self.recentFileActs = []
        self.recentFiles = []
        self.readRececntFiles()

        self.alpha = True

        global Tileset
        Tileset = TilesetClass()

        self.name = ''

        self.setupMenus()
        self.updateRecentFileActions()
        self.readIni()
        self.setupWidgets()

        self.setuptile()

        self.newTileset()
        
        self.setColorMode()

        self.settingsInit()

        self.setSizePolicy(QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed))
        self.setWindowTitle("New Tileset")


        if self.tilesetPath:
            self.openTilesetFromPath(self.tilesetPath)


    def readRececntFiles(self):
        f = open("Other/recent.txt", "r")
        self.recentFiles = f.read().splitlines()


    def saveRececntFiles(self):
        f = open("Other/recent.txt", "w")
        f.write("\n".join(self.recentFiles))


    def readIni(self):
        self.tilesetPath = ""
        self.tilesetDialoguePath = ""

        self.animTilesPath = ""
        self.animTiles_JSON_DialoguePath = ""
        self.animTiles_BIN_DialoguePath = ""

        self.randTilesPath = ""
        self.randTiles_JSON_DialoguePath = ""
        self.randTiles_BIN_DialoguePath = ""

        self.geometrySettings = None
        self.windowStateSettings = None
        self.isDarkMode = True
        self.isSplitWindow = False

        f = open("Other/settings.ini", "r")
        for line in f.read().splitlines():
            attr = line.split("=")
            if attr[0] == "tilesetPath":
                self.tilesetPath = attr[1]
            elif attr[0] == "tilesetDialoguePath":
                self.tilesetDialoguePath = attr[1]
            elif attr[0] == "animTilesPath":
                self.animTilesPath = attr[1]
            elif attr[0] == "animTiles_JSON_DialoguePath":
                self.animTilesTXTDialoguePath = attr[1]
            elif attr[0] == "animTiles_BIN_DialoguePath":
                self.animTilesBINDialoguePath = attr[1]
            elif attr[0] == "randTilesPath":
                self.randTilesPath = attr[1]
            elif attr[0] == "randTiles_JSON_DialoguePath":
                self.randTilesXMLDialoguePath = attr[1]
            elif attr[0] == "randTiles_BIN_DialoguePath":
                self.randTilesBINDialoguePath = attr[1]
            elif attr[0] == "geometry":
                self.restoreGeometry(QtCore.QByteArray.fromHex(bytes(attr[1], "ascii")))
            elif attr[0] == "windowState":
                self.restoreState(QtCore.QByteArray.fromHex(bytes(attr[1], "ascii")))
            elif attr[0] == "isDarkMode":
                self.isDarkMode = attr[1] == "True"
            elif attr[0] == "isSplitWindow":
                self.isSplitWindow = attr[1] == "True"


    def saveIni(self):
        self.tilesetPath = self.tilesetPathBox.text()
        self.tilesetDialoguePath = self.tilesetDialoguePathBox.text()

        self.animTiles_JSON_DialoguePath = self.animTiles_JSON_DialoguePathBox.text()
        self.animTiles_BIN_DialoguePath = self.animTiles_BIN_DialoguePathBox.text()

        self.randTiles_JSON_DialoguePath = self.randTiles_JSON_DialoguePathBox.text()
        self.randTiles_BIN_DialoguePath = self.randTiles_BIN_DialoguePathBox.text()

        self.isDarkMode = self.isDarkModeCheckBox.isChecked()
        self.SplitWindow = self.isSplitWindowCheckBox.isChecked()

        settingsText = ""
        settingsText += "tilesetPath=" + self.tilesetPath
        settingsText += "\ntilesetDialoguePath=" + self.tilesetDialoguePath
        settingsText += "\nanimTiles_JSON_DialoguePath=" + self.animTiles_JSON_DialoguePath
        settingsText += "\nanimTiles_BIN_DialoguePath=" + self.animTiles_BIN_DialoguePath
        settingsText += "\nrandTiles_JSON_DialoguePath=" + self.randTiles_JSON_DialoguePath
        settingsText += "\nrandTiles_BIN_DialoguePath=" + self.randTiles_BIN_DialoguePath
        settingsText += "\ngeometry=" + bytes(self.saveGeometry().toHex()).decode('ascii')
        settingsText += "\nwindowState=" + bytes(self.saveState().toHex()).decode('ascii')
        settingsText += "\nisDarkMode=" + str(self.isDarkMode)
        settingsText += "\nisSplitWindow=" + str(self.SplitWindow)

        f = open("Other/settings.ini", 'w')
        f.write(settingsText)
        self.settingsWindow.hide()
        
        self.setColorMode()


    def setColorMode(self):
        if self.isDarkMode:
            with open("dark.qss", 'r') as file:
                qss = file.read()
            app.setStyleSheet(qss)
        else:
            with open("light.qss", 'r') as file:
                qss = file.read()
            app.setStyleSheet(qss)


    def closeEvent(self, event):
        self.saveIni()
        QtWidgets.QMainWindow.closeEvent(self, event)


    def settingsInit(self):
        self.settingsWindow = QtWidgets.QWidget()
        self.description = QtWidgets.QLabel('Settings')
        font = self.description.font()
        font.setPointSize(18)
        font.setBold(True)
        self.description.setFont(font)

        self.tilesetDescription = QtWidgets.QLabel('Tilesets')
        self.tilesetPathBox = QtWidgets.QLineEdit(self.tilesetPath)
        self.tilesetPathBox.setPlaceholderText('Open this tileset on start ...')
        self.tilesetPathOpen = QtWidgets.QPushButton('Select')

        self.tilesetDialoguePathBox = QtWidgets.QLineEdit(self.tilesetDialoguePath)
        self.tilesetDialoguePathBox.setPlaceholderText('Start in this directory when opening/saving a tileset...')
        self.tilesetDialoguePathOpen = QtWidgets.QPushButton('Select')

        self.animTilesDescription = QtWidgets.QLabel('AnimTiles')

        self.animTiles_JSON_DialoguePathBox = QtWidgets.QLineEdit(self.animTiles_JSON_DialoguePath)
        self.animTiles_JSON_DialoguePathBox.setPlaceholderText('Start in this directory when importing/exporting an AnimTiles JSON...')
        self.animTiles_JSON_DialoguePathOpen = QtWidgets.QPushButton('Select')

        self.animTiles_BIN_DialoguePathBox = QtWidgets.QLineEdit(self.animTiles_BIN_DialoguePath)
        self.animTiles_BIN_DialoguePathBox.setPlaceholderText('Start in this directory when importing/exporting an AnimTiles binary...')
        self.animTiles_BIN_DialoguePathOpen = QtWidgets.QPushButton('Select')

        self.randTilesDescription = QtWidgets.QLabel('RandTiles')

        self.randTiles_JSON_DialoguePathBox = QtWidgets.QLineEdit(self.randTiles_JSON_DialoguePath)
        self.randTiles_JSON_DialoguePathBox.setPlaceholderText('Start in this directory when opening/saving a importing/exporting a RandTiles JSON...')
        self.randTiles_JSON_DialoguePathOpen = QtWidgets.QPushButton('Select')

        self.randTiles_BIN_DialoguePathBox = QtWidgets.QLineEdit(self.randTiles_BIN_DialoguePath)
        self.randTiles_BIN_DialoguePathBox.setPlaceholderText('Start in this directory when importing/exporting a RandTiles binary...')
        self.randTiles_BIN_DialoguePathOpen = QtWidgets.QPushButton('Select')

        self.isDarkModeCheckBox = QtWidgets.QCheckBox("Dark Mode")
        self.isDarkModeCheckBox.setChecked(self.isDarkMode)

        self.isSplitWindowCheckBox = QtWidgets.QCheckBox("Split Window")
        self.isSplitWindowCheckBox.setChecked(self.isSplitWindow)

        self.saveSettings = QtWidgets.QPushButton('Save')

        layout = QtWidgets.QGridLayout()
        layout.addWidget(self.description, 0, 0, 1, 4, Qt.AlignLeft | Qt.AlignTop)
        layout.addWidget(self.tilesetDescription, 1, 0, 1, 4, Qt.AlignCenter)
        layout.addWidget(self.tilesetPathBox, 2, 0, 1, 3)
        layout.addWidget(self.tilesetPathOpen, 2, 3, 1, 1)
        layout.addWidget(self.tilesetDialoguePathBox, 3, 0, 1, 3)
        layout.addWidget(self.tilesetDialoguePathOpen, 3, 3, 1, 1)
        layout.addWidget(self.animTilesDescription, 4, 0, 1, 4, Qt.AlignCenter)
        layout.addWidget(self.animTiles_JSON_DialoguePathBox, 5, 0, 1, 3)
        layout.addWidget(self.animTiles_JSON_DialoguePathOpen, 5, 3, 1, 1)
        layout.addWidget(self.animTiles_BIN_DialoguePathBox, 6, 0, 1, 3)
        layout.addWidget(self.animTiles_BIN_DialoguePathOpen, 6, 3, 1, 1)
        layout.addWidget(self.randTilesDescription, 7, 0, 1, 4, Qt.AlignCenter)
        layout.addWidget(self.randTiles_JSON_DialoguePathBox, 8, 0, 1, 3)
        layout.addWidget(self.randTiles_JSON_DialoguePathOpen, 8, 3, 1, 1)
        layout.addWidget(self.randTiles_BIN_DialoguePathBox, 9, 0, 1, 3)
        layout.addWidget(self.randTiles_BIN_DialoguePathOpen, 9, 3, 1, 1)
        layout.addWidget(self.isDarkModeCheckBox, 10, 0, 1, 2)
        layout.addWidget(self.isSplitWindowCheckBox, 10, 1, 1, 1)
        layout.addWidget(self.saveSettings, 10, 3, 1, 1)
        self.settingsWindow.setLayout(layout)

        self.tilesetPathOpen.released.connect(self.getTilesetPath)
        self.tilesetDialoguePathOpen.released.connect(self.getTilesetDialoguePath)
        self.animTiles_JSON_DialoguePathOpen.released.connect(self.getTXTAnimTilesPathOpen)
        self.animTiles_BIN_DialoguePathOpen.released.connect(self.getBINAnimTilesPathOpen)
        self.randTiles_JSON_DialoguePathOpen.released.connect(self.getXMLRandTilesPathOpen)
        self.randTiles_BIN_DialoguePathOpen.released.connect(self.getBINRandTilesPathOpen)
        self.saveSettings.released.connect(self.saveIni)

        self.settingsWindow.setMinimumWidth(900)
        self.settingsWindow.setWindowFlag(Qt.WindowMinimizeButtonHint, False)
        self.settingsWindow.setWindowFlag(Qt.WindowMaximizeButtonHint, False)
        self.settingsWindow.setWindowTitle('Settings')

    def settings(self):
        self.settingsWindow.show()

    def getTilesetPath(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Choose a file ...", '', "Tileset (*.arc)")[0]
        if not path: return
        self.tilesetPathBox.setText(path)

    def getTilesetDialoguePath(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder ...')
        if not path: return
        self.tilesetDialoguePathBox.setText(path)

    def getAnimTilesPath(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Choose a file ...", '', "AnimTiles.txt (*.txt);;AnimTiles.bin (*.bin)")[0]
        if not path: return
        self.animTilesPathBox.setText(path)

    def getTXTAnimTilesPathOpen(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder ...')
        if not path: return
        self.animTilesTXTDialoguePathBox.setText(path)

    def getBINAnimTilesPathOpen(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder ...')
        if not path: return
        self.animTilesBINDialoguePathBox.setText(path)

    def getRandTilesPath(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Choose a file ...", '', "tilesetinfo.xml (*.xml);;RandTiles.bin (*.bin)")[0]
        if not path: return
        self.randTilesPathBox.setText(path)

    def getXMLRandTilesPathOpen(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder ...')
        if not path: return
        self.randTilesXMLDialoguePathBox.setText(path)

    def getBINRandTilesPathOpen(self):
        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder ...')
        if not path: return
        self.randTilesBINDialoguePathBox.setText(path)


    def exportAllFramesheetsAsTpl(self):
        animationKeys = list(Tileset.animdata.keys())

        if len(animationKeys) == 0:
            QtWidgets.QMessageBox.warning(self, "Export framesheets",
                    "There are no opened framesheets to be exported!",
                    QtWidgets.QMessageBox.Cancel)
            return

        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder for the export ...')
        if not path: return

        for anim in animationKeys:
            #get height -> 2 bytes per pixel and 32 pixel width
            height = '%0*X' % (4, len(Tileset.animdata[anim])//64)
            header = bytearray.fromhex(f"0020AF30000000010000000C0000001400000000{height}002000000005000000400000000000000000000000010000000100000000000000000000000000000000")
            outdata = header + Tileset.animdata[anim]

            if outdata is not None:
                f = open(f"{path}/{anim[7:-4]}.tpl", 'wb')
                f.write(outdata)


    def exportAllFramesheetsAsPng(self):
        if self.framesheetmodel.rowCount() == 0:
            QtWidgets.QMessageBox.warning(self, "Export framesheets",
                    "There are no opened framesheets to be exported!",
                    QtWidgets.QMessageBox.Cancel)
            return

        path = QtWidgets.QFileDialog.getExistingDirectory(self, 'Choose a folder for the export ...')
        if not path: return

        i = 0
        while i < self.framesheetmodel.rowCount():
            item = self.framesheetmodel.itemFromIndex(self.framesheetmodel.index(i, 0))
            icon = item.icon()
            name = item.text()
            pixmap = icon.pixmap(icon.availableSizes()[0])
            pixmap.save("{}/{}.png".format(path, name), "PNG")
            i += 1


    def createClampedFramesheet(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Open unclamped framesheet", '', "Unclamped Framesheet (*.png)")[0]

        if path:
            framesheet = QtGui.QPixmap()
            if not framesheet.load(path):
                QtWidgets.QMessageBox.warning(self, "Open framesheet", "The framesheet file could not be loaded.", QtWidgets.QMessageBox.Cancel)
                return

            if framesheet.height() % framesheet.width() != 0:
                QtWidgets.QMessageBox.information(self, "Warning!",
                        "There seem to be some pixels missing here.\n"
                        "Make sure that the height of your framesheet is a multiple of its' width!")

            downScaledFramesheet = framesheet.scaledToWidth(24)
            downScaledImage = downScaledFramesheet.toImage().convertToFormat(QtGui.QImage.Format_ARGB32)

            image = QtGui.QImage(32, downScaledFramesheet.height() // 24 * 32, QtGui.QImage.Format_ARGB32)
            image.fill(Qt.transparent)

            i = 0
            while i < downScaledImage.height() // 24:
                y = 0
                while y < 24:
                    x = 0
                    while x < 24:
                        color = downScaledImage.pixel(x, y + i*24)
                        image.setPixel(x + 4, i * 32 + y + 4, color)
                        x += 1
                    y += 1

                y = 0                                                   #texture clamp top
                while y < 4:
                    x = 0
                    while x < 24:
                        color = image.pixel(x + 4, i * 32 + 4)
                        image.setPixel(x + 4, i * 32 + y, color)
                        x += 1
                    y += 1

                y = 0                                                   #texture clamp bottom
                while y < 4:
                    x = 0
                    while x < 24:
                        color = image.pixel(x + 4, i * 32 + 27)
                        image.setPixel(x + 4, i * 32 + y + 28, color)
                        x += 1
                    y += 1

                x = 0                                                   #texture clamp left
                while x < 4:
                    y = 0
                    while y < 24:
                        color = image.pixel(4, i * 32 + y + 4)
                        image.setPixel(x, i * 32 + y + 4, color)
                        y += 1
                    x += 1

                x = 0                                                   #texture clamp right
                while x < 4:
                    y = 0
                    while y < 24:
                        color = image.pixel(27, i * 32 + y + 4)
                        image.setPixel(x + 28, i * 32 + y + 4, color)
                        y += 1
                    x += 1

                i += 1

            fn = QtWidgets.QFileDialog.getSaveFileName(self, 'Save the clamped framesheet', '', 'Clamped Framesheet (*.png)')[0]
            if not fn: return

            image.save(fn)

    '''
    def convertGifToFramesheets(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Open a gif file", '', "Animated Image (*.gif)")[0]
        im = Image.open(path)
        try:
            shutil.rmtree("temp")
            os.mkdir("temp")
        except:
            try:
                os.mkdir("temp")
            except:
                print("Couldn't initialize temp directory!")
                return

        for i in range(0, im.n_frames):
            im.seek(i)
            print(i)
            im.save("temp/{}.png".format(i))

        self.popup = QtWidgets.QWidget()
        layout = QtWidgets.QGridLayout()
        self.btn1 = QtWidgets.QPushButton("Import from AnimTiles tab")
        #self.btn1.released.connect(self.fromAnimTiles)


        self.info = QtWidgets.QLabel("Importing from the AnimTiles tab imports all entries with a matching texname.\n\nIt removes those entries from the AnimTiles tab!\n\nSo: don't forget to export after you finished editing!!!")
        self.info.setWordWrap(True)

        layout.addWidget(self.btn1, 0, 0, 1, 1)
        layout.addWidget(self.info, 1, 0, 1, 2)

        self.popup.setLayout(layout)
        self.popup.setWindowTitle("Import framesheet info")
        self.popup.setWindowFlag(Qt.WindowMinimizeButtonHint, False)
        self.popup.setWindowFlag(Qt.WindowMaximizeButtonHint, False)
        self.popup.show()
    '''


    def help(self):
        class QHelpDialog(QtWidgets.QDialog):
            def __init__(self):
                QtWidgets.QDialog.__init__(self)
                self.setWindowTitle('Help')

                try:
                    with open('Other/help.html', 'r') as f:
                        help = f.read()
                except:
                    help = "The file 'help.html' is missing!"

                logo = QtGui.QPixmap('Icons/about.png')
                logoLabel = QtWidgets.QLabel()
                logoLabel.setPixmap(logo)
                logoLabel.setContentsMargins(16, 4, 32, 4)

                helpView = QtWidgets.QTextBrowser()
                helpView.setMinimumWidth(750)
                helpView.setMinimumHeight(500)
                helpView.setHtml(help)
                helpView.setOpenExternalLinks(True)

                buttonBox = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok)
                buttonBox.accepted.connect(self.accept)

                L = QtWidgets.QGridLayout()
                L.addWidget(logoLabel, 0, 0, 3, 1)
                L.addWidget(helpView, 0, 1, 2, 1)
                L.addWidget(buttonBox, 2, 0, 1, 2)
                L.setRowStretch(1, 1)
                L.setColumnStretch(1, 1)
                self.setLayout(L)

        QHelpDialog().exec_()

    def dispTutorial(self, type):
        class QTutorialDialog(QtWidgets.QDialog):
            def __init__(self):
                QtWidgets.QDialog.__init__(self)
                self.setWindowTitle('Tutorial Window')

                files = [
                    'Other/Tutorials/Animations.md',
                    'Other/Tutorials/Randomization.md',
                    'Other/Tutorials/Flower-Grass.md',
                    'Other/Tutorials/Profile-Overrides.md',
                ]

                try:
                    with open(files[type], 'r') as f:
                        contents = f.read()
                except:
                    contents = "The file '{0}' is missing!".format(files[type])

                helpView = QtWidgets.QTextBrowser()
                helpView.setMinimumWidth(750)
                helpView.setMinimumHeight(500)
                helpView.setMarkdown(contents)
                helpView.setOpenExternalLinks(True)

                buttonBox = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok)
                buttonBox.accepted.connect(self.accept)

                L = QtWidgets.QGridLayout()
                L.addWidget(helpView, 0, 0, 2, 1)
                L.addWidget(buttonBox, 2, 0, 1, 2)
                L.setRowStretch(1, 1)
                L.setColumnStretch(1, 1)
                self.setLayout(L)

        QTutorialDialog().exec_()

    def dispTutorialAnim(self):
        self.dispTutorial(0)

    def dispTutorialRand(self):
        self.dispTutorial(1)

    def dispTutorialGrass(self):
        self.dispTutorial(2)

    def dispTutorialProfile(self):
        self.dispTutorial(3)

    def setuptile(self):
        self.tileWidget.tiles.clear()
        self.model.clear()

        if self.alpha == True:
            for tile in Tileset.tiles:
                self.model.addPieces(QtGui.QPixmap.fromImage(tile.image))
        else:
            for tile in Tileset.tiles:
                self.model.addPieces(QtGui.QPixmap.fromImage(tile.noalpha))


    def newTileset(self):
        '''Creates a new, blank tileset'''

        global Tileset
        Tileset.clear()
        Tileset = TilesetClass()

        EmptyImg = QtGui.QImage(24, 24, QtGui.QImage.Format.Format_ARGB32)
        EmptyImg.fill(Qt.black)

        for i in range(256):
            Tileset.addTile(EmptyImg, EmptyImg)

        self.tileWidget.tilesetType.setText('Type: Pa0')

        self.setuptile()
        self.clearObjects()
        self.clearOverrides()

        self.setWindowTitle('New Tileset')


    def openTileset(self):
        '''Asks the user for a filename, then calls openTilesetFromPath().'''

        path = QtWidgets.QFileDialog.getOpenFileName(self, "Open NSMBW Tileset", window.tilesetDialoguePath, "Tileset Files (*.arc)")[0]

        if path:
            self.openTilesetFromPath(path)


    def openTilesetFromPath(self, path, suppressSlotWarning=False):
        '''Opens a Nintendo tileset arc and parses the heck out of it.'''
        if path in self.recentFiles:
            self.recentFiles.insert(0, self.recentFiles.pop(self.recentFiles.index(path)))
        else:
            self.recentFiles.insert(0, path)
            while len(self.recentFiles) > MainWindow.MaxRecentFiles:
                del self.recentFiles[9]
        self.saveRececntFiles()
        self.updateRecentFileActions()

        self.setWindowTitle(os.path.basename(path))
        Tileset.clear()

        basename = os.path.basename(path[str(path).rfind('/')+1:-4])

        with open(path,'rb') as file:
            data = file.read()

        arc = archive.U8()
        arc._load(data)

        Image = None
        behaviourdata = None
        objstrings = None
        metadata = None

        Tileset.animOverrides = None
        Tileset.randOverrides = None
        Tileset.plantOverrides = None
        Tileset.profileOverrides = None

        Tileset.kusaRes = None
        Tileset.hanaRes = None

        for key, value in arc.files:
            if value is None:
                continue
            elif key.startswith('BG_tex/') and key.endswith('_tex.bin.LZ'):
                Image = arc[key]
            elif key.startswith('BG_tex/') and key.endswith('.bin'):
                Tileset.animdata[key] = arc[key]
            elif key.startswith('BG_chk/d_bgchk_') and key.endswith('.bin'):
                behaviourdata = arc[key]
            elif key.startswith('BG_unt/') and key.endswith('_hd.bin'):
                metadata = arc[key]
            elif key.startswith('BG_unt/') and key.endswith('.bin'):
                objstrings = arc[key]
            elif key == 'BG_anim/anim_overrides.bin':
                Tileset.animOverrides = arc[key]
            elif key == 'BG_rand/rand_overrides.bin':
                Tileset.randOverrides = arc[key]
            elif key == 'BG_grass/grass_overrides.bin':
                Tileset.plantOverrides = arc[key]
            elif key == 'BG_prof/prof_overrides.bin':
                Tileset.profileOverrides = arc[key]
            elif key == 'g3d/obj_kusa.brres':
                Tileset.kusaRes = arc[key]
            elif key == 'g3d/obj_hana.brres':
                Tileset.hanaRes = arc[key]
            else:
                Tileset.unknownFiles[key] = arc[key]
                print(f"Unknown File: {key}")

        self.animOverwriteEditor.load_from_bin(Tileset.animOverrides)
        self.randOverwriteEditor.load_from_bin(Tileset.randOverrides)
        self.plantOverwriteEditor.load_from_bin(Tileset.plantOverrides)
        self.profileOverwriteEditor.load_from_bin(Tileset.profileOverrides)

        self.plantOverwriteEditor.grass_load_from_bin(Tileset.kusaRes)
        self.plantOverwriteEditor.flower_load_from_bin(Tileset.hanaRes)

        if (Image is None) or (behaviourdata is None) or (objstrings is None) or (metadata is None):
            QtWidgets.QMessageBox.warning(None, 'Error',  'Error - the necessary files were not found.\n\nNot a valid tileset, sadly.')
            return

        # Stolen from Reggie! Loads the Image Data.
        if HaveNSMBLib:
            tiledata = nsmblib.decompress11LZS(Image)
            if hasattr(nsmblib, 'decodeTilesetNoPremultiplication'):
                argbdata = nsmblib.decodeTilesetNoPremultiplication(tiledata)
                tileImage = QtGui.QImage(argbdata, 1024, 256, QtGui.QImage.Format_ARGB32)
            else:
                tileImage = RGB4A3Decode(tiledata)

            if hasattr(nsmblib, 'decodeTilesetNoPremultiplicationNoAlpha'):
                rgbdata = nsmblib.decodeTilesetNoPremultiplicationNoAlpha(tiledata)
                noalphaImage = QtGui.QImage(rgbdata, 1024, 256, QtGui.QImage.Format_ARGB32)
            else:
                noalphaImage = RGB4A3Decode(tiledata, False)
        else:
            lz = lz77.LZS11()
            decomp = lz.Decompress11LZS(Image)
            tileImage = RGB4A3Decode(decomp)
            noalphaImage = RGB4A3Decode(decomp, False)

        # Loads Tile Behaviours

        behaviours = []
        for entry in range(256):
            behaviours.append(struct.unpack_from('>8B', behaviourdata, entry*8))


        # Makes us some nice Tile Classes!
        Xoffset = 4
        Yoffset = 4
        for i in range(256):
            Tileset.addTile(tileImage.copy(Xoffset,Yoffset,24,24), noalphaImage.copy(Xoffset,Yoffset,24,24), behaviours[i])
            Xoffset += 32
            if Xoffset >= 1024:
                Xoffset = 4
                Yoffset += 32


        # Load Objects

        meta = []
        for i in range(len(metadata) // 4):
            meta.append(struct.unpack_from('>H2B', metadata, i * 4))

        tilelist = [[]]
        upperslope = [0, 0]
        lowerslope = [0, 0]
        byte = 0

        for entry in meta:
            offset = entry[0]
            byte = struct.unpack_from('>B', objstrings, offset)[0]
            row = 0

            while byte != 0xFF:

                if byte == 0xFE:
                    tilelist.append([])

                    if (upperslope[0] != 0) and (lowerslope[0] == 0):
                        upperslope[1] = upperslope[1] + 1

                    if lowerslope[0] != 0:
                        lowerslope[1] = lowerslope[1] + 1

                    offset += 1
                    byte = struct.unpack_from('>B', objstrings, offset)[0]

                elif (byte & 0x80):

                    if upperslope[0] == 0:
                        upperslope[0] = byte
                    else:
                        lowerslope[0] = byte

                    offset += 1
                    byte = struct.unpack_from('>B', objstrings, offset)[0]

                else:
                    tilelist[-1].append(struct.unpack_from('>3B', objstrings, offset))

                    offset += 3
                    byte = struct.unpack_from('>B', objstrings, offset)[0]

            tilelist.pop()

            if (upperslope[0] & 0x80) and (upperslope[0] & 0x2):
                for i in range(lowerslope[1]):
                    pop = tilelist.pop()
                    tilelist.insert(0, pop)

            Tileset.addObject(entry[2], entry[1], upperslope, lowerslope, tilelist)

            tilelist = [[]]
            upperslope = [0, 0]
            lowerslope = [0, 0]

        if Tileset.objects:
            slots = []
            for object in Tileset.objects:
                for row in object.tiles:
                    for tile in row:
                        slot = tile[2] & 3
                        if slot != 0 or tile[1] != 0:
                            slots.append(slot)

            if not slots:
                Tileset.slot = 0
            else:
                Tileset.slot = max(slots, key=slots.count)
            del slots

            if basename[:4] in ('Pa0_', 'Pa1_', 'Pa2_', 'Pa3_'):
                slot = int(basename[2])
                if slot != Tileset.slot and not suppressSlotWarning:
                    QtWidgets.QMessageBox.information(self, "Warning", "Determined tileset slot ({}) does not match with the slot in the filename ({})!".format(Tileset.slot, slot))

            else:
                print("WARNING: Tileset name does not begin with \"PaX_\". Unable to verify tileset slot.")


        else:
            Tileset.slot = 1

        self.tileWidget.tilesetType.setText('Pa{0}'.format(Tileset.slot))

        self.setuptile()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        SetupFramesheetModel(self, Tileset.animdata)

        self.objectList.clearCurrentIndex()
        self.tileWidget.setObject(self.objectList.currentIndex())

        self.objectList.update()
        self.tileWidget.update()

        modelindex = self.framesheetList.currentIndex().sibling(-1, 0)
        self.framesheetList.setCurrentIndex(modelindex)

        self.name = path




    def openImage(self):
        '''Opens an Image from png, and creates a new tileset from it.'''

        path = QtWidgets.QFileDialog.getOpenFileName(self, "Open Image", '',
                    "Image Files (*.png)")[0]

        if not path: return

        tileImage = QtGui.QImage(path)
        if tileImage.isNull():
            QtWidgets.QMessageBox.warning(self, "Open Image",
                    "The image file could not be loaded.",
                    QtWidgets.QMessageBox.Cancel)
            return

        if tileImage.width() != 384 or tileImage.height() != 384:
            QtWidgets.QMessageBox.warning(self, "Open Image",
                    "The image was not the proper dimensions."
                    "Please resize the image to 384x384 pixels.",
                    QtWidgets.QMessageBox.Cancel)
            return

        if tileImage.format() != QtGui.QImage.Format.Format_ARGB32:
            tileImage.convertTo(QtGui.QImage.Format.Format_ARGB32)

        if not hasattr(self, 'extendEdges'):
            self.extendEdges = True
            self.skipExtendEdgesDialog = False
            isFirstTime = True
        else:
            isFirstTime = False

        tileImagesRaw = []
        tileImagesFixed = []
        tileImagesRawNoAlpha = []
        tileImagesFixedNoAlpha = []

        x = 0
        y = 0
        for i in range(256):
            img = tileImage.copy(x*24,y*24,24,24)

            bgra = bytearray(img.bits().asstring(24 * 24 * 4))

            # Skip this if it's not going to be used for anything
            if not self.skipExtendEdgesDialog or not self.extendEdges:
                bgra_bk = bytearray(bgra)

                tileImagesRaw.append(QtGui.QImage(bytes(bgra), 24, 24, QtGui.QImage.Format.Format_ARGB32))
                for offs in range(3, 24 * 24 * 4, 4):
                    bgra[offs] = 0xff
                tileImagesRawNoAlpha.append(QtGui.QImage(bytes(bgra), 24, 24, QtGui.QImage.Format.Format_ARGB32))

                bgra = bgra_bk

            # Ditto
            if not self.skipExtendEdgesDialog or self.extendEdges:
                color_transparent_pixels_around_edges_24_24(bgra)

                tileImagesFixed.append(QtGui.QImage(bytes(bgra), 24, 24, QtGui.QImage.Format.Format_ARGB32))
                for offs in range(3, 24 * 24 * 4, 4):
                    bgra[offs] = 0xff
                tileImagesFixedNoAlpha.append(QtGui.QImage(bytes(bgra), 24, 24, QtGui.QImage.Format.Format_ARGB32))

            x += 1
            if x >= 16:
                y += 1
                x = 0

        # Show dialog if needed
        if not self.skipExtendEdgesDialog:
            fullRaw = QtGui.QImage(384, 384, QtGui.QImage.Format.Format_ARGB32)
            painterRaw = QtGui.QPainter(fullRaw)
            fullFixed = QtGui.QImage(384, 384, QtGui.QImage.Format.Format_ARGB32)
            painterFixed = QtGui.QPainter(fullFixed)

            Xoffset = Yoffset = 0
            for raw, fixed in zip(tileImagesRawNoAlpha, tileImagesFixedNoAlpha):
                painterRaw.drawImage(Xoffset, Yoffset, raw)
                painterFixed.drawImage(Xoffset, Yoffset, fixed)
                Xoffset += 24
                if Xoffset >= 384:
                    Xoffset = 0
                    Yoffset += 24

            del painterRaw, painterFixed

            dlg = self.extendEdgesDialog(fullRaw, fullFixed, self.extendEdges, isFirstTime)

            if dlg.exec() == QtWidgets.QDialog.DialogCode.Accepted:
                self.extendEdges = dlg.doFix
                self.skipExtendEdgesDialog = dlg.rememberCheckbox.isChecked()
            else:
                return

        # Apply new tile images
        if self.extendEdges:
            for i in range(256):
                Tileset.tiles[i].image = tileImagesFixed[i]
                Tileset.tiles[i].noalpha = tileImagesFixedNoAlpha[i]
        else:
            for i in range(256):
                Tileset.tiles[i].image = tileImagesRaw[i]
                Tileset.tiles[i].noalpha = tileImagesRawNoAlpha[i]

        index = self.objectList.currentIndex()
        self.setuptile()
        self.objectList.clearCurrentIndex()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        self.objectList.setCurrentIndex(self.objmodel.index(index.row(), index.column()))
        self.tileWidget.setObject(index)
        self.objectList.update()
        self.tileWidget.update()


    class extendEdgesDialog(QtWidgets.QDialog):
        class linkedScrollArea(QtWidgets.QScrollArea):
            '''A QScrollArea that synchronizes its scrolling with another one (self.other)'''
            updating = False

            def __init__(self, parent=None):
                super().__init__(parent)
                self.horizontalScrollBar().valueChanged.connect(self.handleHorzScroll)
                self.verticalScrollBar().valueChanged.connect(self.handleVertScroll)

            def handleHorzScroll(self, value):
                if self.updating: return
                self.updating = True
                self.other.horizontalScrollBar().setValue(value)
                self.updating = False

            def handleVertScroll(self, value):
                if self.updating: return
                self.updating = True
                self.other.verticalScrollBar().setValue(value)
                self.updating = False

        def __init__(self, rawImage, fixedImage, defaultFixed, defaultRemember):
            super().__init__()

            self.setWindowTitle('Choose import options')

            self.textLabel = QtWidgets.QLabel(
                "When exporting, many image editing tools set the RGB values for transparent pixels to black or other arbitrary colors, since it's irrelevant in most cases."
                " However, with the Wii's texture filtering, this can result in faint dark outlines around tiles."
                "\n\nPuzzle can try to automatically correct for this, or you can keep the existing colors in your image."
                '\n\nYou should choose "Fix colors of transparent pixels" in most cases unless you know what you\'re doing.')
            self.textLabel.setWordWrap(True)

            SCALE = 5

            self.rawLabel = QtWidgets.QLabel(self)
            self.rawLabel.setPixmap(QtGui.QPixmap.fromImage(rawImage).scaledToWidth(384 * SCALE))
            self.fixedLabel = QtWidgets.QLabel(self)
            self.fixedLabel.setPixmap(QtGui.QPixmap.fromImage(fixedImage).scaledToWidth(384 * SCALE))

            self.rawScrollArea = self.linkedScrollArea(self)
            self.rawScrollArea.setWidget(self.rawLabel)
            self.fixedScrollArea = self.linkedScrollArea(self)
            self.fixedScrollArea.setWidget(self.fixedLabel)
            self.rawScrollArea.other = self.fixedScrollArea
            self.fixedScrollArea.other = self.rawScrollArea

            self.rawTitleLabel = QtWidgets.QLabel('<b>Original image without alpha channel:</b>')
            self.fixedTitleLabel = QtWidgets.QLabel('<b>Fixed image without alpha channel:</b>')

            self.rememberCheckbox = QtWidgets.QCheckBox("Don't ask again for this session")
            self.rememberCheckbox.setChecked(defaultRemember)

            self.buttons = QtWidgets.QDialogButtonBox()
            self.buttons.addButton('Preserve colors of transparent pixels', QtWidgets.QDialogButtonBox.ButtonRole.AcceptRole)
            self.fixedButton = QtWidgets.QPushButton('Fix colors of transparent pixels', self)
            self.fixedButton.setDefault(True)
            self.buttons.addButton(self.fixedButton, QtWidgets.QDialogButtonBox.ButtonRole.AcceptRole)
            self.buttons.addButton(QtWidgets.QDialogButtonBox.StandardButton.Cancel)
            self.buttons.accepted.connect(self.accept)
            self.buttons.rejected.connect(self.reject)
            self.buttons.clicked.connect(self.handleClicked)

            self.layout = QtWidgets.QGridLayout()
            self.layout.addWidget(self.textLabel, 0, 0, 1, 2)
            self.layout.setRowMinimumHeight(1, 32)
            self.layout.addWidget(self.rawTitleLabel, 2, 0, 1, 1, Qt.AlignmentFlag.AlignCenter)
            self.layout.addWidget(self.fixedTitleLabel, 2, 1, 1, 1, Qt.AlignmentFlag.AlignCenter)
            self.layout.addWidget(self.rawScrollArea, 3, 0)
            self.layout.addWidget(self.fixedScrollArea, 3, 1)
            self.layout.setRowMinimumHeight(4, 32)
            self.layout.addWidget(self.rememberCheckbox, 5, 0, 1, 2, Qt.AlignmentFlag.AlignRight)
            self.layout.addWidget(self.buttons, 6, 0, 1, 2)
            self.setLayout(self.layout)

        def handleClicked(self, button):
            self.doFix = (button is self.fixedButton)


    def saveImage(self, fn = None):
        if fn is None:
            fn = QtWidgets.QFileDialog.getSaveFileName(self, 'Choose a new filename', '', '.png (*.png)')[0]
        if not fn: return

        tex = QtGui.QImage(384, 384, QtGui.QImage.Format.Format_ARGB32)
        tex.fill(Qt.transparent)
        painter = QtGui.QPainter(tex)

        Xoffset = 0
        Yoffset = 0

        for tile in Tileset.tiles:
            painter.drawImage(Xoffset, Yoffset, tile.image)
            Xoffset += 24
            if Xoffset >= 384:
                Xoffset = 0
                Yoffset += 24

        painter.end()

        tex.save(fn)


    def saveTileset(self):
        if not self.name:
            self.saveTilesetAs()
            return

        outdata = self.saving(os.path.basename(self.name)[:-4])

        if outdata is not None:
            fn = self.name
            with open(fn, 'wb') as f:
                f.write(outdata)


    def saveTilesetAs(self):

        fn = QtWidgets.QFileDialog.getSaveFileName(self, 'Choose a new filename', window.tilesetDialoguePath, '.arc (*.arc)')[0]
        if not fn: return

        outdata = self.saving(os.path.basename(str(fn))[:-4])

        if outdata is not None:
            self.name = fn
            self.setWindowTitle(os.path.basename(str(fn)))

            with open(fn, 'wb') as f:
                f.write(outdata)


    def saving(self, name):

        # Prepare tiles, objects, object metadata, and textures and stuff into buffers.

        textureBuffer = self.PackTexture()

        if textureBuffer is None:
            # The user canceled the saving process in the "use nsmblib?" dialog
            return None

        tileBuffer = self.PackTiles()
        objectBuffers = self.PackObjects()
        objectBuffer = objectBuffers[0]
        objectMetaBuffer = objectBuffers[1]


        # Make an arc and pack up the files!

        # NOTE: adding the files/folders to the U8 object in
        # alphabetical order is a simple workaround for a wii.py... bug?
        # unintuitive quirk? Whatever. Fixes one of the issues listed
        # in GitHub issue #3 (in the RoadrunnerWMC/Puzzle-Updated repo)

        arcFiles = {}
        arcFiles['BG_tex'] = None
        arcFiles['BG_tex/{0}_tex.bin.LZ'.format(name)] = textureBuffer

        arcFiles['BG_chk'] = None
        arcFiles['BG_chk/d_bgchk_{0}.bin'.format(name)] = tileBuffer

        arcFiles['BG_unt'] = None
        arcFiles['BG_unt/{0}.bin'.format(name)] = objectBuffer
        arcFiles['BG_unt/{0}_hd.bin'.format(name)] = objectMetaBuffer

        self.sortedAnimdata = sorted(Tileset.animdata.items())
        arcFiles.update(self.sortedAnimdata)

        grassData = self.plantOverwriteEditor.to_bytes()
        if grassData is not None:
            arcFiles['BG_grass'] = None
            arcFiles['BG_grass/grass_overrides.bin'] = self.plantOverwriteEditor.to_bytes()

        arcFiles['g3d'] = None
        grassResData = self.plantOverwriteEditor.grass_to_bytes()
        if grassResData is not None:
            arcFiles['g3d/obj_kusa.brres'] = self.plantOverwriteEditor.grass_to_bytes()

        flowerResData = self.plantOverwriteEditor.flower_to_bytes()
        if flowerResData is not None:
            arcFiles['g3d/obj_hana.brres'] = self.plantOverwriteEditor.flower_to_bytes()

        profileData = self.profileOverwriteEditor.to_bytes()
        if profileData is not None:
            arcFiles['BG_prof'] = None
            arcFiles['BG_prof/profile_overrides.bin'] = self.profileOverwriteEditor.to_bytes()
        
        randData = self.randOverwriteEditor.to_bytes()
        if randData is not None:
            print('Saving rand_overrides.bin')
            arcFiles['BG_rand'] = None
            arcFiles['BG_rand/rand_overrides.bin'] = self.randOverwriteEditor.to_bytes()

        animData = self.animOverwriteEditor.to_bytes()
        if animData is not None:
            print('Saving anim_overrides.bin')
            arcFiles['BG_anim'] = None
            arcFiles['BG_anim/anim_overrides.bin'] = self.animOverwriteEditor.to_bytes()

        for key, file in Tileset.unknownFiles.items():
            arcFiles[key.split("/")[0]] = None
            arcFiles[key] = file

        arc = archive.U8()
        for name in sorted(arcFiles):
            arc[name] = arcFiles[name]
        return arc._dump()


    def PackTexture(self):

        tex = bytearray(1024 * 256 * 4)
        stride = 1024 * 4

        for i, tile in enumerate(Tileset.tiles):
            tile_bytes = bytearray(tile.image.bits().asstring(24 * 24 * 4))

            row, col = divmod(i, 32)
            dest_offs = row * 32 * stride + col * (32 * 4)

            for src_y in range(24):
                row = tile_bytes[src_y * (24 * 4) : (src_y + 1) * (24 * 4)]

                # Clamp left/right pixels of the row
                row = row[:4] * 4 + row + row[-4:] * 4

                tex[dest_offs : dest_offs + (32 * 4)] = row
                dest_offs += stride
                if src_y == 0 or src_y == 23:
                    # Clamp top/bottom rows of the tile
                    tex[dest_offs : dest_offs + (32 * 4)] = row
                    dest_offs += stride
                    tex[dest_offs : dest_offs + (32 * 4)] = row
                    dest_offs += stride
                    tex[dest_offs : dest_offs + (32 * 4)] = row
                    dest_offs += stride
                    tex[dest_offs : dest_offs + (32 * 4)] = row
                    dest_offs += stride

        tex = bytes(tex)

        tex = RGB4A3Encode(tex)

        useNSMBLib = HaveNSMBLib and hasattr(nsmblib, 'compress11LZS')

        if useNSMBLib:
            # There are two versions of nsmblib floating around: the original
            # one, where the compression doesn't work correctly, and a fixed one
            # with correct compression.
            # We're going to show a warning to the user if they have the broken one installed.
            # To detect which one it is, we use the following test data:
            COMPRESSION_TEST = b'\0\1\0\0\0\0\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0'

            # The original broken algorithm compresses that incorrectly.
            # So let's compress it, and then decompress it, and see if we
            # got the right output.
            compressionWorking = (nsmblib.decompress11LZS(nsmblib.compress11LZS(COMPRESSION_TEST)) == COMPRESSION_TEST)

            if not compressionWorking:
                # NSMBLib is available, but only with the broken compression algorithm,
                # so the user can choose whether to use it or not

                items = ['Slow compression, good quality', 'Fast compression, but the image gets damaged']

                item, ok = QtWidgets.QInputDialog.getItem(self, 'Choose compression method',
                    'Two methods of compression are available. Choose<br />'
                    '"fast compression" for rapid testing. Choose "slow<br />'
                    'compression" for releases.<br />'
                    '<br />'
                    'To fix the fast compression, download and install<br />'
                    'NSMBLib-Updated:<br />'
                    '"pip install --force-reinstall --no-cache-dir nsmblib"',
                    items, 0, False)
                if not ok:
                    return None

                if item == "Slow compression, good quality":
                    useNSMBLib = False
                else:
                    useNSMBLib = True

        else:
            # NSMBLib is not available, so we have to use the Python version
            useNSMBLib = False

        if useNSMBLib:
            return nsmblib.compress11LZS(bytes(tex))
        else:
            return lz77.LZS11().Compress11LZS(tex)


    def PackTiles(self):
        offset = 0
        tilespack = struct.Struct('>8B')
        Tilebuffer = create_string_buffer(2048)
        for tile in Tileset.tiles:
            tilespack.pack_into(Tilebuffer, offset, tile.byte0, tile.byte1, tile.byte2, tile.byte3, tile.byte4, tile.byte5, tile.byte6, tile.byte7)
            offset += 8

        return Tilebuffer.raw


    def PackObjects(self):
        objectStrings = []

        o = 0
        for object in Tileset.objects:


            # Slopes
            if object.upperslope[0] != 0:

                # Reverse Slopes
                if object.upperslope[0] & 0x2:
                    a = struct.pack('>B', object.upperslope[0])

                    for row in range(object.lowerslope[1], object.height):
                        for tile in object.tiles[row]:
                            a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                        a += b'\xfe'

                    if object.height > 1 and object.lowerslope[1]:
                        a += struct.pack('>B', object.lowerslope[0])

                        for row in range(0, object.lowerslope[1]):
                            for tile in object.tiles[row]:
                                a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                            a += b'\xfe'

                    a += b'\xff'

                    objectStrings.append(a)


                # Regular Slopes
                else:
                    a = struct.pack('>B', object.upperslope[0])

                    for row in range(0, object.upperslope[1]):
                        try:
                            for tile in object.tiles[row]:
                                a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                            a += b'\xfe'
                        except:
                            print(row)
                            print(len(object.tiles))
                            continue
                    if object.height > 1 and object.lowerslope[1]:
                        a += struct.pack('>B', object.lowerslope[0])

                        for row in range(object.upperslope[1], object.height):
                            for tile in object.tiles[row]:
                                a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                            a += b'\xfe'

                    a += b'\xff'

                    objectStrings.append(a)


            # Not slopes!
            else:
                a = b''

                for tilerow in object.tiles:
                    for tile in tilerow:
                        a += struct.pack('>BBB', tile[0], tile[1], tile[2])

                    a += b'\xfe'

                a += b'\xff'

                objectStrings.append(a)

            o += 1

        Objbuffer = b''
        Metabuffer = b''
        i = 0
        for a in objectStrings:
            Metabuffer += struct.pack('>H2B', len(Objbuffer), Tileset.objects[i].width, Tileset.objects[i].height)
            Objbuffer += a

            i += 1

        return (Objbuffer, Metabuffer)


    def setupMenus(self):
        def get(name):
            """
            Returns an icon
            """
            try:
                return QtGui.QIcon('MenuIcons/icon-' + name + '.png')
            except:
                return None

        fileMenu = self.menuBar().addMenu("&File")

        self.action = fileMenu.addAction(get('new'), "New", self.newTileset, QtGui.QKeySequence('Ctrl+N'))
        fileMenu.addAction(get('open'), "Open Tileset", self.openTileset, QtGui.QKeySequence('Ctrl+O'))
        self.recentMenu = fileMenu.addMenu(get('open'), "Open recent Tileset")
        fileMenu.addAction(get('import'), "Import Image", self.openImage, QtGui.QKeySequence('Ctrl+I'))
        fileMenu.addAction(get('export'), "Export Image", self.saveImage, QtGui.QKeySequence('Ctrl+E'))
        fileMenu.addAction(get('save'), "Save Tileset", self.saveTileset, QtGui.QKeySequence('Ctrl+S'))
        fileMenu.addAction(get('saveas'), "Save Tileset as", self.saveTilesetAs, QtGui.QKeySequence('Ctrl+Shift+S'))
        fileMenu.addAction(get('exit'), "Quit", self.close, QtGui.QKeySequence('Ctrl+Q'))
        fileMenu.addAction(get('settings'), "Settings", self.settings, QtGui.QKeySequence('Ctrl+P'))

        fileMenu.addSeparator()
        nsmblibAct = fileMenu.addAction('Using NSMBLib' if HaveNSMBLib else 'Not using NSMBLib')
        nsmblibAct.setEnabled(False)

        taskMenu = self.menuBar().addMenu("&Tasks")

        taskMenu.addAction("Set Tileset Slot", self.setSlot, QtGui.QKeySequence('Ctrl+T'))
        taskMenu.addAction("Clear Collision Data", self.clearCollisions, QtGui.QKeySequence('Ctrl+Shift+Backspace'))
        taskMenu.addAction("Clear Object Data", self.clearObjects, QtGui.QKeySequence('Ctrl+Alt+Backspace'))
        taskMenu.addAction("Export all objects", self.saveAllObjects)
        taskMenu.addAction("Switch objects", self.switchObjects)

        animMenu = self.menuBar().addMenu("&Framesheets")
        animMenu.addAction("Export all framesheets as .tpl", self.exportAllFramesheetsAsTpl, QtGui.QKeySequence('Ctrl+F'))
        animMenu.addAction("Export all framesheets as .png", self.exportAllFramesheetsAsPng, QtGui.QKeySequence('Ctrl+Shift+F'))
        animMenu.addAction("Create clamped framesheet", self.createClampedFramesheet, QtGui.QKeySequence('Ctrl+Shift+C'))
        #animMenu.addAction("Convert gif to framesheet(s)", self.convertGifToFramesheets, QtGui.QKeySequence('Ctrl+G'))

        tutorialMenu = self.menuBar().addMenu("&Tutorials")
        tutorialMenu.addAction("AnimTiles", self.dispTutorialAnim)
        tutorialMenu.addAction("RandTiles", self.dispTutorialRand)
        tutorialMenu.addAction("Flower/Grass Overrides", self.dispTutorialGrass)
        tutorialMenu.addAction("Profile Overrides", self.dispTutorialProfile)

        otherMenu = self.menuBar().addMenu("&Help")
        otherMenu.addAction("Help", self.help)

        for i in range(MainWindow.MaxRecentFiles):
            self.recentFileActs.append(QtWidgets.QAction(self, visible=False, triggered=self.openRecentFile))

        for i in range(MainWindow.MaxRecentFiles):
            self.recentMenu.addAction(self.recentFileActs[i])


    def updateRecentFileActions(self):
        numRecentFiles = min(len(self.recentFiles), MainWindow.MaxRecentFiles)

        if numRecentFiles == 0:
            self.recentMenu.setEnabled(False)
        else:
            self.recentMenu.setEnabled(True)

        for i in range(numRecentFiles):
            text = "&%d %s" % (i + 1, self.strippedName(self.recentFiles[i]))
            self.recentFileActs[i].setText(text)
            self.recentFileActs[i].setData(self.recentFiles[i])
            self.recentFileActs[i].setVisible(True)

        for j in range(numRecentFiles, MainWindow.MaxRecentFiles):
            self.recentFileActs[j].setVisible(False)


    def openRecentFile(self):
        action = self.sender()
        if action:
            self.openTilesetFromPath(action.data())


    def strippedName(self, fullFileName):
        return QtCore.QFileInfo(fullFileName).fileName()


    def setSlot(self):
        global Tileset

        items = ("Pa0", "Pa1", "Pa2", "Pa3")

        item, ok = QtWidgets.QInputDialog.getItem(self, "Set Tileset Slot",
                "Warning: \n    Setting the tileset slot will override any \n    tiles set to draw from other tilesets.\n\nCurrent slot is Pa%d" % Tileset.slot, items, 0, False)
        if ok and item:
            Tileset.slot = int(item[2])
            self.tileWidget.tilesetType.setText(item)

            self.updateInfo(0, 0)

            cobj = 0
            crow = 0
            ctile = 0
            for object in Tileset.objects:
                for row in object.tiles:
                    for tile in row:
                        if (tile[2] & 3) != 0 or tile[1] != 0:
                            Tileset.objects[cobj].tiles[crow][ctile] = (tile[0], tile[1], (tile[2] & 0xFC) | int(str(item[2])))
                        ctile += 1
                    crow += 1
                    ctile = 0
                cobj += 1
                crow = 0
                ctile = 0


    def toggleAlpha(self):
        # Replace Alpha Image with non-Alpha images in model
        self.alpha = not self.alpha

        self.setuptile()

        self.tileWidget.setObject(self.objectList.currentIndex())
        self.tileWidget.update()


    def importObjFromFile(self):
        usedTiles = Tileset.getUsedTiles()
        if len(usedTiles) >= 256:  # It can't be more than 256, oh well
            QtWidgets.QMessageBox.warning(self, "Open Object",
                    "There isn't enough room in the Tileset.",
                    QtWidgets.QMessageBox.Cancel)
            return

        file = QtWidgets.QFileDialog.getOpenFileName(self, "Open Object", '',
                    "Object files (*.json)")[0]

        if not file: return

        with open(file) as inf:
            jsonData = json.load(inf)

        dir = os.path.dirname(file)

        tilelist = [[]]
        upperslope = [0, 0]
        lowerslope = [0, 0]

        metaData = open(dir + "/" + jsonData["meta"], "rb").read()
        objstrings = open(dir + "/" + jsonData["objlyt"], "rb").read()
        colls = open(dir + "/" + jsonData["colls"], "rb").read()

        tilesUsed = []

        pos = 0
        while objstrings[pos] != 0xFF:
            if objstrings[pos] & 0x80:
                pos += 1
                continue

            tile = objstrings[pos:pos+3]
            if tile != b'\0\0\0':
                if tile[1] not in tilesUsed:
                    tilesUsed.append(tile[1])

            pos += 3

        numTiles = len(tilesUsed)

        if numTiles + len(usedTiles) > 256:
            QtWidgets.QMessageBox.warning(self, "Open Object",
                    "There isn't enough room for the object.",
                    QtWidgets.QMessageBox.Cancel)
            return

        freeTiles = [i for i in range(256) if i not in usedTiles]

        tilesUsed = {}

        offset = 0
        byte = struct.unpack_from('>B', objstrings, offset)[0]
        i = 0
        row = 0

        while byte != 0xFF:

            if byte == 0xFE:
                tilelist.append([])

                if (upperslope[0] != 0) and (lowerslope[0] == 0):
                    upperslope[1] = upperslope[1] + 1

                if lowerslope[0] != 0:
                    lowerslope[1] = lowerslope[1] + 1

                offset += 1
                byte = struct.unpack_from('>B', objstrings, offset)[0]

            elif (byte & 0x80):

                if upperslope[0] == 0:
                    upperslope[0] = byte
                else:
                    lowerslope[0] = byte

                offset += 1
                byte = struct.unpack_from('>B', objstrings, offset)[0]

            else:
                tileBytes = objstrings[offset:offset + 3]
                if tileBytes == b'\0\0\0':
                    tile = [0, 0, 0]

                else:
                    tile = []
                    tile.append(byte)

                    if tileBytes[1] not in tilesUsed:
                        tilesUsed[tileBytes[1]] = i
                        tile.append(freeTiles[i])
                        i += 1
                    else:
                        tile.append(freeTiles[tilesUsed[tileBytes[1]]])

                    byte2 = (struct.unpack_from('>B', objstrings, offset + 2)[0]) & 0xFC
                    byte2 |= Tileset.slot
                    tile.append(byte2)

                tilelist[-1].append(tile)

                offset += 3
                byte = struct.unpack_from('>B', objstrings, offset)[0]

        tilelist.pop()

        if (upperslope[0] & 0x80) and (upperslope[0] & 0x2):
            for i in range(lowerslope[1]):
                pop = tilelist.pop()
                tilelist.insert(0, pop)

        Tileset.addObject(metaData[3], metaData[2], upperslope, lowerslope, tilelist)

        count = len(Tileset.objects)

        object = Tileset.objects[count-1]

        tileImage = QtGui.QPixmap(dir + "/" + jsonData["img"])

        tex = QtGui.QPixmap(object.width * 24, object.height * 24)
        tex.fill(Qt.transparent)
        painter = QtGui.QPainter(tex)

        Xoffset = 0
        Yoffset = 0

        colls_off = 0

        tilesReplaced = []

        for row in object.tiles:
            for tile in row:
                if tile[2] & 3 or not Tileset.slot:
                    if tile[1] not in tilesReplaced:
                        tilesReplaced.append(tile[1])

                        Tileset.tiles[tile[1]].image = tileImage.copy(Xoffset,Yoffset,24,24).toImage()
                        Tileset.tiles[tile[1]].byte0 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte1 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte2 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte3 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte4 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte5 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte6 = colls[colls_off]
                        colls_off += 1
                        Tileset.tiles[tile[1]].byte7 = colls[colls_off]
                        colls_off += 1

                    painter.drawImage(Xoffset, Yoffset, Tileset.tiles[tile[1]].image)
                Xoffset += 24
            Xoffset = 0
            Yoffset += 24

        painter.end()

        self.setuptile()

        self.objmodel.appendRow(QtGui.QStandardItem(QtGui.QIcon(tex), 'Object {0}'.format(count-1)))

        index = self.objectList.currentIndex()
        self.objectList.setCurrentIndex(index)
        self.objectList.setCurrentIndex(index)
        self.tileWidget.setObject(index)

        self.objectList.update()
        self.tileWidget.update()


    def moveObjectUp(self):
        if len(Tileset.objects) == 0: return

        if self.objectList.currentIndex().row()-1 < 0:
            return

        curr = Tileset.objects[self.objectList.currentIndex().row()]
        dest = Tileset.objects[self.objectList.currentIndex().row()-1]

        Tileset.objects[self.objectList.currentIndex().row()-1] = curr
        Tileset.objects[self.objectList.currentIndex().row()] = dest

        index = self.objectList.currentIndex()
        self.objectList.clearCurrentIndex()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        self.objectList.setCurrentIndex(self.objmodel.index(index.row()-1, index.column()))
        self.objectList.update()

    def moveObjectDown(self):
        if len(Tileset.objects) == 0: return

        if self.objectList.currentIndex().row()+1 >= len(Tileset.objects):
            return

        curr = Tileset.objects[self.objectList.currentIndex().row()]
        dest = Tileset.objects[self.objectList.currentIndex().row()+1]

        Tileset.objects[self.objectList.currentIndex().row()+1] = curr
        Tileset.objects[self.objectList.currentIndex().row()] = dest

        index = self.objectList.currentIndex()
        self.objectList.clearCurrentIndex()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        self.objectList.setCurrentIndex(self.objmodel.index(index.row()+1, index.column()))
        self.objectList.update()


    def switchObjects(self):
        self.switchObjectsWindow = QtWidgets.QWidget()
        self.object1label = QtWidgets.QLabel("1st Object:")
        self.object1spin = QtWidgets.QSpinBox()
        self.object1spin.setRange(0, len(Tileset.objects)-1)
        self.object2label = QtWidgets.QLabel("2nd Object:")
        self.object2spin = QtWidgets.QSpinBox()
        self.object2spin.setRange(0, len(Tileset.objects)-1)
        self.objectOkButton = QtWidgets.QPushButton("Switch")

        layout = QtWidgets.QGridLayout()
        layout.addWidget(self.object1label, 0, 0, 1, 1)
        layout.addWidget(self.object1spin, 0, 1, 1, 1)
        layout.addWidget(self.object2label, 0, 2, 1, 1)
        layout.addWidget(self.object2spin, 0, 3, 1, 1)
        layout.addWidget(self.objectOkButton, 1, 0, 1, 4)
        self.switchObjectsWindow.setLayout(layout)

        self.objectOkButton.released.connect(self.actuallySwitchObjects)

        self.switchObjectsWindow.setWindowTitle('Switch Objects')
        self.switchObjectsWindow.setFixedSize(self.switchObjectsWindow.sizeHint())
        self.switchObjectsWindow.setWindowFlag(Qt.WindowMinimizeButtonHint, False)
        self.switchObjectsWindow.setWindowFlag(Qt.WindowMaximizeButtonHint, False)
        self.switchObjectsWindow.setWindowFlag(Qt.WindowStaysOnTopHint, True)
        self.switchObjectsWindow.show()


    def actuallySwitchObjects(self):
        self.switchObjectsWindow.hide()

        # Prevent a crash if the object list is empty
        if (len(Tileset.objects) < 1):
            return

        temp = Tileset.objects[self.object1spin.value()]
        Tileset.objects[self.object1spin.value()] = Tileset.objects[self.object2spin.value()]
        Tileset.objects[self.object2spin.value()] = temp

        index = self.objectList.currentIndex()
        self.objectList.clearCurrentIndex()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        self.objectList.setCurrentIndex(self.objmodel.index(index.row(), index.column()))
        self.objectList.update()


    def saveAllObjects(self):
        tile_name = (os.path.basename(self.name) if self.name else 'NewTileset')

        save_path = QtWidgets.QFileDialog.getExistingDirectory(None, "Choose where to save the Object folder")
        if not save_path: return

        for object in Tileset.objects:
            object.jsonData = {}

        count = 0
        for object in Tileset.objects:
            tex = QtGui.QPixmap(object.width * 24, object.height * 24)
            tex.fill(Qt.transparent)
            painter = QtGui.QPainter(tex)

            Xoffset = 0
            Yoffset = 0

            Tilebuffer = b''

            for i in range(len(object.tiles)):
                for tile in object.tiles[i]:
                    if (Tileset.slot == 0) or ((tile[2] & 3) != 0):
                        painter.drawImage(Xoffset, Yoffset, Tileset.tiles[tile[1]].image)
                    Tilebuffer += (Tileset.tiles[tile[1]].byte0).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte1).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte2).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte3).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte4).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte5).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte6).to_bytes(1, 'big')
                    Tilebuffer += (Tileset.tiles[tile[1]].byte7).to_bytes(1, 'big')
                    Xoffset += 24
                Xoffset = 0
                Yoffset += 24

            painter.end()

            # Slopes
            if object.upperslope[0] != 0:

                # Reverse Slopes
                if object.upperslope[0] & 0x2:
                    a = struct.pack('>B', object.upperslope[0])

                    if object.height == 1:
                        iterationsA = 0
                        iterationsB = 1
                    else:
                        iterationsA = object.upperslope[1]
                        iterationsB = object.lowerslope[1] + object.upperslope[1]

                    for row in range(iterationsA, iterationsB):
                        for tile in object.tiles[row]:
                            a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                        a += b'\xfe'

                    if object.height > 1:
                        a += struct.pack('>B', object.lowerslope[0])

                        for row in range(0, object.upperslope[1]):
                            for tile in object.tiles[row]:
                                a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                            a += b'\xfe'

                    a += b'\xff'


                # Regular Slopes
                else:
                    a = struct.pack('>B', object.upperslope[0])

                    #for row in range(0, object.upperslope[1]):
                    #    for tile in object.tiles[row]:
                    #        a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                    #    a += b'\xfe'

                    if object.height > 1:
                        a += struct.pack('>B', object.lowerslope[0])

                        for row in range(object.upperslope[1], object.height):
                            for tile in object.tiles[row]:
                                a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                            a += b'\xfe'

                    a += b'\xff'


            # Not slopes!
            else:
                a = b''

                for tilerow in object.tiles:
                    for tile in tilerow:
                        a += struct.pack('>BBB', tile[0], tile[1], tile[2])

                    a += b'\xfe'

                a += b'\xff'

            Objbuffer = a
            Metabuffer = struct.pack('>HBB', (0 if count == 0 else len(Objbuffer)), object.width, object.height)

            if not os.path.isdir(save_path + "/" + tile_name + "_objects"):
                os.mkdir(save_path + "/" + tile_name + "_objects")

            tex.save(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(count) + ".png", "PNG")

            object.jsonData['img'] = tile_name + "_object_" + str(count) + ".png"

            with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(count) + ".colls", "wb+") as colls:
                colls.write(Tilebuffer)

            object.jsonData['colls'] = tile_name + "_object_" + str(count) + ".colls"

            with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(count) + ".objlyt", "wb+") as objlyt:
                objlyt.write(Objbuffer)

            object.jsonData['objlyt'] = tile_name + "_object_" + str(count) + ".objlyt"

            with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(count) + ".meta", "wb+") as meta:
                meta.write(Metabuffer)

            object.jsonData['meta'] = tile_name + "_object_" + str(count) + ".meta"

            count += 1

        count = 0
        for object in Tileset.objects:
            with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(count) + ".json", 'w+') as outfile:
                json.dump(object.jsonData, outfile)

            count += 1


    def saveFramesheet(self, index):
        item = self.framesheetmodel.itemFromIndex(index)
        icon = item.icon()
        name = item.text()

        path = QtWidgets.QFileDialog.getSaveFileName(self, 'Framesheet', '{}.png'.format(name), 'Framesheet (*.png)')[0]
        if not path: return


        pixmap = icon.pixmap(icon.availableSizes()[0])
        pixmap.save(path, "PNG")



    def saveObject(self, index):
        if len(Tileset.objects) == 0: return

        tile_name = (os.path.basename(self.name) if self.name else 'NewTileset')

        save_path = QtWidgets.QFileDialog.getExistingDirectory(None, "Choose where to save the Object folder")
        if not save_path: return

        n = index.row()

        object = Tileset.objects[n]

        object.jsonData = {}

        tex = QtGui.QPixmap(object.width * 24, object.height * 24)
        tex.fill(Qt.transparent)
        painter = QtGui.QPainter(tex)

        Xoffset = 0
        Yoffset = 0

        Tilebuffer = b''

        for i in range(len(object.tiles)):
            for tile in object.tiles[i]:
                if (Tileset.slot == 0) or ((tile[2] & 3) != 0):
                    painter.drawImage(Xoffset, Yoffset, Tileset.tiles[tile[1]].image)
                Tilebuffer += (Tileset.tiles[tile[1]].byte0).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte1).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte2).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte3).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte4).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte5).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte6).to_bytes(1, 'big')
                Tilebuffer += (Tileset.tiles[tile[1]].byte7).to_bytes(1, 'big')
                Xoffset += 24
            Xoffset = 0
            Yoffset += 24

        painter.end()

        # Slopes
        if object.upperslope[0] != 0:

            # Reverse Slopes
            if object.upperslope[0] & 0x2:
                a = struct.pack('>B', object.upperslope[0])

                if object.height == 1:
                    iterationsA = 0
                    iterationsB = 1
                else:
                    iterationsA = object.upperslope[1]
                    iterationsB = object.lowerslope[1] + object.upperslope[1]

                for row in range(iterationsA, iterationsB):
                    for tile in object.tiles[row]:
                        a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                    a += b'\xfe'

                if object.height > 1:
                    a += struct.pack('>B', object.lowerslope[0])

                    for row in range(0, object.upperslope[1]):
                        for tile in object.tiles[row]:
                            a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                        a += b'\xfe'

                a += b'\xff'


            # Regular Slopes
            else:
                a = struct.pack('>B', object.upperslope[0])

                for row in range(0, object.upperslope[1]):
                    for tile in object.tiles[row]:
                        a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                    a += b'\xfe'

                if object.height > 1:
                    a += struct.pack('>B', object.lowerslope[0])

                    for row in range(object.upperslope[1], object.height):
                        for tile in object.tiles[row]:
                            a += struct.pack('>BBB', tile[0], tile[1], tile[2])
                        a += b'\xfe'

                a += b'\xff'


        # Not slopes!
        else:
            a = b''

            for tilerow in object.tiles:
                for tile in tilerow:
                    a += struct.pack('>BBB', tile[0], tile[1], tile[2])

                a += b'\xfe'

            a += b'\xff'

        Objbuffer = a
        Metabuffer = struct.pack('>HBB', (0 if n == 0 else len(Objbuffer)), object.width, object.height)

        if not os.path.isdir(save_path + "/" + tile_name + "_objects"):
            os.mkdir(save_path + "/" + tile_name + "_objects")

        tex.save(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(n) + ".png", "PNG")

        object.jsonData['img'] = tile_name + "_object_" + str(n) + ".png"

        with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(n) + ".colls", "wb+") as colls:
            colls.write(Tilebuffer)

        object.jsonData['colls'] = tile_name + "_object_" + str(n) + ".colls"

        with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(n) + ".objlyt", "wb+") as objlyt:
            objlyt.write(Objbuffer)

        object.jsonData['objlyt'] = tile_name + "_object_" + str(n) + ".objlyt"

        with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(n) + ".meta", "wb+") as meta:
            meta.write(Metabuffer)

        object.jsonData['meta'] = tile_name + "_object_" + str(n) + ".meta"

        if not os.path.isdir(save_path + "/" + tile_name + "_objects"):
            os.mkdir(save_path + "/" + tile_name + "_objects")

        with open(save_path + "/" + tile_name + "_objects" + "/" + tile_name + "_object_" + str(n) + ".json", 'w+') as outfile:
            json.dump(object.jsonData, outfile)


    def clearObjects(self):
        '''Clears the object data'''

        Tileset.objects = []
        Tileset.animdata = {}

        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        SetupFramesheetModel(self, Tileset.animdata)


        self.objectList.update()
        self.framesheetList.update()
        self.tileWidget.update()


    def clearOverrides(self):
        '''Clears the override tabs'''
        self.animOverwriteEditor.load_from_bin(None)
        self.randOverwriteEditor.load_from_bin(None)
        self.plantOverwriteEditor.load_from_bin(None)
        self.profileOverwriteEditor.load_from_bin(None)


    def clearCollisions(self):
        '''Clears the collisions data'''

        for tile in Tileset.tiles:
            tile.byte0 = 0
            tile.byte1 = 0
            tile.byte2 = 0
            tile.byte3 = 0
            tile.byte4 = 0
            tile.byte5 = 0
            tile.byte6 = 0
            tile.byte7 = 0

        self.updateInfo(0, 0)
        self.tileDisplay.update()


    def setupWidgets(self):
        frame = QtWidgets.QFrame()
        frameLayout = QtWidgets.QGridLayout(frame)

        # Displays the tiles
        self.tileDisplay = displayWidget()

        # Info Box for tile information
        self.infoDisplay = InfoBox(self)

        # Sets up the model for the tile pieces
        self.model = PiecesModel(self)
        self.tileDisplay.setModel(self.model)

        # Object List
        self.objectList = objectList()
        self.objmodel = QtGui.QStandardItemModel()
        SetupObjectModel(self.objmodel, Tileset.objects, Tileset.tiles)
        self.objectList.setModel(self.objmodel)

        self.importObject = QtWidgets.QPushButton('Import Object')
        self.moveObjUp = QtWidgets.QPushButton('Move Object Up')
        self.moveObjDown = QtWidgets.QPushButton('Move Object Down')
        self.swapObjects = QtWidgets.QPushButton('Switch Objects')

        # Creates the Tab Widget for behaviours and objects
        self.tabWidget = QtWidgets.QTabWidget()
        self.tabWidget.setUsesScrollButtons(False)
        self.framesheetWidget = framesheetOverlord()
        self.tileWidget = tileOverlord()
        self.paletteWidget = paletteWidget(self)

        # Second Tab
        self.container = QtWidgets.QWidget()
        layout = QtWidgets.QGridLayout()
        layout.addWidget(self.objectList, 3, 0, 1, 1)
        layout.addWidget(self.importObject, 4, 0, 1, 1)
        layout.addWidget(self.moveObjUp, 0, 0, 1, 1)
        layout.addWidget(self.moveObjDown, 1, 0, 1, 1)
        layout.addWidget(self.swapObjects, 2, 0, 1, 1)
        layout.addWidget(self.tileWidget, 0, 1, 5, 1)
        self.container.setLayout(layout)

        # Framesheet List
        self.framesheetList = framesheetList()
        self.framesheetmodel = QtGui.QStandardItemModel()
        self.frames = {}
        SetupFramesheetModel(self, Tileset.animdata)
        self.framesheetList.setModel(self.framesheetmodel)

        # Third Tab
        self.framesheetContainer = QtWidgets.QWidget()
        animationLayout = QtWidgets.QVBoxLayout()
        animationLayout.addWidget(self.framesheetList)
        animationLayout.addWidget(self.framesheetWidget)
        self.framesheetContainer.setLayout(animationLayout)

        # Fourth Tab
        self.animOverwriteEditor = AnimTileWidget()

        # Fifth Tab
        self.randOverwriteEditor = RandTileWidget()

        # Sixth Tab
        self.plantOverwriteEditor = FlowerGrassWidget()

        # Seventh Tab
        self.profileOverwriteEditor = ProfileOverrideWidget()

        # Sets the Tabs
        self.tabWidget.addTab(self.paletteWidget, 'Behaviors')
        self.tabWidget.addTab(self.container, 'Objects')
        self.tabWidget.addTab(self.framesheetContainer, 'Framesheets')
        self.tabWidget.addTab(self.animOverwriteEditor, 'AnimTiles')
        self.tabWidget.addTab(self.randOverwriteEditor, 'RandTiles')
        self.tabWidget.addTab(self.plantOverwriteEditor, "Flower/Grass Overrides")
        self.tabWidget.addTab(self.profileOverwriteEditor, "Profile Overrides")

        # Connections do things!
        self.tileDisplay.clicked.connect(self.paintFormat)
        self.tileDisplay.mouseMoved.connect(self.updateInfo)
        self.tileDisplay.rightClicked.connect(self.editHexData)
        self.objectList.selectionModel().currentChanged.connect(self.tileWidget.setObject)
        self.objectList.doubleClicked.connect(self.saveObject)
        self.importObject.released.connect(self.importObjFromFile)
        self.moveObjUp.released.connect(self.moveObjectUp)
        self.moveObjDown.released.connect(self.moveObjectDown)
        self.swapObjects.released.connect(self.switchObjects)

        self.framesheetList.doubleClicked.connect(self.saveFramesheet)

        # Layout
        if self.isSplitWindow:
            frameLayout.addWidget(self.infoDisplay, 0, 0, 1, 3, Qt.AlignTop)
            frameLayout.addWidget(self.tileDisplay, 1, 0, 1, 3, Qt.AlignTop | Qt.AlignCenter)
            self.anotherWidget = QtWidgets.QWidget()
            anotherLayout = QtWidgets.QGridLayout()
            anotherLayout.addWidget(self.tabWidget, 0, 3, 2, 3)
            anotherLayout.setMenuBar(QtWidgets.QWidget(self.menuBar()))
            self.anotherWidget.setLayout(anotherLayout)
            self.anotherWidget.setWindowTitle (" ")
            self.anotherWidget.setWindowFlags(Qt.WindowStaysOnTopHint)
            self.anotherWidget.setAttribute(Qt.WA_DeleteOnClose)
            self.anotherWidget.destroyed.connect(osExit)
            self.anotherWidget.show()
        else:
            frameLayout.addWidget(self.infoDisplay, 0, 0, 1, 3, Qt.AlignTop)
            frameLayout.addWidget(self.tileDisplay, 1, 0, 1, 3, Qt.AlignCenter)
            frameLayout.addWidget(self.tabWidget, 0, 3, 3, 3)
        self.setCentralWidget(frame)


    def updateInfo(self, x, y, noRecursion = False, realIndex = None):
        index = [self.tileDisplay.indexAt(QtCore.QPoint(x, y))]
        
        if not noRecursion:
            if app.mouseButtons() == QtCore.Qt.LeftButton:
                if not index[0].column() == -1:
                    self.paintFormat(index[0])
        else:
            index[0] = realIndex
        
        if self.tabWidget.currentIndex() != 1:
            self.tileDisplay.setCurrentIndex(index[0])
        
        curTile = Tileset.tiles[index[0].row()]
        info = self.infoDisplay
        palette = self.paletteWidget

        propertyList = []
        propertyText = ''
        coreType = 0

        if curTile.byte3 & 32:
            coreType = 1
        elif curTile.byte3 & 64:
            coreType = 2
        elif curTile.byte2 & 8:
            coreType = 3
        elif curTile.byte3 & 2:
            coreType = 4
        elif curTile.byte3 & 8:
            coreType = 5
        elif curTile.byte2 & 4:
            coreType = 6
        elif curTile.byte2 & 16:
            coreType = 7
        elif curTile.byte1 & 1:
            coreType = 8
        elif 0 > curTile.byte7 > 0x23:
            coretype = 9
        elif curTile.byte5 == 4 or 5:
            coretype = 10
        elif curTile.byte3 & 4:
            coreType = 11

        if curTile.byte3 & 1:
            propertyList.append('Solid')
        if curTile.byte3 & 16:
            propertyList.append('Breakable')
        if curTile.byte2 & 128:
            propertyList.append('Pass-Through')
        if curTile.byte2 & 64:
            propertyList.append('Pass-Down')
        if curTile.byte1 & 2:
            propertyList.append('Falling')
        if curTile.byte1 & 8:
            propertyList.append('Ledge')
        if curTile.byte0 & 2:
            propertyList.append('Meltable')


        if len(propertyList) == 0:
            propertyText = 'None'
        elif len(propertyList) == 1:
            propertyText = propertyList[0]
        else:
            propertyText = propertyList.pop(0)
            for string in propertyList:
                propertyText = propertyText + ', ' + string

        if coreType == 0:
            if curTile.byte7 == 0x23:
                parameter = palette.ParameterList[coreType][1]
            elif curTile.byte7 == 0x28:
                parameter = palette.ParameterList[coreType][2]
            elif curTile.byte7 >= 0x35:
                parameter = palette.ParameterList[coreType][curTile.byte7 - 0x32]
            else:
                parameter = palette.ParameterList[coreType][0]
        else:
            try:
                parameter = palette.ParameterList[coreType][curTile.byte7]
            except:
                parameter = ["Unknown", QtGui.QPixmap(24, 24)]
                parameter[1].fill(Qt.transparent)

        try:
            info.coreImage.setPixmap(palette.coreTypes[coreType][1].pixmap(24,24))
            info.coreInfo.setText(palette.coreTypes[coreType][0])
        except:
            info.coreImage.clear()
            info.coreInfo.setText("Unknown")
        try:
            info.terrainImage.setPixmap(palette.terrainTypes[curTile.byte5][1].pixmap(24,24))
            info.terrainInfo.setText(palette.terrainTypes[curTile.byte5][0])
        except:
            info.terrainImage.clear()
            info.terrainInfo.setText("Unknown")
        try:
            info.parameterImage.setPixmap(parameter[1].pixmap(24,24))
            info.paramInfo.setText(parameter[0])
        except:
            info.parameterImage.clear()
            info.paramInfo.setText("Unknown")


        info.propertyInfo.setText("Properties:\n{0}".format(propertyText))
        


        info.hexdata.setText('Hex Data: {0} {1} {2} {3} {4} {5} {6} {7}'.format(
                                hex(curTile.byte0), hex(curTile.byte1), hex(curTile.byte2), hex(curTile.byte3),
                                hex(curTile.byte4), hex(curTile.byte5), hex(curTile.byte6), hex(curTile.byte7)))

        column = index[0].row() % 16
        row = index[0].row() // 16
        if 0 <= row <= 15 and 0 <= column <= 15:
            info.numInfo.setText('Slot: %X Row: 0x%X Column: 0x%X' % (Tileset.slot, row, column))


    def editHexData(self, x, y):
        self.index = [self.tileDisplay.indexAt(QtCore.QPoint(x, y))][0]
        curTile = Tileset.tiles[self.index.row()]
        
        self.hexEditWindow = QtWidgets.QWidget()
        self.hexOkButton = QtWidgets.QPushButton("Set")

        self.spin1 = QtWidgets.QSpinBox()
        self.spin1.setDisplayIntegerBase(16)
        self.spin1.setRange(0, 0xFF)
        self.spin1.setValue(curTile.byte0)
        self.spin2 = QtWidgets.QSpinBox()
        self.spin2.setDisplayIntegerBase(16)
        self.spin2.setRange(0, 0xFF)
        self.spin2.setValue(curTile.byte1)
        self.spin3 = QtWidgets.QSpinBox()
        self.spin3.setDisplayIntegerBase(16)
        self.spin3.setRange(0, 0xFF)
        self.spin3.setValue(curTile.byte2)
        self.spin4 = QtWidgets.QSpinBox()
        self.spin4.setDisplayIntegerBase(16)
        self.spin4.setRange(0, 0xFF)
        self.spin4.setValue(curTile.byte3)
        self.spin5 = QtWidgets.QSpinBox()
        self.spin5.setDisplayIntegerBase(16)
        self.spin5.setRange(0, 0xFF)
        self.spin5.setValue(curTile.byte4)
        self.spin6 = QtWidgets.QSpinBox()
        self.spin6.setDisplayIntegerBase(16)
        self.spin6.setRange(0, 0xFF)
        self.spin6.setValue(curTile.byte5)
        self.spin7 = QtWidgets.QSpinBox()
        self.spin7.setDisplayIntegerBase(16)
        self.spin7.setRange(0, 0xFF)
        self.spin7.setValue(curTile.byte6)
        self.spin8 = QtWidgets.QSpinBox()
        self.spin8.setDisplayIntegerBase(16)
        self.spin8.setRange(0, 0xFF)
        self.spin8.setValue(curTile.byte7)
        
        spinFont = self.spin1.font()
        spinFont.setCapitalization(QtGui.QFont.AllUppercase)
        self.spin1.setFont(spinFont)
        self.spin2.setFont(spinFont)
        self.spin3.setFont(spinFont)
        self.spin4.setFont(spinFont)
        self.spin5.setFont(spinFont)
        self.spin6.setFont(spinFont)
        self.spin7.setFont(spinFont)
        self.spin8.setFont(spinFont)

        layout = QtWidgets.QGridLayout()
        layout.addWidget(self.spin1, 0, 0, 1, 1)
        layout.addWidget(self.spin2, 0, 1, 1, 1)
        layout.addWidget(self.spin3, 0, 2, 1, 1)
        layout.addWidget(self.spin4, 0, 3, 1, 1)
        layout.addWidget(self.spin5, 0, 4, 1, 1)
        layout.addWidget(self.spin6, 0, 5, 1, 1)
        layout.addWidget(self.spin7, 0, 6, 1, 1)
        layout.addWidget(self.spin8, 0, 7, 1, 1)
        layout.addWidget(self.hexOkButton, 1, 7, 1, 1)
        self.hexEditWindow.setLayout(layout)

        self.hexOkButton.released.connect(self.hexSetRaw)

        self.hexEditWindow.setWindowTitle('Set Raw Hex Data')
        self.hexEditWindow.setFixedSize(self.hexEditWindow.sizeHint())
        self.hexEditWindow.setWindowFlag(Qt.WindowMinimizeButtonHint, False)
        self.hexEditWindow.setWindowFlag(Qt.WindowMaximizeButtonHint, False)
        self.hexEditWindow.setWindowFlag(Qt.WindowStaysOnTopHint, True)
        self.hexEditWindow.show()
        
        
    def hexSetRaw(self):
        self.hexEditWindow.hide()
        
        curTile = Tileset.tiles[self.index.row()]
        
        curTile.byte0 = self.spin1.value()
        curTile.byte1 = self.spin2.value()
        curTile.byte2 = self.spin3.value()
        curTile.byte3 = self.spin4.value()
        curTile.byte4 = self.spin5.value()
        curTile.byte5 = self.spin6.value()
        curTile.byte6 = self.spin7.value()
        curTile.byte7 = self.spin8.value()
        

    def paintFormat(self, index):
        if self.tabWidget.currentIndex() != 0:
            return

        curTile = Tileset.tiles[index.row()]
        palette = self.paletteWidget

        if palette.coreWidgets[8].isChecked() == 1 or palette.propertyWidgets[0].isChecked() == 1:
            solid = 1
        else:
            solid = 0

        if palette.coreWidgets[1].isChecked() == 1 or palette.coreWidgets[2].isChecked() == 1:
            solid = 0


        curTile.byte0 = ((palette.propertyWidgets[4].isChecked() << 1))
        curTile.byte1 = ((palette.coreWidgets[8].isChecked()) +
                        (palette.propertyWidgets[2].isChecked() << 1) +
                        (palette.propertyWidgets[3].isChecked() << 3))
        curTile.byte2 = ((palette.coreWidgets[6].isChecked() << 2) +
                        (palette.coreWidgets[3].isChecked() << 3) +
                        (palette.coreWidgets[7].isChecked() << 4) +
                        (palette.PassDown.isChecked() << 6) +
                        (palette.PassThrough.isChecked() << 7))
        curTile.byte3 = ((solid) +
                        (palette.coreWidgets[4].isChecked() << 1) +
                        (palette.coreWidgets[5].isChecked() << 3) +
                        (palette.propertyWidgets[1].isChecked() << 4) +
                        (palette.coreWidgets[1].isChecked() << 5) +
                        (palette.coreWidgets[2].isChecked() << 6) +
                        (palette.coreWidgets[11].isChecked() << 2))
        curTile.byte4 = 0
        if palette.coreWidgets[2].isChecked():
            curTile.byte5 = 4
        curTile.byte5 = palette.terrainType.currentIndex()

        if palette.coreWidgets[0].isChecked():
            params = palette.parameters.currentIndex()
            if params == 0:
                curTile.byte7 = 0
            elif params == 1:
                curTile.byte7 = 0x23
            elif params == 2:
                curTile.byte7 = 0x28
            elif params >= 3:
                curTile.byte7 = params + 0x32
        else:
            curTile.byte7 = palette.parameters.currentIndex()

        self.updateInfo(0, 0, noRecursion = True, realIndex = index)
        self.tileDisplay.update()



#############################################################################################
####################################### Main Function #######################################

def osExit(self):
    os._exit(0)


if '-nolib' in sys.argv:
    HaveNSMBLib = False
    sys.argv.remove('-nolib')

if '-split' in sys.argv:
    sys.argv.remove('-split')

# export all .arc files from -export-all folder to -out folder as .png
# -export-all specifies an input path for .arc files and -out the output path for .png files
if '-export-all' in sys.argv and '-out' in sys.argv:
    import sys
    try:
        inputPath = os.path.normpath(sys.argv[sys.argv.index('-export-all') + 1])
        outputPath = os.path.normpath(sys.argv[sys.argv.index('-out') + 1])
        if not os.path.isdir(inputPath):
            raise Exception("Input path does not exist")
        if not os.path.isdir(outputPath):
            os.makedirs(outputPath)

        app = QtWidgets.QApplication(sys.argv)

        window = MainWindow()

        window.setAttribute(Qt.WA_DeleteOnClose)
        window.destroyed.connect(osExit)

        try:
            for root, dirs, files in os.walk(inputPath):
                for file in files:
                    if file.endswith('.arc'):
                        print(f"Exporting: {file} to {outputPath}")
                        window.openTilesetFromPath(os.path.join(root, file), suppressSlotWarning=True)
                        window.saveImage(os.path.join(outputPath, file.replace('.arc', '.png')))
        except Exception as e:
            print(f"Error exporting {file}: {e}")

        os._exit(0) # exit the program after exporting all .arc files

    except Exception as e:
        print("Error: {}".format(e))
        #print("Error: -export-all and -out arguments must be directly followed by a path")
        os._exit(1)
        
elif '-export-all' in sys.argv or '-out' in sys.argv:
    print("Error: -export-all and -out arguments must be used together, each directly followed by a path")
    sys.exit(1)


if __name__ == '__main__':
    import sys

    app = QtWidgets.QApplication(sys.argv)
    app.setAttribute(Qt.AA_DisableWindowContextHelpButton)

    # go to the script path
    path = module_path()
    if path is not None:
        os.chdir(path)

    app.setWindowIcon(QtGui.QIcon("Icons/about.png"))

    window = MainWindow()

    window.setAttribute(Qt.WA_DeleteOnClose)
    window.destroyed.connect(osExit)

    if len(sys.argv) > 1:
        window.openTilesetFromPath(sys.argv[1])
    window.show()

    sys.exit(app.exec_())
    app.deleteLater()
