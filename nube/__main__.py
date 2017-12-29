import signal
import sys

from PyQt5.QtCore import pyqtProperty, QObject, QUrl
from PyQt5.QtQml import qmlRegisterType, QQmlComponent, QQmlEngine
from PyQt5.QtGui import QGuiApplication

def main():
	# Set up correct Ctrl-C behavior.
	signal.signal(signal.SIGINT, signal.SIG_DFL)
	
	app = QGuiApplication( sys.argv )
	engine = QQmlEngine()
	component = QQmlComponent( engine )
	component.loadUrl( QUrl( 'nube.qml' ) )

	window = component.create()
	window.show()

	app.exec_()

if __name__ == '__main__':
	main()
