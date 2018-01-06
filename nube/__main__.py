from os import path
import signal
import sys

from PyQt5.QtCore import pyqtProperty, QObject, QUrl
from PyQt5.QtQml import qmlRegisterType, QQmlComponent, QQmlEngine
from PyQt5.QtGui import QGuiApplication

from .ui import core

def main():
	# Set up correct Ctrl-C behavior.
	signal.signal(signal.SIGINT, signal.SIG_DFL)
	
	app = QGuiApplication( sys.argv )
	engine = QQmlEngine()
	engine.addImportPath( path.join( path.dirname( __file__ ), 'ui', 'qml' ) )
	core.register_types()

	component = QQmlComponent( engine )
	component.loadUrl( QUrl( '/home/feoh3/.config/nube/hud.qml' ) )

	if component.isError():
		print( "\n".join( error.toString() for error in component.errors() ) )
	else:
		window = component.create()

		app.exec_()

if __name__ == '__main__':
	main()
