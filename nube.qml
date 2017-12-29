import QtQuick 2.3
import QtQuick.Window 2.3

Window {
	x: 480
	y: 270
	width: 960
	height: 540
	flags: Window.BypassWindowManagerHint

	color: "transparent"

	Text {
		font.family: "Exo 2"
		font.pixelSize: 32
		color: "white"
		text: "12:30"
	}
}
