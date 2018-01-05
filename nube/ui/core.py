from PyQt5.QtCore import Qt, QObject, pyqtProperty, pyqtSignal, pyqtRemoveInputHook
from PyQt5.QtGui import QColor
from PyQt5.QtQml import qmlRegisterType
from PyQt5.QtQuick import QQuickItem, QQuickWindow

def dataPyqtProperty( type, origName ):
	name = '_' + origName

	signalName = origName + 'Changed' 
	signal = pyqtSignal( type, name = signalName )

	def getter( self ):
		return getattr( self, name )

	def setter( self, value ):
		if value != getter( self ):
			getattr( self, signalName ).emit( value )

		setattr( self, name, value )

	return pyqtProperty( type, getter, setter, notify = signal ), signal

def themePyqtProperty( type, origName ):
	name = '_' + origName

	signalName = origName + 'Changed' 
	signal = pyqtSignal( type, name = signalName )

	def getter( self ):
		return self.lookupThemeProperty( origName )

	def setter( self, value ):
		if getattr( self, name ) != value:
			setattr( self, name, value )
			self.notify( origName, value )

	return pyqtProperty( type, getter, setter, notify = signal ), signal

def themeNotifyChildren( object, prop, value, ignore ):
	for child in getattr( object, 'childItems', object.children )():
		if child is ignore: continue

		if hasattr( child, 'receiveThemeNotify' ):
			child.receiveThemeNotify( prop, value )
		elif isinstance( child, QQuickItem ):
			themeNotifyChildren( child, prop, value, ignore )

class Theme( QObject ):
	textColor, textColorChanged = themePyqtProperty( QColor, 'textColor' )
	textFont, textFontChanged = themePyqtProperty( str, 'textFont' )
	textSize, textSizeChanged = themePyqtProperty( int, 'textSize' )

	def __init__( self, parent, **kwargs ):
		self._textColor = None
		self._textFont = None
		self._textSize = None

		super().__init__( parent, **kwargs )

	def notify( self, prop, value ):
		getattr( self, prop + 'Changed' ).emit( value )

		themeNotifyChildren( self.parent(), prop, value, ignore = self )

	def receiveThemeNotify( self, prop, value ):
		if getattr( self, '_' + prop ) is not None:
			return

		self.notify( prop, value )

	def lookupThemeProperty( self, prop ):
		ownValue = getattr( self, '_' + prop )

		if ownValue is not None:
			return ownValue

		parent = self.parent().parent()

		while not hasattr( parent, 'lookupThemeProperty' ):
			parent = parent.parent()

		return parent.lookupThemeProperty( prop )

def Themed( parent ):
	class ThemedParent( parent ):
		@pyqtProperty( Theme, constant = True )
		def theme( self ):
			return self._theme

		@theme.getter
		def theme( self ):
			return self._theme

		def __init__( self, parent ):
			super().__init__( parent )

			self._theme = Theme( self )

		def receiveThemeNotify( self, prop, value ):
			self._theme.receiveThemeNotify( prop, value )

		def lookupThemeProperty( self, prop ):
			return self._theme.lookupThemeProperty( prop )

	return ThemedParent

class HUD( Themed( QQuickWindow ) ):
	def __init__( self, parent ):
		super().__init__( parent )
	
		self.setX(480)
		self.setY(270)
		self.setWidth(960)
		self.setHeight(540)
		self.setFlags(Qt.BypassWindowManagerHint)
		self.setColor(QColor( Qt.transparent ))

		self._theme.pyqtConfigure(
			textColor = QColor( Qt.white ),
			textFont = "Exo 2",
			textSize = 20
		)

class ThemedItem( Themed( QQuickItem ) ):
	pass

def register_types():
	qmlRegisterType( HUD, 'Nube.Core', 0,1, 'HUD' )
	qmlRegisterType( Theme, 'Nube.Core', 0,1, 'Theme' )
	qmlRegisterType( ThemedItem, 'Nube.Core', 0,1, 'ThemedItem' )
