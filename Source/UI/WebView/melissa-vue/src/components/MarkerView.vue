<template>
    <div class="markers-container pa-0" :style="$attrs.style">
        <div class="d-flex h-100">
            <div class="markers-table-container flex-grow-1">
                <table class="markers-table">
                    <thead>
                        <tr>
                            <th>色</th>
                            <th>時間</th>
                            <th>メモ</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr v-for="marker in markers" :key="marker.id" @click="selectMarker(marker)"
                            :class="{ 'selected-row': isMarkerSelected(marker) }">
                            <td><v-icon :color="marker.color">mdi-bookmark</v-icon></td>
                            <td>{{ marker.time }}</td>
                            <td>{{ marker.memo }}</td>
                        </tr>
                    </tbody>
                </table>
            </div>
            <div class="ml-4 d-flex flex-column">
                <v-btn icon class="mb-2" color="primary" @click="addMarker">
                    <v-icon>mdi-plus</v-icon>
                </v-btn>
                <v-btn icon class="mb-2" color="primary" @click="editMarker" :disabled="!selectedMarker">
                    <v-icon>mdi-pencil</v-icon>
                </v-btn>
                <v-btn icon color="primary" @click="deleteMarker" :disabled="!selectedMarker">
                    <v-icon>mdi-delete</v-icon>
                </v-btn>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const props = defineProps<{
    style?: object
}>();

const markers = ref([]);
const selectedMarker = ref(null);
const songLengthMs = ref(0);

const getCurrentValue = Juce.getNativeFunction("getCurrentValue");
const addDefaultMarker = Juce.getNativeFunction("addDefaultMarker");
const removeMarker = Juce.getNativeFunction("removeMarker");
const overwriteMarker = Juce.getNativeFunction("overwriteMarker");
const excuteCommand = Juce.getNativeFunction("excuteCommand");

const formatColor = (colorString: string): string => {
    // "0xffrrggbb" 形式の文字列から RGB 値を抽出
    const colorValue = parseInt(colorString.slice(2), 16);
    const r = (colorValue >> 16) & 0xFF;
    const g = (colorValue >> 8) & 0xFF;
    const b = colorValue & 0xFF;
    return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
};

const formatTime = (seconds: number) => {
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
};

const updateMarkers = (markersData: string) => {
    try {
        const parsedMarkers = JSON.parse(markersData);
        markers.value = parsedMarkers.map((marker: any, index: number) => ({
            id: index + 1,
            color: formatColor(marker.colour),
            time: formatTime(marker.position * songLengthMs.value / 1000),
            position: marker.position,
            memo: marker.memo
        }));
    } catch (error) {
        console.error('マーカーデータの解析に失敗しました:', error);
    }
};

const updateSongLength = async () => {
    try {
        const length = await getCurrentValue("getLengthMSec");
        songLengthMs.value = parseInt(length);
    } catch (error) {
        console.error('曲の長さの取得に失敗しました:', error);
    }
};

onMounted(async () => {
    await updateSongLength();
    getCurrentValue("getMarkers")
        .then((markersData) => {
            updateMarkers(markersData);
        })
        .catch((error) => {
            console.error('マーカーデータの取得に失敗しました:', error);
        });

    window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        const message = objectFromBackend[0];
        if (message === 'markerUpdated') {
            updateMarkers(objectFromBackend[1]);
        } else if (message === 'songChanged') {
            updateSongLength();
        }
    });
});

const selectMarker = (marker) => {
    selectedMarker.value = marker;

    // マーカーの位置に再生位置をジャンプさせる
    excuteCommand("PlaybackPositionValue", marker.position);
};

const isMarkerSelected = (marker) => {
    return marker === selectedMarker.value;
};

const addMarker = () => {
    getCurrentValue("getPlayingPosRatio")
        .then((position) => {
            addDefaultMarker(position);
        })
        .catch((error) => {
            console.error('再生位置の取得に失敗しました:', error);
        });
};

const deleteMarker = () => {
    if (selectedMarker.value) {
        const index = markers.value.findIndex(m => m === selectedMarker.value);
        if (index !== -1) {
            removeMarker(index);
        }
    }
};

const editMarker = () => {
    if (selectedMarker.value) {
        const index = markers.value.findIndex(m => m === selectedMarker.value);
        if (index !== -1) {
            // ここでマーカー編集ダイアログを表示し、ユーザーに新しい値を入力させます
            // 以下は例として、固定値を使用しています
            const newPosition = selectedMarker.value.time / (songLengthMs.value / 1000);
            const newMemo = "編集されたメモ";
            const newColor = "0xFF0000FF"; // 赤色 ("0xffrrggbb"形式)

            overwriteMarker(index, newPosition, newMemo, newColor);
        }
    }
};

defineExpose({ updateMarkers });
</script>

<style scoped>
.markers-container {
    display: flex;
    flex-direction: column;
    height: 100%;
}

.d-flex {
    height: 100%;
}

.markers-table-container {
    flex-grow: 1;
    overflow-y: auto;
    border: 1px solid rgba(0, 0, 0, 0.12);
    border-radius: 4px;
}

.markers-table {
    width: 100%;
    border-collapse: collapse;
}

.markers-table thead {
    position: sticky;
    top: 0;
    z-index: 1;
}

.markers-table th,
.markers-table td {
    padding: 8px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.12);
    text-align: left;
}

.markers-table tr {
    cursor: pointer;
}

.selected-row {
    background-color: rgba(0, 0, 0, 0.12);
}

.v-btn {
    margin-bottom: 8px;
}

.v-icon {
    font-size: 20px;
}
</style>