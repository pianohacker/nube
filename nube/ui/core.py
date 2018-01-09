from PyQt5.QtX11Extras import QX11Info
from PyQt5.QtCore import Qt, QEvent, QObject, pyqtProperty, pyqtSignal, pyqtRemoveInputHook
from PyQt5.QtGui import QColor, QGuiApplication
from PyQt5.QtQml import qmlAttachedPropertiesObject, qmlRegisterType, QQmlEngine
from PyQt5.QtQuick import QQuickItem, QQuickWindow

from . import platform

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

def stylePyqtProperty( type, origName ):
	name = '_' + origName

	signalName = origName + 'Changed' 
	signal = pyqtSignal( type, name = signalName )

	def getter( self ):
		return self.lookupProperty( origName )

	def setter( self, value ):
		print( f'setting {origName} of {self.parent().metaObject().className()} to {value}' )
		if getattr( self, name ) != value:
			setattr( self, name, value )
			self.notify( origName, value )

	return pyqtProperty( type, getter, setter, notify = signal ), signal

def styleNotifyChildren( obj, prop, value ):
	try:
		style = qmlAttachedPropertiesObject( StyleAttached, obj, False )
	except TypeError:
		style = None

	# If `receiveNotify` returns false, this Style has a value for this property
	if style and not style.receiveNotify( prop, value ):
		return

	for child in obj.children():
		if child is style: continue

		styleNotifyChildren( child, prop, value )

class StyleAttached( QObject ):
	margin, marginChanged = stylePyqtProperty( int, 'margin' )
	textColor, textColorChanged = stylePyqtProperty( QColor, 'textColor' )
	textFont, textFontChanged = stylePyqtProperty( str, 'textFont' )
	textSize, textSizeChanged = stylePyqtProperty( int, 'textSize' )

	def __init__( self, attachee, **kwargs ):
		super().__init__( attachee, **kwargs )

		self._margin = None
		self._textColor = None
		self._textFont = None
		self._textSize = None

		print( f'StyleAttached of {self.parent().metaObject().className()} has {self.margin} margin' )

	def notify( self, prop, value ):
		getattr( self, prop + 'Changed' ).emit( value )

		styleNotifyChildren(
			self.parent(),
			prop,
			value
		)

	def receiveNotify( self, prop, value ):
		if getattr( self, '_' + prop ) is None:
			getattr( self, prop + 'Changed' ).emit( value )
			return True
		else:
			return False

	def lookupProperty( self, prop ):
		obj = self.parent()
		style = self

		while obj:
			if style:
				print( f'style of {obj}: {obj.objectName()} looking for {prop}' )
				value = getattr( style, '_' + prop )
				if value: return value
			else:
				print( f'{obj}: {obj.objectName()} has no style' )

			obj = getattr( obj, "parentItem", lambda: None)() or obj.parent()
			if obj: style = qmlAttachedPropertiesObject( StyleAttached, obj, False )

		return DEFAULT_STYLE[ prop ]

DEFAULT_STYLE = dict(
	margin = 10,
	textColor = QColor( Qt.white ),
	textFont = "Exo 2",

	textSize = 20,
)

class Style( QObject ):
	def __init__( self, parent ):
		raise RuntimeError( 'Style should not be instantiated; use `Style.PROP: value` in other items' )

	@classmethod
	def qmlAttachedProperties( kls, attachee ):
		return StyleAttached( attachee )

class HUD( QQuickWindow ):
	@pyqtProperty( str )
	def showKey( self ):
		return self._showKey

	@showKey.setter
	def showKey( self, value ):
		if self._showKey != value:
			self._showKey = value
			self._keyGrabber.grabKey( self.winId(), self._showKey )

	def __init__( self, parent ):
		QQuickWindow.setDefaultAlphaBuffer( True )

		super().__init__( parent )

		screen = QGuiApplication.instance().primaryScreen()
		self._resizeTo( screen.geometry() )
		screen.geometryChanged.connect( self._resizeTo )

		self.setFlags(Qt.BypassWindowManagerHint)
		self.setColor(QColor( Qt.transparent ))

		self._showKey = None

		self._keyGrabber = platform.KeyGrabber( self )
	
	def grabbedKeyPressed( self ):
		self.show()
	
	def grabbedKeyReleased( self ):
		self.hide()

	def _resizeTo( self, rect ):
		self.setX( rect.x() )
		self.setY( rect.y() )
		self.setWidth( rect.width() )
		self.setHeight( rect.height() )

def register_types():
	qmlRegisterType( HUD, 'Nube.Core', 0,1, 'HUD' )
	qmlRegisterType( Style, 'Nube.Core', 0,1, 'Style', attachedProperties = StyleAttached )
