from PyQt6 import QtCore, QtGui
Qt = QtCore.Qt

import spritelib as SLib
import sprites_common as common

ImageCache = SLib.ImageCache

################################################################################
################################################################################
################################################################################

class SpriteImage_MiniGoomba(SLib.SpriteImage_Static):  # 22
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
        

class SpriteImage_Shyguy(SLib.SpriteImage_StaticMultiple):  # 487
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['Shyguy'],
            (-2.5, -7.5),
        )
        self.aux.append(SLib.AuxiliaryTrackObject(parent, 0, 0, SLib.AuxiliaryTrackObject.Horizontal))

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Shyguy', 'shyguy.png')
        SLib.loadIfNotInImageCache('ShyguySleep', 'shyguy_sleeper.png')
        SLib.loadIfNotInImageCache('ShyguyJump', 'shyguy_jumper.png')
        
    def dataChanged(self):
        
        type = self.parent.spritedata[2] >> 4 & 0xF
        distance = (self.parent.spritedata[4] >> 4 & 0xF) * 32
        if type == 2:
            self.image = ImageCache['ShyguySleep']
            self.offset = (-2.5, -5)
        elif type == 3:
            self.image = ImageCache['ShyguyJump']
            self.offset = (-1, -7)
        else:
            self.image = ImageCache['Shyguy']
            self.offset = (-2.5, -7.5)
        
        if type == 4:
            self.aux[0].setSize(distance, 8)
            self.aux[0].setPos((-distance // 2 - self.offset[0] + 8) * 1.5, 16)
        else:
            self.aux[0].setSize(0, 0)


class SpriteImage_FlipBlock(SLib.SpriteImage):  # 488
    def __init__(self, parent, scale=1.5):
        super().__init__(parent, scale)
        self.spritebox.shown = False

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Flipblock', 'flipblock.png')
    
    def dataChanged(self):
        super().dataChanged()

        # SET CONTENTS
        # In the block_contents.png file:
        # 0 = Empty, 1 = Coin, 2 = Mushroom, 3 = Fire Flower, 4 = Propeller, 5 = Penguin Suit,
        # 6 = Mini Shroom, 7 = Star, 8 = Continuous Star, 9 = Yoshi Egg, 10 = 10 Coins,
        # 11 = 1-up, 12 = Vine, 13 = Spring, 14 = Shroom/Coin, 15 = Ice Flower, 16 = Toad, 17 = Hammer

        contents = self.parent.spritedata[5] & 0xF

        if contents == 2:  # 1 and 2 are always fire flowers
            contents = 3

        self.image = ImageCache['BlockContents'][contents]

    def paint(self, painter):
        super().paint(painter)

        painter.setRenderHint(QtGui.QPainter.RenderHint.Antialiasing)
        painter.drawPixmap(0, 0, ImageCache['Flipblock'])
        painter.drawPixmap(0, 0, self.image)

ImageClasses = {
    22: SpriteImage_MiniGoomba,
    191: SpriteImage_TileEventImproved,
    486: SpriteImage_WaterPlatform,
    487: SpriteImage_Shyguy,
    488: SpriteImage_FlipBlock
}