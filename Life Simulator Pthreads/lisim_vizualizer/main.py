

import sys
import os
from shutil import copyfile

from PyQt5.QtCore import *
from PyQt5.QtGui import *

from PyQt5.QtWidgets import *



class MainWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.time_step = 0

        self.MATRIX_SIZE = 60
        self.matrix = []

        self.painter = QPainter()

        self.clock_button = QPushButton('Clock', self)
        self.clock_button.move(640, 700)
        self.clock_button.resize(100, 40)
        self.clock_button.clicked.connect(self.update_matrix)

        self.time_label = QLabel(self)
        self.time_label.setText('Time step: ' + str(self.time_step))
        self.time_label.move(650, 630)
        self.time_label.resize(200, 40)

        
        scale = 10

        for i in range(self.MATRIX_SIZE):
            self.matrix.append([QRectF(380 + scale * j, scale * i, scale, scale) for j in range(self.MATRIX_SIZE)])
        
        if len(sys.argv) != 3:
            print('Usage [num_procs] [init_file]')
            sys.exit()
        
        self.init_filename = sys.argv[2]
        self.tmp_filename = 'tmp_file.txt'
        copyfile(self.init_filename, self.tmp_filename)


    def paintEvent(self, event):
        self.painter.begin(self)

        self.init_from_file()

        self.painter.end()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            sys.exit()

    def init_from_file(self):
        try:
            with open(self.tmp_filename, 'r') as f:

                init_file_strs = f.readlines()

                if len(init_file_strs) != self.MATRIX_SIZE:
                    print('Incorrect file')
                    sys.exit()

                i = 0

                for tmp_string in init_file_strs:
                

                    string = tmp_string[:-1]

                    if len(string) != self.MATRIX_SIZE:
                        print('Incorrect file')
                        sys.exit()
                    
                    for j in range(self.MATRIX_SIZE):
                        if string[j] == '0':
                            self.painter.setBrush(QBrush(Qt.white))
                            self.painter.drawRect(self.matrix[i][j])
                        elif string[j] == '1':
                            self.painter.setBrush(QBrush(Qt.green))
                            self.painter.drawRect(self.matrix[i][j])
                        else:
                            print('Unknown symbol')
                            sys.exit()
                    
                    i += 1
                    
                        
        except FileNotFoundError:
            print('Init file does not exist')
            sys.exit()

    def update_matrix(self):
        os.system(f'./lisim_perfrunner_60 1 {self.tmp_filename} ' + sys.argv[1]);

        self.time_step += 1
        self.time_label.setText('Time step: ' + str(self.time_step))

        self.update()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = MainWindow()

    win.showFullScreen()
    sys.exit(app.exec_())
