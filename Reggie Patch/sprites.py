from PyQt6 import QtCore, QtGui
Qt = QtCore.Qt

import spritelib as SLib
import sprites_common as common

ImageCache = SLib.ImageCache

################################################################################
################################################################################
################################################################################

class SpriteImage_TileEventImproved(common.SpriteImage_TileEvent):  # 191
    def __init__(self, parent):
        super().__init__(parent)
        self.notAllowedTypes = (2, 5, 6, 7, 12, 13, 14, 15)

    def getTileFromType(self, type_):
        if type_ == 0:
            return SLib.GetTile(55)

        if type_ == 1:
            return SLib.GetTile(48)

        if type_ == 3:
            return SLib.GetTile(52)

        if type_ == 4:
            return SLib.GetTile(51)

        if type_ in [8, 9, 10, 11]:
            row = self.parent.spritedata[2] & 0xF
            col = self.parent.spritedata[3] >> 4

            tilenum = 256 * (type_ - 8)
            tilenum += row * 16 + col

            return SLib.GetTile(tilenum)

        return None

ImageClasses = {
    191: SpriteImage_TileEventImproved,
}