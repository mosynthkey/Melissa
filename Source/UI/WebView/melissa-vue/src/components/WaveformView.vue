<template>
    <div ref="waveformContainer" class="waveform-view">
        <canvas ref="waveformCanvas" :width="canvasWidth" height="100" @mousemove="handleMouseMove"
            @mouseleave="handleMouseLeave" @click="handleClick"></canvas>
        <canvas ref="timelineCanvas" :width="canvasWidth" height="20"></canvas>
        <div v-if="hoverPosition >= 0" :style="tooltipStyle" class="tooltip">
            {{ getPositionAsString(hoverPosition / numStrips) }}
        </div>
        <div v-if="loopStartRatio >= 0 && loopEndRatio > loopStartRatio" :style="selectionStyle" class="selection">
            <div class="handle start-handle" @mousedown="startDrag('start', $event)"></div>
            <div class="handle end-handle" @mousedown="startDrag('end', $event)"></div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const waveformContainer = ref<HTMLDivElement | null>(null);
const waveformCanvas = ref<HTMLCanvasElement | null>(null);
const timelineCanvas = ref<HTMLCanvasElement | null>(null);

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

const songLengthMs = ref(0);

const loopStartRatio = ref(0.2); // 初期値
const loopEndRatio = ref(0.8); // 初期値
const dragging = ref<null | 'start' | 'end'>(null);

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
    context.font = '10px Arial';
    context.textAlign = 'center';

    for (let i = 1; i <= totalMinutes; i++) {
        const x = (i * 60000 / songLengthMs.value) * width;

        // 文字がキャンバスの端にかかる場合は描画しない
        if (x < 10 || x > width - 10) continue;

        context.fillRect(x, 0, 1, 5);
        context.fillText(`${i}:00`, x, 15);
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
    hoverPosition.value = Math.floor((x / canvasWidth.value) * numStrips.value);

    if (dragging.value) {
        const ratio = x / canvasWidth.value;

        if (dragging.value === 'start') {
            const startRatio = Math.min(Math.max(ratio, 0), loopEndRatio.value - 0.01);
            // loopStartRatio.value = startRatio;
            excuteCommand('SetLoopStartValue', startRatio);
        } else if (dragging.value === 'end') {
            const endRatio = Math.max(Math.min(ratio, 1), loopStartRatio.value + 0.01);
            //loopEndRatio.value = endRatio;
            excuteCommand('SetLoopEndValue', endRatio);
        }
    }

    drawWaveform();
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

const startDrag = (handle: 'start' | 'end', event: MouseEvent) => {
    event.preventDefault(); // ドラッグ開始時のデフォルト動作を防ぐ
    dragging.value = handle;
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
        left: `${startX + 20}px`, // 左のパディングを考慮
        width: `${endX - startX}px`,
        top: '0',
        height: '100px',
        backgroundColor: 'rgba(0, 123, 255, 0.3)',
        border: '1px solid rgba(0, 123, 255, 0.7)',
        pointerEvents: 'none',
    };
});

onMounted(() => {
    updateContainerWidth();
    loadWaveform();
    playbackUpdateInterval = setInterval(updatePlaybackPosition, 500);

    resizeObserver = new ResizeObserver(updateContainerWidth);
    if (waveformContainer.value) {
        resizeObserver.observe(waveformContainer.value);
    }

    window.addEventListener('resize', updateContainerWidth);

    notificationToken = window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        const message = objectFromBackend[0];
        if (message == 'loopPosChanged') {
            loopStartRatio.value = objectFromBackend[2];
            loopEndRatio.value = objectFromBackend[4];
        }
    });

    getCurrentValue("getLengthMSec").then((length: number) => {
        songLengthMs.value = length;
    });

    window.addEventListener('mouseup', handleMouseUp);
});

onUnmounted(() => {
    clearInterval(playbackUpdateInterval);
    if (resizeObserver && waveformContainer.value) {
        resizeObserver.unobserve(waveformContainer.value);
    }
    window.removeEventListener('resize', updateContainerWidth);

    window.__JUCE__.backend.removeEventListener(notificationToken);
    window.removeEventListener('mouseup', handleMouseUp);
});

// @ts-ignore

</script>

<style scoped>
.waveform-view {
    position: relative;
    width: 100%;
    height: 120px;
    /* Increased height to accommodate timeline */
    cursor: pointer;
    padding: 0 20px;
    /* Increased height to accommodate timeline */
    box-sizing: border-box;
    /* パディングを要素の幅に含める */
}

canvas {
    display: block;
    /* インライン要素による余白を削除 */
    margin: 0 auto;
    /* 水平方向に中央揃え */
}

.tooltip {
    position: absolute;
    background-color: rgba(0, 0, 0, 0.7);
    color: white;
    padding: 2px 5px;
    border-radius: 3px;
    pointer-events: none;
}

.selection {
    position: absolute;
    top: 0;
    height: 100px;
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
</style>