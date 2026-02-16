import copy
import io
import json
import re

from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout, QHBoxLayout, QPushButton, QTableWidget,
    QTableWidgetItem, QFileDialog, QLabel, QLineEdit, QHeaderView
)

import tools.anim as anim

__EMPTY__ = {"entries": []}


class AnimTileWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._anim_data = None

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

        self.headers = ["Tile Number", "Is Reversed", "Frame Delays"]
        self.table.setColumnCount(len(self.headers))
        self.table.setHorizontalHeaderLabels(self.headers)

        # Make columns fill all width evenly, not user-resizable
        header = self.table.horizontalHeader()
        for col in range(len(self.headers)):
            header.setSectionResizeMode(col, QHeaderView.Stretch)
        header.setSectionsClickable(False)
        header.setSectionsMovable(False)

        self.table.setEditTriggers(QTableWidget.AllEditTriggers)

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

        self.setLayout(mainLayout)

    def _load_into_table(self, data):
        self.currentData = data

        # Populate the table
        entries = data.get("entries", [])
        self.table.setRowCount(len(entries))

        for rowIndex, entry in enumerate(entries):
            tilenum = entry.get("tilenum", 0)
            reverse = entry.get("reverse", 0)

            delayList = ''
            delays = entry.get("delays", [])
            for index, val in enumerate(delays):
                delayList += str(val)
                # Add comma separator if we're not at the end of the list
                if (index+1 < len(delays)):
                    delayList += ', '

            self._setTableItem(rowIndex, 0, str(tilenum))
            self._setTableItem(rowIndex, 1, str(reverse))
            self._setTableItem(rowIndex, 2, str(delayList))

    def updateCurrentData(self):
        rowCount = self.table.rowCount()
        entries = []
        for rowIndex in range(rowCount):
            def textOrZero(r, c):
                item = self.table.item(r, c)
                return item.text().strip() if item else "0x0"
            def textOrFalse(r, c):
                item = self.table.item(r, c)
                return item.text().strip() if item else "False"

            tilenum = hex(int(textOrZero(rowIndex, 0), 16))
            reverse = str(textOrFalse(rowIndex, 1))
            delays  = re.findall(r'0x\w+', textOrZero(rowIndex, 2))

            entries.append({
                "tilenum": tilenum,
                "reverse": reverse,
                "delays": delays
            })

        self.currentData["entries"] = entries

    def to_bytes(self) -> bytes | None:
        self.updateCurrentData()

        if self.currentData == __EMPTY__:
            return None

        self._anim_data = anim.encode(self.currentData)

        return self._anim_data

    def load_from_bin(self, data: bytes | None):
        if data is None:
            self.table.setRowCount(0)
            self.currentData = copy.deepcopy(__EMPTY__)
            return

        self.currentData = anim.decode(io.BytesIO(data))
        self._load_into_table(self.currentData)

    # -------------------------------------------------------------------------
    # Row Management: Add/Remove
    # -------------------------------------------------------------------------
    def addRow(self):
        """Append a new row to the bottom of the table with default values."""
        rowCount = self.table.rowCount()
        self.table.insertRow(rowCount)
        # Optional: set defaults
        self._setTableItem(rowCount, 0, "0x0")  # tilenum
        self._setTableItem(rowCount, 1, "False") # reverse
        self._setTableItem(rowCount, 2, "0x0") # delays

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

        self._load_into_table(anim.decode(open(path, 'rb')))

    def saveBinFile(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save .bin", "", "Binary Files (*.bin)")
        if not path:
            return

        self.updateCurrentData()

        try:
            with open(path, "wb") as f:
                f.write(anim.encode(self.currentData))
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

