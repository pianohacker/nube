from PyQt5.QtCore import QAbstractNativeEventFilter, QCoreApplication, QObject
from PyQt5.QtX11Extras import QX11Info
import sip
import xcffib
from xcffib import xproto

class KeyGrabber( QAbstractNativeEventFilter ):
	def __init__( self, parent ):
		super().__init__()

		self.parent = parent

		QCoreApplication.instance().installNativeEventFilter( self )
		
		# First, we need to convert Qt's SIP-wrapped xcb_connection_t to an xcffib Connection.
		self.connection = xcffib.wrap( sip.unwrapinstance( QX11Info.connection() ) )
		self.connection.ensure_connected()

	def nativeEventFilter( self, eventType, message ):
		# To filter out the key events we care about, we:
		#
		# 1. Convert the event into an xcffib `xcb_generic_event_t *`.
		assert( eventType == b'xcb_generic_event_t' )

		event = xcffib.ffi.cast( 'xcb_generic_event_t *', message.__int__() )

		# 2. Ensure that the event is actually a known event, to avoid crashing xcffib.
		if event.response_type > len( xproto._events ):
			return False, 0

		# 3. Convert it to a specific event and check its type.
		event = self.connection.hoist_event( event )

		if isinstance( event, xproto.KeyPressEvent ) and event.detail == 133:
			self.parent.grabbedKeyPressed()

			return True, 0
		elif isinstance( event, xproto.KeyReleaseEvent ) and event.detail == 133:
			self.parent.grabbedKeyReleased()

			return True, 0
		else:
			return False, 0

	def grabKey( self, winId, key ):
		# Then, we can actually grab the key.
		xprotoExtension = xproto.xprotoExtension( self.connection )

		xprotoExtension.GrabKey(
			key = 133,
			modifiers = xproto.ModMask.Any,
			grab_window = QX11Info.appRootWindow(),
			# This is False because we want to steal the keyboard events from the root window.
			owner_events = False,
			# The name `Async` is misleading here; all it means is that we don't freeze the entire
			# server with our grab.
			pointer_mode = xproto.GrabMode.Async,
			keyboard_mode = xproto.GrabMode.Async
		)
