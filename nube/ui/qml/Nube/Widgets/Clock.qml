import QtQuick 2.3
import Nube.Core 0.1

ThemedItem {
	property int updateInterval: 1000
	property string format: "HH:MM:ss"

	width: text.contentWidth
	height: text.contentHeight

	Text {
		id: text

		anchors.fill: parent

		color: theme.textColor
		font.family: theme.textFont
		font.pixelSize: theme.textSize

		function update() {
			text.text = Qt.formatDateTime( new Date(), format );
		}
	}

	Timer {
		interval: updateInterval
		repeat: true
		running: true
		triggeredOnStart: true
		onTriggered: text.update()
	}
}
