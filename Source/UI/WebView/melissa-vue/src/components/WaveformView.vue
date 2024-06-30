<template>
    <div ref="waveformContainer" class="waveform-view" @mousemove="handleMouseMove" @mouseleave="handleMouseLeave"
        @click="handleClick">
        <canvas ref="waveformCanvas" :width="canvasWidth" height="100"></canvas>
        <canvas ref="timelineCanvas" :width="canvasWidth" height="20"></canvas>
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
    context.font = '10px Roboto';
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
    if (!waveformContainer.value) return;
    const rect = waveformContainer.value.getBoundingClientRect();
    const x = event.clientX - rect.left - 20; // 左のパディングを考慮
    hoverPosition.value = Math.floor((x / canvasWidth.value) * numStrips.value);
    drawWaveform();
};

const handleMouseLeave = () => {
    hoverPosition.value = -1;
    drawWaveform();
};

const handleClick = (event: MouseEvent) => {
    if (!waveformContainer.value) return;
    const rect = waveformContainer.value.getBoundingClientRect();
    const x = event.clientX - rect.left - 20; // 左のパディングを考慮
    const position = x / canvasWidth.value;
    excuteCommand("PlaybackPositionValue", position);
};

let playbackUpdateInterval: number;
let resizeObserver: ResizeObserver;
let dataSourceToken = ref(null);

onMounted(() => {
    updateContainerWidth();
    loadWaveform();
    playbackUpdateInterval = setInterval(updatePlaybackPosition, 500);

    resizeObserver = new ResizeObserver(updateContainerWidth);
    if (waveformContainer.value) {
        resizeObserver.observe(waveformContainer.value);
    }

    window.addEventListener('resize', updateContainerWidth);

    dataSourceToken = window.__JUCE__.backend.addEventListener("MessageFromMelissaDataSource", (objectFromBackend) => {
        console.log('messageFromMelissaDataSource');
        console.log(objectFromBackend);
    });

    getCurrentValue("getLengthMSec").then((length: number) => {
        songLengthMs.value = length;
    });
});

onUnmounted(() => {
    clearInterval(playbackUpdateInterval);
    if (resizeObserver && waveformContainer.value) {
        resizeObserver.unobserve(waveformContainer.value);
    }
    window.removeEventListener('resize', updateContainerWidth);

    window.__JUCE__.backend.removeEventListener(dataSourceToken);
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
    /* 左右に20pxのパ��ィングを設定 */
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
</style>