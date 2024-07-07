<template>
    <div class="markers-container pa-0" :style="$attrs.style">
        <div class="d-flex h-100">
            <div class="markers-table-container flex-grow-1">
                <table class="markers-table">
                    <thead>
                        <tr>
                            <th class="color-column">色</th>
                            <th class="time-column">時間</th>
                            <th class="memo-column">メモ</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr v-for="marker in markers" :key="marker.id" @click="selectMarker(marker)"
                            :class="{ 'selected-row': isMarkerSelected(marker) }">
                            <td class="color-column"><v-icon :color="marker.color">mdi-bookmark</v-icon></td>
                            <td class="time-column">{{ marker.time }}</td>
                            <td class="memo-column">{{ marker.memo }}</td>
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
    <v-dialog v-model="editDialogOpen" max-width="400px" :style="dialogStyle">
        <v-card>
            <v-card-title class="pb-2">マーカーの編集</v-card-title>
            <v-card-text class="pt-0 pb-2">
                <v-text-field v-model="editedMarker.memo" label="メモ" @focus="onInputFocus" @blur="onInputBlur"
                    dense></v-text-field>
            </v-card-text>
            <v-card-actions class="pt-0">
                <v-spacer></v-spacer>
                <v-btn color="blue darken-1" text @click="closeEditDialog">キャンセル</v-btn>
                <v-btn color="blue darken-1" text @click="saveEditedMarker">保存</v-btn>
            </v-card-actions>
        </v-card>
    </v-dialog>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue';
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

const reverseFormatColor = (hexColor: string): string => {
    // '#rrggbb' 形式の文字列から '0xffrrggbb' 形式に変換
    const rgb = hexColor.slice(1); // '#' を削除
    return `ff${rgb}`;
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

const editDialogOpen = ref(false);
const editedMarker = ref({ memo: '' });
const keyboardOpen = ref(false);

const onInputFocus = () => {
    keyboardOpen.value = true;
};

const onInputBlur = () => {
    keyboardOpen.value = false;
};

const dialogStyle = computed(() => {
    if (keyboardOpen.value) {
        return {
            top: '5%',
            transition: 'top 0.3s',
        };
    }
    return {};
});

const editMarker = () => {
    if (selectedMarker.value) {
        editedMarker.value = {
            memo: selectedMarker.value.memo
        };
        editDialogOpen.value = true;
    }
};

const closeEditDialog = () => {
    editDialogOpen.value = false;
};

const saveEditedMarker = () => {
    if (selectedMarker.value) {
        const index = markers.value.findIndex(m => m === selectedMarker.value);
        if (index !== -1) {
            const newPosition = selectedMarker.value.position;
            const newMemo = editedMarker.value.memo;
            const currentColor = reverseFormatColor(selectedMarker.value.color);
            console.log(`${selectedMarker.value.color} -> ${currentColor}`)

            overwriteMarker(index, newPosition, newMemo, currentColor);
            editDialogOpen.value = false;
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

.v-dialog {
    transition: top 0.3s;
}

.v-card-title {
    font-size: 1.25rem;
    padding-bottom: 8px !important;
}

.v-card-text {
    padding-top: 0 !important;
    padding-bottom: 8px !important;
}

.v-card-actions {
    padding-top: 0 !important;
}

.color-column {
    width: 10%;
}

.time-column {
    width: 30%;
}

.memo-column {
    width: 60%;
}
</style>