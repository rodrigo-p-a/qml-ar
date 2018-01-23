import QtQuick 2.0
import AR 1.0

Item {
    id: root
    anchors.fill: parent
    property real scaleDots: 1.0
    property int last_update: new Date().getTime()
    property int min_delta_ms: 50
    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: doPaint()

        function doPaint() {
            if(!root.visible) return;
            if(new Date().getTime() - last_update < min_delta_ms) return;
            var markers = AR.markers;
            var ctx = getContext("2d");
            ctx.reset();
            ctx.fillStyle = "green";
            ctx.globalAlpha = 0.2;
            for(var i = 0; i < markers.length; i += 4) {
                var marker_tl = markers[i];
                var marker_br = markers[i + 1];
                ctx.beginPath();
                ctx.moveTo(markers[i + 0].x * root.scaleDots, markers[i + 0].y * root.scaleDots);

                ctx.lineTo(markers[i + 1].x * root.scaleDots, markers[i + 1].y * root.scaleDots);
                ctx.lineTo(markers[i + 2].x * root.scaleDots, markers[i + 2].y * root.scaleDots);
                ctx.lineTo(markers[i + 3].x * root.scaleDots, markers[i + 3].y * root.scaleDots);
                ctx.lineTo(markers[i + 0].x * root.scaleDots, markers[i + 0].y * root.scaleDots);
                ctx.fill();
            }
            canvas.requestPaint()
        }
        Connections {
            target: AR
            onNewMarkers: canvas.doPaint()
        }
    }
}