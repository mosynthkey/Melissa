<template>
    <div ref="waveformContainer" class="waveform-view">
        <div class="waveform-container">
            <canvas ref="waveformCanvas" :width="canvasWidth" :height="waveformHeight" @mousemove="handleMouseMove"
                @mouseleave="handleMouseLeave" @click="handleClick"></canvas>
            <div v-if="loopStartRatio >= 0 && loopEndRatio > loopStartRatio" :style="selectionStyle" class="selection">
                <div class="handle start-handle" @mousedown="startDrag('start', $event)"
                    @touchstart="startDrag('start', $event)"></div>
                <div class="handle end-handle" @mousedown="startDrag('end', $event)"
                    @touchstart="startDrag('end', $event)">
                </div>
            </div>
            <div v-for="marker in markers" :key="marker.position" :style="getMarkerStyle(marker)" class="marker">
                <div class="marker-head" @click.stop="setPlayPositionToMarker(marker)"></div>
            </div>
        </div>
        <div class="timeline-container">
            <div v-for="label in timeLabels" :key="label.time" :style="label.style" class="time-label">
                {{ label.text }}
            </div>
        </div>
        <div v-if="hoverPosition >= 0" :style="tooltipStyle" class="tooltip">
            {{ getPositionAsString(hoverPosition / numStrips) }}
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const waveformContainer = ref<HTMLDivElement | null>(null);
const waveformCanvas = ref<HTMLCanvasElement | null>(null);

const excuteCommand = Juce.getNativeFunction("excuteCommand");
const requestWaveform = Juce.getNativeFunction("requestWaveform");
const getCurrentValue = Juce.getNativeFunction("getCurrentValue");

const waveformData = ref<number[]>([]);
const stripWidth = 3;
const stripInterval = 1;
const playbackPosition = ref(0);
const containerWidth = ref(0);
const canvasWidth = ref(0);
const hoverPosition = ref(-1);

const loopStartRatio = ref(0.2); // 初期値
const loopEndRatio = ref(0.8); // 初期値
const dragging = ref<null | 'start' | 'end'>(null);

const waveformHeight = ref(60); // 波形の高さを60pxに設定
const timelineHeight = ref(20); // タイムラインの高さを30pxに設定

const timeLabels = ref<{ time: number; text: string; style: any }[]>([]);
const songLengthMs = ref(0);

const loadWaveform = () => {
    requestWaveform().then((waveformAsJson: string) => {
        waveformData.value = JSON.parse(waveformAsJson);
        drawWaveform();
    });
};

function resampleData(inputData: number[], numOutputData: number): number[] {
    const numInputData = inputData.length;
    const outputData: number[] = [];
    const step = (numInputData - 1) / (numOutputData - 1);

    for (let i = 0; i < numOutputData; i++) {
        const position = i * step;
        const index = Math.floor(position);
        const fraction = position - index;

        if (index >= numInputData - 1) {
            outputData.push(inputData[numInputData - 1]);
        } else {
            const value = inputData[index] * (1 - fraction) + inputData[index + 1] * fraction;
            outputData.push(value);
        }
    }

    return outputData;
}

const drawWaveform = () => {
    if (!waveformCanvas.value || waveformData.value.length === 0) return;

    const context = waveformCanvas.value.getContext('2d');
    if (!context) return;

    const { height } = waveformCanvas.value;
    context.clearRect(0, 0, canvasWidth.value, height);

    const numStrips = Math.floor(canvasWidth.value / (stripWidth + stripInterval));
    const resampledData = resampleData(waveformData.value, numStrips);

    resampledData.forEach((amplitude, index) => {
        const x = index * (stripWidth + stripInterval);
        const h = amplitude * height;

        if (index === hoverPosition.value) {
            context.fillStyle = 'rgba(255, 255, 0, 1.0)'; // ホバー時は黄色
        } else if (index / numStrips <= playbackPosition.value) {
            context.fillStyle = 'rgba(255, 0, 0, 1.0)'; // 再生済みは赤
        } else {
            context.fillStyle = 'rgba(0, 123, 255, 1.0)'; // 未再生は青
        }

        context.fillRect(x, height - h, stripWidth, h);
    });

    // After drawing the waveform, call drawTimeline
    drawTimeline();
};

const drawTimeline = () => {
    if (!timelineCanvas.value || songLengthMs.value === 0) return;

    const context = timelineCanvas.value.getContext('2d');
    if (!context) return;

    const { width, height } = timelineCanvas.value;
    context.clearRect(0, 0, width, height);

    const totalMinutes = Math.floor(songLengthMs.value / 60000);
    const minuteWidth = width / (songLengthMs.value / 60000);

    context.fillStyle = 'white';
    context.font = '14px Arial'; // フォントサイズを大きくする
    context.textAlign = 'center';

    for (let i = 1; i <= totalMinutes; i++) {
        const x = (i * 60000 / songLengthMs.value) * width;

        // 文字がキャンバスの端にかかる場合は描画しない
        if (x < 10 || x > width - 10) continue;

        context.fillRect(x, 0, 1, 4);
        context.fillText(`${i}:00`, x, 20); // y座標を調整
    }
};

const updateContainerWidth = () => {
    if (waveformContainer.value) {
        containerWidth.value = waveformContainer.value.offsetWidth;
        canvasWidth.value = containerWidth.value - 40; // マージンを考慮
        drawWaveform();
    }
};

const updatePlaybackPosition = () => {
    getCurrentValue("getPlayingPosRatio").then((position: number) => {
        playbackPosition.value = position;
        drawWaveform();
    });
};

const numStrips = computed(() => {
    return Math.floor(canvasWidth.value / (stripWidth + stripInterval));
});

const tooltipStyle = computed(() => {
    if (!waveformContainer.value || hoverPosition.value < 0) return {};
    const x = (hoverPosition.value / numStrips.value) * canvasWidth.value;
    return {
        position: 'absolute',
        left: `${x + 20}px`, // 左のパディングを考慮
        top: '100%', // 波形の下に配置
        transform: 'translateX(-50%)', // 水平方向の中央揃え
        backgroundColor: 'rgba(0, 0, 0, 0.7)',
        color: 'white',
        padding: '2px 5px',
        borderRadius: '3px',
        pointerEvents: 'none',
        whiteSpace: 'nowrap', // テキストを1行に保つ
    };
});

const getPositionAsString = (position: number): string => {
    const totalSeconds = Math.floor(songLengthMs.value * position / 1000);
    const hours = Math.floor(totalSeconds / 3600);
    const minutes = Math.floor((totalSeconds % 3600) / 60);
    const seconds = totalSeconds % 60;

    if (hours > 0) {
        return `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
    } else {
        return `${minutes}:${seconds.toString().padStart(2, '0')}`;
    }
};

const handleMouseMove = (event: MouseEvent) => {
    event.preventDefault(); // ドラッグ中のデフォルト動作を防ぐ
    if (!waveformContainer.value) return;
    const rect = waveformContainer.value.getBoundingClientRect();
    const x = event.clientX - rect.left - 20; // 左のパディングを考慮
    handleMove(x);
};

const handleMouseLeave = () => {
    hoverPosition.value = -1;
    drawWaveform();
};

const handleClick = (event: MouseEvent) => {
    if (dragging.value) return; // ドラッグ中は再生位置を変更しない
    if (!waveformContainer.value) return;
    const rect = waveformContainer.value.getBoundingClientRect();
    const x = event.clientX - rect.left - 20; // 左のパディングを考慮
    const position = x / canvasWidth.value;
    excuteCommand("PlaybackPositionValue", position);
};

const startDrag = (handle: 'start' | 'end', event: MouseEvent | TouchEvent) => {
    event.preventDefault(); // ドラッグ開始時のデフォルト動作を防ぐ
    dragging.value = handle;
    if (event.type === 'touchstart') {
        window.addEventListener('touchmove', handleTouchMove, { passive: false });
        window.addEventListener('touchend', handleTouchEnd);
    }
};

const handleTouchMove = (event: TouchEvent) => {
    event.preventDefault();
    if (!waveformContainer.value) return;
    const touch = event.touches[0];
    const rect = waveformContainer.value.getBoundingClientRect();
    const x = touch.clientX - rect.left - 20;
    handleMove(x);
};

const handleMove = (x: number) => {
    hoverPosition.value = Math.floor((x / canvasWidth.value) * numStrips.value);

    if (dragging.value) {
        const ratio = x / canvasWidth.value;

        if (dragging.value === 'start') {
            const startRatio = Math.min(Math.max(ratio, 0), loopEndRatio.value - 0.01);
            excuteCommand('SetLoopStartValue', startRatio);
        } else if (dragging.value === 'end') {
            const endRatio = Math.max(Math.min(ratio, 1), loopStartRatio.value + 0.01);
            excuteCommand('SetLoopEndValue', endRatio);
        }
    }

    drawWaveform();
};

const handleTouchEnd = (event: TouchEvent) => {
    event.preventDefault();
    dragging.value = null;
    window.removeEventListener('touchmove', handleTouchMove);
    window.removeEventListener('touchend', handleTouchEnd);
};

const handleMouseUp = (event: MouseEvent) => {
    if (dragging.value) {
        dragging.value = null;
        event.preventDefault(); // ドラッグ終了時のデフォルト動作を防ぐ
    }
};

let playbackUpdateInterval: number;
let resizeObserver: ResizeObserver;
let notificationToken = ref(null);

const selectionStyle = computed(() => {
    const startX = loopStartRatio.value * canvasWidth.value;
    const endX = loopEndRatio.value * canvasWidth.value;
    return {
        position: 'absolute',
        left: `${startX}px`,
        width: `${endX - startX}px`,
        top: '0',
        height: '100%',
        backgroundColor: 'rgba(0, 123, 255, 0.3)',
        border: '1px solid rgba(0, 123, 255, 0.7)',
        pointerEvents: 'none',
    };
});

const resetWaveform = () => {
    waveformData.value = [];
    playbackPosition.value = 0;
    hoverPosition.value = -1;
    loopStartRatio.value = 0;
    loopEndRatio.value = 1;
    songLengthMs.value = 0;
    loadWaveform();
    getCurrentValue("getLengthMSec").then((length: number) => {
        songLengthMs.value = length;
        updateTimeLabels();
    });
};

const updateTimeLabels = () => {
    const labels: { time: number; text: string; style: any }[] = [];
    const totalMinutes = Math.floor(songLengthMs.value / 60000);
    const containerWidth = canvasWidth.value;

    for (let i = 0; i <= totalMinutes; i++) {
        const x = (i * 60000 / songLengthMs.value) * containerWidth;
        labels.push({
            time: i * 60,
            text: `${i}:00`,
            style: {
                position: 'absolute',
                left: `${x}px`,
                transform: 'translateX(-50%)',
            }
        });
    }

    // 重なりを避けるために間引く
    const visibleLabels = [];
    let prevLabelRight = -Infinity;

    for (let i = 0; i < labels.length; i++) {
        const label = labels[i];
        const labelLeft = parseFloat(label.style.left);
        if (labelLeft - prevLabelRight > 40) {
            visibleLabels.push(label);
            prevLabelRight = labelLeft + 20; // ラベルの幅を考慮
        }
    }

    timeLabels.value = visibleLabels;
};

const markers = ref<{ position: number; color: string }[]>([]);

const formatColor = (colorString: string): string => {
    // "0xffrrggbb" 形式の文字列から RGB 値を抽出
    const colorValue = parseInt(colorString.slice(2), 16);
    const r = (colorValue >> 16) & 0xFF;
    const g = (colorValue >> 8) & 0xFF;
    const b = colorValue & 0xFF;
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
};

const updateMarkers = (markersData: string) => {
    try {
        const parsedMarkers = JSON.parse(markersData);
        markers.value = parsedMarkers.map((marker: any) => ({
            position: marker.position,
            color: formatColor(marker.colour)
        }));
    } catch (error) {
        console.error('マーカーデータの解析に失敗しました:', error);
    }
};

const getMarkerStyle = (marker: { position: number; color: string }) => {
    const x = marker.position * canvasWidth.value;
    return {
        position: 'absolute',
        left: `${x}px`,
        top: '-16px', // 波形の上にはみ出すように調整
        width: '3px', // マーカーを3pxに太く
        height: 'calc(100% + 16px)', // 高さを増やして波形の上にはみ出すように
        backgroundColor: marker.color,
    };
};

const setPlayPositionToMarker = (marker: { position: number; color: string }) => {
    excuteCommand("PlaybackPositionValue", marker.position);
};

onMounted(() => {
    updateContainerWidth();
    loadWaveform();
    getCurrentValue("getLengthMSec").then((length: number) => {
        songLengthMs.value = length;
        updateTimeLabels();
    });
    getCurrentValue("getMarkers").then((markersData) => {
        updateMarkers(markersData);
    }).catch((error) => {
        console.error('マーカーデータの取得に失敗しました:', error);
    });
    playbackUpdateInterval = setInterval(updatePlaybackPosition, 500);

    resizeObserver = new ResizeObserver(() => {
        updateContainerWidth();
        updateTimeLabels();
    });
    if (waveformContainer.value) {
        resizeObserver.observe(waveformContainer.value);
    }

    notificationToken = window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        const message = objectFromBackend[0];
        if (message == 'loopPosChanged') {
            loopStartRatio.value = objectFromBackend[2];
            loopEndRatio.value = objectFromBackend[4];
        }
        else if (message == 'songChanged') {
            resetWaveform();
        }
        else if (message === 'markerUpdated') {
            updateMarkers(objectFromBackend[1]);
        }
    });
});

onUnmounted(() => {
    clearInterval(playbackUpdateInterval);
    if (resizeObserver && waveformContainer.value) {
        resizeObserver.unobserve(waveformContainer.value);
    }
    window.removeEventListener('resize', updateContainerWidth);

    window.__JUCE__.backend.removeEventListener(notificationToken);
    window.removeEventListener('mouseup', handleMouseUp);
    window.removeEventListener('touchend', handleTouchEnd);
});

// @ts-ignore

</script>

<style scoped>
.waveform-view {
    position: relative;
    width: 100%;
    height: 90px;
    /* 波形(60px) + タイムライン(30px) */
    cursor: pointer;
    padding: 0 20px;
    box-sizing: border-box;
}

.waveform-container {
    position: relative;
    height: 60px;
    /* 波形の高さ */
}

.timeline-container {
    position: relative;
    height: 30px;
    /* タイムラインの高さ */
}

.time-label {
    position: absolute;
    font-size: 12px;
    color: white;
    bottom: 5px;
    /* 下端から少し上に配置 */
    text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.8);
    /* 読みやすさのために影をつける */
}

.marker {
    position: absolute;
    width: 3px;
    /* マーカーを3pxに太く */
    background-color: red;
    /* デフォルトの色 */
    pointer-events: none;
}

.marker-head {
    position: absolute;
    top: 0;
    left: 50%;
    transform: translateX(-50%);
    width: 10px;
    height: 18px;
    border-radius: 6px;
    background-color: inherit;
    cursor: pointer;
}

canvas {
    display: block;
    margin: 0 auto;
}

.selection {
    position: absolute;
    top: 0;
    height: 100%;
    background-color: rgba(0, 123, 255, 0.3);
    border: 1px solid rgba(0, 123, 255, 0.7);
    pointer-events: none;
}

.handle {
    position: absolute;
    top: 0;
    width: 10px;
    height: 100%;
    background-color: rgba(0, 123, 255, 0.7);
    cursor: ew-resize;
    pointer-events: auto;
}

.start-handle {
    left: 0;
}

.end-handle {
    right: 0;
}

.tooltip {
    position: absolute;
    background-color: rgba(0, 0, 0, 0.7);
    color: white;
    padding: 2px 5px;
    border-radius: 3px;
    pointer-events: none;
}
</style>