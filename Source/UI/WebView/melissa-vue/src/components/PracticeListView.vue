<template>
    <div class="d-flex h-100">
        <div class="practice-table-container flex-grow-1">
            <table class="practice-table">
                <thead>
                    <tr>
                        <th>名前</th>
                        <th>開始位置</th>
                        <th>終了位置</th>
                        <th>再生速度</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(item, index) in practiceItems" :key="index" @click="selectItem(item)"
                        :class="{ 'selected-row': isSelected(item) }">
                        <td>{{ item.name }}</td>
                        <td>{{ formatTime(item.startRatio) }}</td>
                        <td>{{ formatTime(item.endRatio) }}</td>
                        <td>{{ item.speed }}%</td>
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
import { ref, onMounted } from 'vue';
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

onMounted(async () => {
    await updatePracticeItems();
    await updateSongLength();

    window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        const message = objectFromBackend[0];
        if (message === 'practiceListUpdated') {
            updatePracticeItems();
        } else if (message === 'songChanged') {
            updateSongLength();
        }
    });
});

const formatTime = (ratio: number) => {
    const totalSeconds = (ratio * songLengthMs.value) / 1000;
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = Math.floor(totalSeconds % 60);
    return `${minutes}:${seconds.toString().padStart(2, '0')}`;
};

const selectItem = (item) => {
    selectedPracticeItem.value = item;
    excuteCommand('SetLoopStartValue', item.startRatio);
    excuteCommand('SetLoopEndValue', item.endRatio);
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
}

.practice-table tr {
    cursor: pointer;
}

.selected-row {
    background-color: rgba(0, 0, 0, 0.12);
}
</style>