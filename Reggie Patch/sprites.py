from PyQt6 import QtCore, QtGui
Qt = QtCore.Qt

import spritelib as SLib
import sprites_common as common

ImageCache = SLib.ImageCache

################################################################################
################################################################################
################################################################################

class SpriteImage_MiniGoomba(SLib.SpriteImage_StaticMultiple):  # 22
    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Goomba', 'goomba.png')

    def dataChanged(self):
        base = ImageCache['Goomba']
        self.image = base.scaled(
            int(base.width() * 0.6),
            int(base.height() * 0.6),
        )
        self.offset = (3, 4)

        super().dataChanged()

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

class SpriteImage_WaterPlatform(SLib.SpriteImage):  # 486
    def __init__(self, parent, scale=1.5):
        super().__init__(parent, scale)
        self.spritebox.shown = False
        self.offset = (-32, -8)
        self.width = 64

    @staticmethod
    def loadImages():
        if 'WoodenPlatformL' not in ImageCache:
            ImageCache['WoodenPlatformL'] = SLib.GetImg('wood_platform_left.png')
            ImageCache['WoodenPlatformM'] = SLib.GetImg('wood_platform_middle.png')
            ImageCache['WoodenPlatformR'] = SLib.GetImg('wood_platform_right.png')

    def paint(self, painter):
        super().paint(painter)
        painter.drawTiledPixmap(24, 0, int((self.width * 1.5) - 48), int(self.height * 1.5), ImageCache['WoodenPlatformM'])
        painter.drawPixmap(int((self.width - 16) * 1.5), 0, ImageCache['WoodenPlatformR'])
        painter.drawPixmap(0, 0, ImageCache['WoodenPlatformL'])

ImageClasses = {
    22: SpriteImage_MiniGoomba,
    191: SpriteImage_TileEventImproved,
    486: SpriteImage_WaterPlatform
}