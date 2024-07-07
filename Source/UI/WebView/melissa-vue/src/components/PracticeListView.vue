<template>
    <div class="d-flex h-100">
        <div class="practice-table-container flex-grow-1">
            <table class="practice-table">
                <thead>
                    <tr>
                        <th class="name-column">名前</th>
                        <th class="range-column">ループ範囲</th>
                        <th class="speed-column">再生速度</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, index) in practiceItems" :key="index" @click="selectItem(item)"
                        :class="{ 'selected-row': isSelected(item) }">
                        <td class="name-column">{{ item.name }}</td>
                        <td class="range-column">
                            <LoopRangeComponent :startRatio="item.startRatio" :endRatio="item.endRatio" :height="12" />
                        </td>
                        <td class="speed-column">{{ item.speed }}%</td>
                    </tr>
                </tbody>
            </table>
        </div>
        <div class="ml-4 d-flex flex-column">
            <v-btn icon class="mb-2" color="primary" @click="addPracticeItem">
                <v-icon>mdi-plus</v-icon>
            </v-btn>
            <v-btn icon class="mb-2" color="primary"
                @click="editPracticeItem(selectedPracticeItem, practiceItems.indexOf(selectedPracticeItem))"
                :disabled="!selectedPracticeItem">
                <v-icon>mdi-pencil</v-icon>
            </v-btn>
            <v-btn icon color="primary" @click="deletePracticeItem" :disabled="!selectedPracticeItem">
                <v-icon>mdi-delete</v-icon>
            </v-btn>
        </div>
    </div>

    <v-dialog v-model="editDialog" max-width="300px">
        <v-card>
            <v-card-title>練習項目の編集</v-card-title>
            <v-card-text>
                <v-text-field v-model="editingItem.name" label="名前"></v-text-field>
                <v-slider v-model="editingItem.speed" min="25" max="200" step="1" label="再生速度" thumb-label></v-slider>
            </v-card-text>
            <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn color="blue darken-1" text @click="editDialog = false">キャンセル</v-btn>
                <v-btn color="blue darken-1" text @click="savePracticeItem">保存</v-btn>
            </v-card-actions>
        </v-card>
    </v-dialog>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed, defineComponent, watch, h, onBeforeUnmount } from 'vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const practiceItems = ref([]);
const selectedPracticeItem = ref(null);
const songLengthMs = ref(0);
const editDialog = ref(false);
const editingItem = ref({ name: '', startRatio: 0, endRatio: 0, speed: 100 });
const editingIndex = ref(-1);

const getCurrentPracticeList = Juce.getNativeFunction("getCurrentPracticeList");
const addPracticeList = Juce.getNativeFunction("addPracticeList");
const removePracticeList = Juce.getNativeFunction("removePracticeList");
const overwritePracticeList = Juce.getNativeFunction("overwritePracticeList");
const excuteCommand = Juce.getNativeFunction("excuteCommand");
const getCurrentValue = Juce.getNativeFunction("getCurrentValue");
const setLoopPosition = Juce.getNativeFunction("setLoopPosition");

const updatePracticeItems = async () => {
    try {
        const practiceListJson = await getCurrentPracticeList();
        practiceItems.value = JSON.parse(practiceListJson);
    } catch (error) {
        console.error('練習リストの取得に失敗しました:', error);
    }
};

const updateSongLength = async () => {
    try {
        const lengthMSec = await getCurrentValue("getLengthMSec");
        songLengthMs.value = parseFloat(lengthMSec);
    } catch (error) {
        console.error('曲の長さの取得に失敗しました:', error);
    }
};

const handleMelissaNotification = (objectFromBackend: any) => {
    const message = objectFromBackend[0];
    if (message === 'practiceListUpdated') {
        updatePracticeItems();
    } else if (message === 'songChanged') {
        updateSongLength();
        updatePracticeItems(); // 曲が変わったときに練習リストも更新
    }
};

onMounted(async () => {
    await updatePracticeItems();
    await updateSongLength();

    window.__JUCE__.backend.addEventListener("MelissaNotification", handleMelissaNotification);
});

onUnmounted(() => {
    window.__JUCE__.backend.removeEventListener("MelissaNotification", handleMelissaNotification);
});

const formatTime = (ratio: number) => {
    const totalSeconds = (ratio * songLengthMs.value) / 1000;
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = Math.floor(totalSeconds % 60);
    return `${minutes}:${seconds.toString().padStart(2, '0')}`;
};

const selectItem = (item) => {
    selectedPracticeItem.value = item;
    setLoopPosition(item.startRatio, item.endRatio);
    excuteCommand('SetSpeedValue', (item.speed - 20) / (200.0 - 20.0));
};

const isSelected = (item) => {
    return item === selectedPracticeItem.value;
};

const addPracticeItem = async () => {
    try {
        const startRatio = await getCurrentValue("getLoopAPosRatio");
        const endRatio = await getCurrentValue("getLoopBPosRatio");
        const speed = await getCurrentValue("getSpeed");

        const newName = `${formatTime(parseFloat(startRatio))} - ${formatTime(endRatio)}`;
        addPracticeList(newName, startRatio, endRatio, speed);
    } catch (error) {
        console.error('練習項目の追加に失敗しました:', error);
    }
};

const editPracticeItem = (item, index) => {
    editingItem.value = { ...item };
    editingIndex.value = index;
    editingItem.value.startRatio = item.startRatio;
    editingItem.value.endRatio = item.endRatio;
    editingItem.value.speed = item.speed;
    editDialog.value = true;
};

const savePracticeItem = async () => {
    if (editingIndex.value !== -1) {
        overwritePracticeList(editingIndex.value, editingItem.value.name, editingItem.value.startRatio, editingItem.value.endRatio, editingItem.value.speed);
    }
    editDialog.value = false;
};

const deletePracticeItem = async () => {
    if (selectedPracticeItem.value) {
        const index = practiceItems.value.indexOf(selectedPracticeItem.value);
        if (index !== -1) {
            await removePracticeList(index);
            selectedPracticeItem.value = null;
        }
    }
};

// LoopRangeComponent の定義
const LoopRangeComponent = defineComponent({
    props: {
        startRatio: {
            type: Number,
            required: true
        },
        endRatio: {
            type: Number,
            required: true
        },
        height: {
            type: Number,
            default: 12 // デフォルトの高さを12に設定
        }
    },
    setup(props) {
        const canvasRef = ref(null);

        const drawLoopRange = () => {
            const canvas = canvasRef.value;
            if (!canvas) return;

            const ctx = canvas.getContext('2d');
            const width = canvas.clientWidth;
            const height = canvas.clientHeight;
            const radius = props.height / 2; // 角丸の半径

            // 背景をクリア
            ctx.clearRect(0, 0, width, height);

            // 全体の背景を描画
            ctx.fillStyle = 'rgba(0, 0, 0, 0.12)';
            ctx.beginPath();
            ctx.moveTo(radius, 0);
            ctx.lineTo(width - radius, 0);
            ctx.quadraticCurveTo(width, 0, width, radius);
            ctx.lineTo(width, height - radius);
            ctx.quadraticCurveTo(width, height, width - radius, height);
            ctx.lineTo(radius, height);
            ctx.quadraticCurveTo(0, height, 0, height - radius);
            ctx.lineTo(0, radius);
            ctx.quadraticCurveTo(0, 0, radius, 0);
            ctx.closePath();
            ctx.fill();

            // ループ範囲を描画
            const startX = props.startRatio * width;
            const endX = props.endRatio * width;
            ctx.fillStyle = 'rgba(255, 107, 107, 0.7)';
            ctx.beginPath();
            ctx.moveTo(startX + radius, 0);
            ctx.lineTo(endX - radius, 0);
            ctx.quadraticCurveTo(endX, 0, endX, radius);
            ctx.lineTo(endX, height - radius);
            ctx.quadraticCurveTo(endX, height, endX - radius, height);
            ctx.lineTo(startX + radius, height);
            ctx.quadraticCurveTo(startX, height, startX, height - radius);
            ctx.lineTo(startX, radius);
            ctx.quadraticCurveTo(startX, 0, startX + radius, 0);
            ctx.closePath();
            ctx.fill();
        };

        const resizeCanvas = () => {
            const canvas = canvasRef.value;
            if (!canvas) return;
            canvas.width = canvas.clientWidth;
            canvas.height = props.height;
            drawLoopRange();
        };

        onMounted(() => {
            console.log('LoopRangeComponent mounted');
            resizeCanvas();
            window.addEventListener('resize', resizeCanvas);
        });

        onBeforeUnmount(() => {
            window.removeEventListener('resize', resizeCanvas);
        });

        watch([() => props.startRatio, () => props.endRatio, () => props.height], () => {
            console.log('LoopRangeComponent props changed', props.startRatio, props.endRatio, props.height);
            resizeCanvas();
        });

        return { canvasRef };
    },
    render() {
        return h('div', { class: 'loop-range-container' }, [
            h('canvas', { ref: 'canvasRef', style: { width: '100%', height: `${this.height}px` } })
        ]);
    }
});
</script>

<style scoped>
.practice-table-container {
    flex-grow: 1;
    overflow-y: auto;
    border: 1px solid rgba(0, 0, 0, 0.12);
    border-radius: 4px;
}

.practice-table {
    width: 100%;
    border-collapse: collapse;
    table-layout: fixed;
    /* テーブルの幅を固定 */
}

.practice-table thead {
    position: sticky;
    top: 0;
    z-index: 1;
}

.practice-table th,
.practice-table td {
    padding: 8px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.12);
    text-align: left;
    vertical-align: middle;
    overflow: hidden;
    /* 内容がはみ出さないようにする */
    text-overflow: ellipsis;
    /* 内容がはみ出た場合に省略記号を表示 */
    white-space: nowrap;
    /* 内容を折り返さない */
}

.practice-table tr {
    cursor: pointer;
}

.selected-row {
    background-color: rgba(0, 0, 0, 0.12);
}

.loop-range-container {
    width: 100%;
    height: 24px;
    display: flex;
    align-items: center;
    justify-content: center;
    /* 中央揃え */
}

.practice-table td {
    padding: 8px;
    vertical-align: middle;
}

.selected-row {
    background-color: rgba(255, 107, 107, 0.2);
}

.name-column {
    width: 30%;
    /* 名前列の幅を30%に設定 */
}

.range-column {
    width: 50%;
    /* ループ範囲列の幅を50%に設定 */
}

.speed-column {
    width: 20%;
    /* 再生速度列の幅を20%に設定 */
}
</style>
