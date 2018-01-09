import QtQuick 2.3
import Nube.Core 0.1

Text {
	id: text
	width: text.contentWidth
	height: text.contentHeight

	anchors.margins: Style.margin

	color: Style.textColor
	font.family: Style.textFont
	font.pixelSize: Style.textSize
}
