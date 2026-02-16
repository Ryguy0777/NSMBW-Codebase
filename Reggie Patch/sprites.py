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


class SpriteImage_ColorExcSwitch(SLib.SpriteImage_StaticMultiple):  # 42
    def __init__(self, parent, scale=1.5):
        super().__init__(parent, scale)
        
    @staticmethod
    def loadImages():
        if 'ESwitch' not in ImageCache:
            e = SLib.GetImg('e_switch.png', True)
            ImageCache['ESwitch'] = QtGui.QPixmap.fromImage(e)
            ImageCache['ESwitchU'] = QtGui.QPixmap.fromImage(e.mirrored(True, True))
        
        if 'ESwitch_Y' not in ImageCache:
            e = SLib.GetImg('e_switch_y.png', True)
            ImageCache['ESwitch_Y'] = QtGui.QPixmap.fromImage(e)
            ImageCache['ESwitchU_Y'] = QtGui.QPixmap.fromImage(e.mirrored(True, True))
        
        if 'ESwitch_G' not in ImageCache:
            e = SLib.GetImg('e_switch_g.png', True)
            ImageCache['ESwitch_G'] = QtGui.QPixmap.fromImage(e)
            ImageCache['ESwitchU_G'] = QtGui.QPixmap.fromImage(e.mirrored(True, True))
        
        if 'ESwitch_B' not in ImageCache:
            e = SLib.GetImg('e_switch_b.png', True)
            ImageCache['ESwitch_B'] = QtGui.QPixmap.fromImage(e)
            ImageCache['ESwitchU_B'] = QtGui.QPixmap.fromImage(e.mirrored(True, True))

    def dataChanged(self):

        upsideDown = self.parent.spritedata[5] & 1
        
        styleType = self.parent.spritedata[5] >> 1 & 7
        
        if styleType == 0 or styleType == 2:
            switchType = ''
        elif styleType == 1:
            switchType = '_Y'
        elif styleType == 3:
            switchType = '_G'
        elif styleType == 4:
            switchType = '_B'

        if upsideDown:
            self.image = ImageCache['ESwitchU' + switchType]
        else:
            self.image = ImageCache['ESwitch' + switchType]
            self.yOffset -= 3

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


class SpriteImage_CliffKoopa(SLib.SpriteImage_StaticMultiple):  # 302
    def __init__(self, parent):
        super().__init__(parent, 1.5)

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('FenceKoopaHG', 'fencekoopa_horz.png')
        SLib.loadIfNotInImageCache('FenceKoopaHR', 'fencekoopa_horz_red.png')

    def dataChanged(self):

        fix = self.parent.spritedata[5] & 1
        if fix == 1:
            self.offset = (-3, -13)
        else:
            self.offset = (-3, 2)
        
        color = self.parent.spritedata[4] & 1
        if color == 1:
            self.image = ImageCache['FenceKoopaHR']
        else:
            self.image = ImageCache['FenceKoopaHG']

        super().dataChanged()


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


class SpriteImage_OnOffBlock(SLib.SpriteImage_Static):  # 489
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            SLib.GetTile(0x9C),
        )

    
class SpriteImage_NipperPlant(SLib.SpriteImage_StaticMultiple):  # 490
    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Nipper', 'nipper.png')
        SLib.loadIfNotInImageCache('NipperF', 'nipper_frozen.png')

    def dataChanged(self):

        frozen = self.parent.spritedata[5] & 1
        if frozen == 1:
            self.image = ImageCache['NipperF']
            self.offset = (0, 0)
        else:
            self.image = ImageCache['Nipper']
            self.offset = (0, -1)

        super().dataChanged()


class SpriteImage_MessageBlock(SLib.SpriteImage_Static):  # 491
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            SLib.GetTile(0x9A),
        )
    

class SpriteImage_BombBro(SLib.SpriteImage_Static):  # 492
    def __init__(self, parent, scale=1.5):
        super().__init__(
            parent,
            scale,
            ImageCache['BombBro'],
            (-4, -21)
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('BombBro', 'bombbro.png')


class SpriteImage_Splunkin(SLib.SpriteImage_Static):  # 495
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['Splunkin'],
            (-3, -4)
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Splunkin', 'splunkin.png')


class SpriteImage_MegaSplunkin(SLib.SpriteImage_Static):  # 496
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['SplunkinMega'],
            (-20, -34)
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('SplunkinMega', 'splunkin_mega.png')


class SpriteImage_JackOGoomba(SLib.SpriteImage_Static):  # 497
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['JackOGoomba'],
            (-4, -8)
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('JackOGoomba', 'jack-o-goomba.png')


class SpriteImage_Goombrat(SLib.SpriteImage_Static):  # 498
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['Goombrat'],
            (-2, -5)
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Goombrat', 'goombrat.png')


class SpriteImage_Galoomba(SLib.SpriteImage_Static):  # 499
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['Galoomba'],
            (-3, 0),
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Galoomba', 'galoomba.png')


class SpriteImage_ParaGaloomba(SLib.SpriteImage_Static):  # 500
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['ParaGaloomba'],
            (-2.5, -6.75),
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('ParaGaloomba', 'paragaloomba.png')


class SpriteImage_Goombud(SLib.SpriteImage_Static):  # 501
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['Goombud'],
            (-3, 0),
        )
    
    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('Goombud', 'goombud.png')


class SpriteImage_ShyguyBubble(SLib.SpriteImage_StaticMultiple):  # 502
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['ShyguyBubble'],
            (-10.75, -13),
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('ShyguyBubble', 'shyguy_bubble_idle.png')
        SLib.loadIfNotInImageCache('ShyguyBubble2', 'shyguy_bubble_moving.png')
        SLib.loadIfNotInImageCache('ShyguyBalloon', 'shyguy_balloon.png')
        
    def dataChanged(self):
        balloon = self.parent.spritedata[3] >> 2 & 1
        if balloon:
            self.width = 30
            self.height = 52
            
            self.image = ImageCache['ShyguyBalloon']
            self.offset = (-7, -8)
        else:
            self.width = 40
            self.height = 40
            
            moving = self.parent.spritedata[3] >> 5 & 3
            if moving > 0:
                self.image = ImageCache['ShyguyBubble2']
                self.offset = (-10.75, -13)
            else:
                self.image = ImageCache['ShyguyBubble']
                self.offset = (-10.75, -13)


class SpriteImage_ShyguyClimb(SLib.SpriteImage_StaticMultiple):  # 503
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            ImageCache['ShyguyClimbH'],
            (-2, -5),
        )

    @staticmethod
    def loadImages():
        SLib.loadIfNotInImageCache('ShyguyClimbH', 'shyguy_climb_h.png')
        SLib.loadIfNotInImageCache('ShyguyClimbV', 'shyguy_climb_v.png')
        
    def dataChanged(self):
        
        vertical = self.parent.spritedata[3] >> 2 & 1
        if vertical:
            self.image = ImageCache['ShyguyClimbV']
            self.offset = (-2, -5)
        else:
            self.image = ImageCache['ShyguyClimbH']
            self.offset = (-2, -5)


class SpriteImage_StarCoinFake(SLib.SpriteImage_Static):  # 510
    def __init__(self, parent, scale=1.5):
        super().__init__(
            parent,
            scale,
            ImageCache['StarCoin'],
            (0, 3),
        )


class SpriteImage_SwitchBlock(SLib.SpriteImage_StaticMultiple):  # 528
    def __init__(self, parent):
        super().__init__(
            parent,
            1.5,
            SLib.GetTile(0xB6),
        )
        
    def dataChanged(self):
        palace = self.parent.spritedata[5] & 0xF
        tile_ = 0xB6 + palace
        
        self.image = SLib.GetTile(tile_)

ImageClasses = {
    22: SpriteImage_MiniGoomba,
    42: SpriteImage_ColorExcSwitch,
    191: SpriteImage_TileEventImproved,
    302: SpriteImage_CliffKoopa,
    486: SpriteImage_WaterPlatform,
    487: SpriteImage_Shyguy,
    488: SpriteImage_FlipBlock,
    489: SpriteImage_OnOffBlock,
    490: SpriteImage_NipperPlant,
    491: SpriteImage_MessageBlock,
    492: SpriteImage_BombBro,
    495: SpriteImage_Splunkin,
    496: SpriteImage_MegaSplunkin,
    497: SpriteImage_JackOGoomba,
    498: SpriteImage_Goombrat,
    499: SpriteImage_Galoomba,
    500: SpriteImage_ParaGaloomba,
    501: SpriteImage_Goombud,
    502: SpriteImage_ShyguyBubble,
    503: SpriteImage_ShyguyClimb,
    510: SpriteImage_StarCoinFake,
    528: SpriteImage_SwitchBlock,
}