import copy
import io
import json

from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout, QHBoxLayout, QPushButton, QTableWidget,
    QTableWidgetItem, QFileDialog, QLabel, QLineEdit, QHeaderView
)

import tools.grass as grass

__EMPTY__ = {"flowerfile": 0, "grassfile": 0, "entries": []}


class FlowerGrassWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._grass_data = None

        self.hasGrassBRRES = False
        self.hasFlowerBRRES = False

        self.grassResData = None
        self.flowerResData = None

        self.currentData = copy.deepcopy(__EMPTY__)
        mainLayout = QVBoxLayout(self)

        topButtonLayout = QHBoxLayout()
        self.addRowButton = QPushButton("Add Entry")
        self.removeRowButton = QPushButton("Remove Entry")

        topButtonLayout.addWidget(self.addRowButton)
        topButtonLayout.addWidget(self.removeRowButton)
        mainLayout.addLayout(topButtonLayout)

        self.addRowButton.clicked.connect(self.addRow)
        self.removeRowButton.clicked.connect(self.removeRow)

        self.table = QTableWidget()
        mainLayout.addWidget(self.table)

        self.headers = ["Tile Number", "Flower Type", "Grass Type"]
        self.table.setColumnCount(len(self.headers))
        self.table.setHorizontalHeaderLabels(self.headers)

        # Make columns fill all width evenly, not user-resizable
        header = self.table.horizontalHeader()
        for col in range(len(self.headers)):
            header.setSectionResizeMode(col, QHeaderView.Stretch)
        header.setSectionsClickable(False)
        header.setSectionsMovable(False)

        self.table.setEditTriggers(QTableWidget.AllEditTriggers)

        dropdownList = [
            'Default (Nohara)',
            'Forest (Daishizen)',
            'Custom'
        ]

        buttonLayout = QHBoxLayout()

        self.loadJsonButton = QPushButton("Import JSON")
        self.saveJsonButton = QPushButton("Export JSON")
        self.loadBinButton = QPushButton("Import Binary")
        self.saveBinButton = QPushButton("Export Binary")

        buttonLayout.addWidget(self.loadJsonButton)
        buttonLayout.addWidget(self.saveJsonButton)
        buttonLayout.addWidget(self.loadBinButton)
        buttonLayout.addWidget(self.saveBinButton)
        mainLayout.addLayout(buttonLayout)

        self.loadJsonButton.clicked.connect(self.loadJson)
        self.saveJsonButton.clicked.connect(self.saveJson)
        self.loadBinButton.clicked.connect(self.loadBinFile)
        self.saveBinButton.clicked.connect(self.saveBinFile)

        resLabelLayout = QHBoxLayout()
        resLabel = QLabel('Resource Management:')

        resLabelLayout.addWidget(resLabel)
        mainLayout.addLayout(resLabelLayout)

        fileLayout = QtWidgets.QFormLayout()

        self.flowerfileEdit = QtWidgets.QComboBox()
        self.flowerfileEdit.addItems(dropdownList)

        self.grassfileEdit = QtWidgets.QComboBox()
        self.grassfileEdit.addItems(dropdownList)

        fileLayout.addRow('Flower File:', self.flowerfileEdit)
        fileLayout.addRow('Grass File:', self.grassfileEdit)
        mainLayout.addLayout(fileLayout)

        resButtonLayout = QtWidgets.QGridLayout()

        self.importGrassButton = QPushButton("Import Grass")
        self.exportGrassButton = QPushButton("Export Grass")
        self.importFlowerButton = QPushButton("Import Flower")
        self.exportFlowerButton = QPushButton("Export Flower")

        resButtonLayout.addWidget(self.importGrassButton,  0, 0)
        resButtonLayout.addWidget(self.exportGrassButton,  1, 0)
        resButtonLayout.addWidget(self.importFlowerButton, 0, 1)
        resButtonLayout.addWidget(self.exportFlowerButton, 1, 1)
        mainLayout.addLayout(resButtonLayout)

        self.importGrassButton.clicked.connect(self.importGrass)
        self.exportGrassButton.clicked.connect(self.exportGrass)
        self.importFlowerButton.clicked.connect(self.importFlower)
        self.exportFlowerButton.clicked.connect(self.exportFlower)

        self.exportGrassButton.setEnabled(self.hasGrassBRRES)
        self.exportFlowerButton.setEnabled(self.hasFlowerBRRES)

        self.setLayout(mainLayout)

    def _load_into_table(self, data):
        self.currentData = data
        # Update line edits
        flowerVal = data.get("flowerfile", 0)
        grassVal = data.get("grassfile", 0)

        flowerIdx = int(flowerVal)
        if flowerIdx > 2:
            flowerIdx = 2

        grassIdx = int(grassVal)
        if grassIdx > 2:
            grassIdx = 2

        self.flowerfileEdit.setCurrentIndex(flowerIdx)
        self.grassfileEdit.setCurrentIndex(grassIdx)

        # Populate the table
        entries = data.get("entries", [])
        self.table.setRowCount(len(entries))

        for rowIndex, entry in enumerate(entries):
            tilenum = entry.get("tilenum", 0)
            flowertype = entry.get("flowertype", 0)
            grasstype = entry.get("grasstype", 0)

            self._setTableItem(rowIndex, 0, str(tilenum))
            self._setTableItem(rowIndex, 1, str(flowertype))
            self._setTableItem(rowIndex, 2, str(grasstype))

    def updateCurrentData(self):
        try:
            flowerVal = self.flowerfileEdit.currentIndex()
        except ValueError:
            flowerVal = 0
        try:
            grassVal = self.grassfileEdit.currentIndex()
        except ValueError:
            grassVal = 0

        self.currentData["flowerfile"] = flowerVal
        self.currentData["grassfile"] = grassVal

        rowCount = self.table.rowCount()
        entries = []
        for rowIndex in range(rowCount):
            def textOrZero(r, c):
                item = self.table.item(r, c)
                return item.text().strip() if item else "0"

            tilenum    = int(textOrZero(rowIndex, 0))
            flowertype = int(textOrZero(rowIndex, 1))
            grasstype  = int(textOrZero(rowIndex, 2))

            entries.append({
                "tilenum": tilenum,
                "flowertype": flowertype,
                "grasstype": grasstype
            })

        self.currentData["entries"] = entries

    def to_bytes(self) -> bytes | None:
        self.updateCurrentData()

        if self.currentData == __EMPTY__:
            return None

        self._grass_data = grass.encode(self.currentData)

        return self._grass_data

    def load_from_bin(self, data: bytes | None):
        if data is None:
            self.table.setRowCount(0)
            self.currentData = copy.deepcopy(__EMPTY__)
            return

        self.currentData = grass.decode(io.BytesIO(data))
        self._load_into_table(self.currentData)
    
    def grass_to_bytes(self) -> bytes | None:
        if self.grassResData == None:
            return None

        return self.grassResData

    def grass_load_from_bin(self, data: bytes | None):
        if data is None:
            self.hasGrassBRRES = False
            self.exportGrassButton.setEnabled(False)
            return

        self.hasGrassBRRES = True
        self.exportGrassButton.setEnabled(True)
        self.grassResData = data

    def flower_to_bytes(self) -> bytes | None:
        if self.flowerResData == None:
            return None

        return self.flowerResData

    def flower_load_from_bin(self, data: bytes | None):
        if data is None:
            self.hasFlowerBRRES = False
            self.exportFlowerButton.setEnabled(False)
            return

        self.hasFlowerBRRES = True
        self.exportFlowerButton.setEnabled(True)
        self.flowerResData = data

    # -------------------------------------------------------------------------
    # Row Management: Add/Remove
    # -------------------------------------------------------------------------
    def addRow(self):
        """Append a new row to the bottom of the table with default values."""
        rowCount = self.table.rowCount()
        self.table.insertRow(rowCount)
        # Optional: set defaults
        self._setTableItem(rowCount, 0, "0")  # tilenum
        self._setTableItem(rowCount, 1, "0")  # flowertype
        self._setTableItem(rowCount, 2, "0")  # grasstype

    def removeRow(self):
        """Remove the currently selected row (if any)."""
        selectedRow = self.table.currentRow()
        if selectedRow >= 0:
            self.table.removeRow(selectedRow)

    # -------------------------------------------------------------------------
    # Table Helper
    # -------------------------------------------------------------------------
    def _setTableItem(self, row, col, text):
        item = QTableWidgetItem(text)
        item.setTextAlignment(Qt.AlignCenter)
        self.table.setItem(row, col, item)

    # -------------------------------------------------------------------------
    # Binary Load/Save
    # -------------------------------------------------------------------------
    def loadBinFile(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open .bin", "", "Binary Files (*.bin)")
        if not path:
            return

        self._load_into_table(grass.decode(open(path, 'rb')))

    def saveBinFile(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save .bin", "", "Binary Files (*.bin)")
        if not path:
            return

        self.updateCurrentData()

        try:
            with open(path, "wb") as f:
                f.write(grass.encode(self.currentData))
            print(f"Saved .bin to {path}")
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error',
                                          f"Error writing .bin: {e}")

    # -------------------------------------------------------------------------
    # JSON Load/Save
    # -------------------------------------------------------------------------
    def loadJson(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open JSON", "", "JSON Files (*.json)")
        if not path:
            return

        try:
            with open(path, 'r') as f:
                data = json.load(f)
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error',
                                          f"Error reading JSON: {e}")
            return

        self._load_into_table(data)

    def saveJson(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save JSON", "", "JSON Files (*.json)")
        if not path:
            return

        self.updateCurrentData()

        try:
            with open(path, 'w') as f:
                json.dump(self.currentData, f, indent=4)
            print(f"Saved JSON to {path}")
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error',
                                          f"Error writing JSON: {e}")

    # -------------------------------------------------------------------------
    # BRRES Load/Save
    # -------------------------------------------------------------------------
    def importGrass(self, data: bytes | None):
        path, _ = QFileDialog.getOpenFileName(self, "Open BRRES", "", "BRRES Files (*.brres)")
        if not path:
            return

        try:
            with open(path, 'rb') as f:
                self.grassResData = f.read()
                self.hasGrassBRRES = True
                self.exportGrassButton.setEnabled(True)
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error', f"Error reading BRRES: {e}")

    def exportGrass(self, data: bytes | None):
        path, _ = QFileDialog.getSaveFileName(self, "Save BRRES", "", "BRRES Files (*.brres)")
        if not path:
            return

        try:
            with open(path, "wb") as f:
                f.write(self.grassResData)
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error', f"Error writing BRRES: {e}")

    def importFlower(self, data: bytes | None):
        path, _ = QFileDialog.getOpenFileName(self, "Open BRRES", "", "BRRES Files (*.brres)")
        if not path:
            return

        try:
            with open(path, 'rb') as f:
                self.flowerResData = f.read()
                self.hasFlowerBRRES = True
                self.exportFlowerButton.setEnabled(True)
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error', f"Error reading BRRES: {e}")

    def exportFlower(self, data: bytes | None):
        path, _ = QFileDialog.getSaveFileName(self, "Save BRRES", "", "BRRES Files (*.brres)")
        if not path:
            return

        try:
            with open(path, "wb") as f:
                f.write(self.flowerResData)
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error', f"Error writing BRRES: {e}")
