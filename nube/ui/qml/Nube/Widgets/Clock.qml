import QtQuick 2.3

StyledText {
	id: text
	property int updateInterval: 1000
	property string format: "HH:mm:ss"

	function update() {
		text.text = Qt.formatDateTime( new Date(), format );
	}

	Timer {
		interval: updateInterval
		repeat: true
		running: true
		triggeredOnStart: true
		onTriggered: text.update()
	}
}
