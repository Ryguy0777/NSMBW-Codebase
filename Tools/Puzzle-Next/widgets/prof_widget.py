import copy
import io
import json

from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout, QHBoxLayout, QPushButton, QTableWidget,
    QTableWidgetItem, QFileDialog, QHeaderView, QLabel, QLineEdit
)

import tools.profoverride as profile

__EMPTY__ = {"profver": 0, "entries": []}


class ProfileOverrideWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)

        self._profile_data = None
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

        self.headers = [
            "Tile Number",
            "Profile ID",
            "Offset X", "Offset Y", "Offset Z",
            "Scale X", "Scale Y",
            "Param",
            "Rail Color",
            "Rail SRT X", "Rail SRT Y"
        ]
        self.table.setColumnCount(len(self.headers))
        self.table.setHorizontalHeaderLabels(self.headers)

        # Make columns stretch to fill width, not user-resizable
        header = self.table.horizontalHeader()
        for col in range(len(self.headers)):
            header.setSectionResizeMode(col, QHeaderView.Stretch)
        header.setSectionsClickable(False)
        header.setSectionsMovable(False)

        self.table.setEditTriggers(QTableWidget.AllEditTriggers)

        profverLayout = QHBoxLayout()
        profverLayout.addWidget(QLabel("Prof Ver:"))
        self.profverEdit = QLineEdit("0")
        profverLayout.addWidget(self.profverEdit)
        #mainLayout.addLayout(profverLayout)

        # --- 4) Bottom row #2: Load/Save JSON + Load/Save .bin
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

        # Connect signals
        self.loadJsonButton.clicked.connect(self.loadJson)
        self.saveJsonButton.clicked.connect(self.saveJson)
        self.loadBinButton.clicked.connect(self.loadBinFile)
        self.saveBinButton.clicked.connect(self.saveBinFile)

        self.setLayout(mainLayout)

    def updateCurrentData(self):
        # Read profver from line edit
        try:
            self.currentData["profver"] = int(self.profverEdit.text())
        except ValueError:
            self.currentData["profver"] = 0

        # Build entries from table
        rowCount = self.table.rowCount()
        newEntries = []
        for rowIndex in range(rowCount):
            def getText(r, c):
                itm = self.table.item(r, c)
                return itm.text().strip() if itm else ""

            # Parse each cell
            try:
                tilenum = int(getText(rowIndex, 0))
            except ValueError:
                tilenum = 0
            try:
                profileid = int(getText(rowIndex, 1))
            except ValueError:
                profileid = 0

            try:
                offsetX = float(getText(rowIndex, 2))
            except ValueError:
                offsetX = 0.0
            try:
                offsetY = float(getText(rowIndex, 3))
            except ValueError:
                offsetY = 0.0
            try:
                offsetZ = float(getText(rowIndex, 4))
            except ValueError:
                offsetZ = 0.0

            try:
                scaleX = float(getText(rowIndex, 5))
            except ValueError:
                scaleX = 0.0
            try:
                scaleY = float(getText(rowIndex, 6))
            except ValueError:
                scaleY = 0.0

            settings = getText(rowIndex, 7)
            railcolor = getText(rowIndex, 8)

            try:
                railsrtX = float(getText(rowIndex, 9))
            except ValueError:
                railsrtX = 0.0
            try:
                railsrtY = float(getText(rowIndex, 10))
            except ValueError:
                railsrtY = 0.0

            entry = {
                "tilenum": tilenum,
                "profileid": profileid,
                "offset": [offsetX, offsetY, offsetZ],
                "scale": [scaleX, scaleY],
                "settings": settings
            }
            # Only add railcolor if not blank
            if railcolor:
                entry["railcolor"] = railcolor
            # Only add railsrt if not both zero
            if abs(railsrtX) > 1e-15 or abs(railsrtY) > 1e-15:
                entry["railsrt"] = [railsrtX, railsrtY]

            newEntries.append(entry)

        self.currentData["entries"] = newEntries

    def _load_into_table(self, data):
        self.currentData = data

        # Update profver
        profverVal = data.get("profver", 0)
        self.profverEdit.setText(str(profverVal))

        # Grab entries
        entries = data.get("entries", [])
        self.table.setRowCount(len(entries))

        for rowIndex, entry in enumerate(entries):
            # Extract each field with fallback
            tilenum = entry.get("tilenum", 0)
            profileid = entry.get("profileid", 0)
            offset = entry.get("offset", [0.0, 0.0, 0.0])
            offsetX, offsetY, offsetZ = offset if len(offset) == 3 else (0.0, 0.0, 0.0)
            scale = entry.get("scale", [0.0, 0.0])
            scaleX, scaleY = scale if len(scale) == 2 else (0.0, 0.0)
            settings = entry.get("settings", "")
            railcolor = entry.get("railcolor", "")
            railsrt = entry.get("railsrt", [0.0, 0.0])
            railsrtX, railsrtY = railsrt if len(railsrt) == 2 else (0.0, 0.0)

            # Populate table cells
            self._setTableItem(rowIndex, 0, str(tilenum))
            self._setTableItem(rowIndex, 1, str(profileid))
            self._setTableItem(rowIndex, 2, str(offsetX))
            self._setTableItem(rowIndex, 3, str(offsetY))
            self._setTableItem(rowIndex, 4, str(offsetZ))
            self._setTableItem(rowIndex, 5, str(scaleX))
            self._setTableItem(rowIndex, 6, str(scaleY))
            self._setTableItem(rowIndex, 7, settings)
            self._setTableItem(rowIndex, 8, railcolor)
            self._setTableItem(rowIndex, 9, str(railsrtX))
            self._setTableItem(rowIndex, 10, str(railsrtY))

    def to_bytes(self) -> bytes | None:
        self.updateCurrentData()

        if self.currentData == __EMPTY__:
            return None

        self._profile_data = profile.encode(self.currentData)

        return self._profile_data

    def load_from_bin(self, data: bytes | None):
        if data is None:
            self.table.setRowCount(0)
            self.currentData = copy.deepcopy(__EMPTY__)
            return

        self.currentData = profile.decode(io.BytesIO(data))
        self._load_into_table(self.currentData)

    # -------------------------------------------------------------------------
    # Row Management
    # -------------------------------------------------------------------------
    def addRow(self):
        """Add a new blank row at the bottom with default (zero/empty) values."""
        rowCount = self.table.rowCount()
        self.table.insertRow(rowCount)

        # Provide some defaults if you'd like
        defaults = ["0", "0", "0.0", "0.0", "0.0", "0.0", "0.0", "0x00000000", "", "", ""]
        for colIndex, val in enumerate(defaults):
            self._setTableItem(rowCount, colIndex, val)

    def removeRow(self):
        """Remove the currently selected row, if any."""
        selectedRow = self.table.currentRow()
        if selectedRow >= 0:
            self.table.removeRow(selectedRow)

    def _setTableItem(self, row, col, text):
        item = QTableWidgetItem(text)
        item.setTextAlignment(Qt.AlignCenter)
        self.table.setItem(row, col, item)

    # -------------------------------------------------------------------------
    # JSON Load/Save
    # -------------------------------------------------------------------------
    def loadJson(self):
        """Reads a JSON file, populates profver & the table of entries."""
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
        """Collects data from profver & table, writes JSON."""
        path, _ = QFileDialog.getSaveFileName(self, "Save JSON", "", "JSON Files (*.json)")
        if not path:
            return

        self.updateCurrentData()

        # Write JSON to disk
        try:
            with open(path, 'w') as f:
                json.dump(self.currentData, f, indent=4)
            print(f"Saved JSON to {path}")
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error',
                                          f"Error writing JSON: {e}")

    # -------------------------------------------------------------------------
    # Binary Load/Save
    # -------------------------------------------------------------------------
    def loadBinFile(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open .bin", "", "Binary Files (*.bin)")
        if not path:
            return

        self._load_into_table(profile.decode(open(path, 'rb')))

    def saveBinFile(self):
        path, _ = QFileDialog.getSaveFileName(self, "Save .bin", "", "Binary Files (*.bin)")
        if not path:
            return

        self.updateCurrentData()

        try:
            with open(path, "wb") as f:
                f.write(profile.encode(self.currentData))
            print(f"Saved .bin to {path}")
        except Exception as e:
            QtWidgets.QMessageBox.warning(None, 'Error',
                                          f"Error writing .bin: {e}")
