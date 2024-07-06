<template>
    <div class="d-flex h-100">
        <div class="practice-table-container flex-grow-1">
            <table class="practice-table">
                <thead>
                    <tr>
                        <th>Name</th>
                        <th>Loop range</th>
                        <th>Pitch</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="item in practiceItems" :key="item.name" @click="selectItem(item)"
                        :class="{ 'selected-row': isSelected(item) }">
                        <td>{{ item.name }}</td>
                        <td>{{ item.loopRange }}</td>
                        <td>{{ item.speed }}</td>
                    </tr>
                </tbody>
            </table>
        </div>
        <div class="ml-4 d-flex flex-column">
            <v-btn icon class="mb-2" color="primary" @click="addPracticeItem">
                <v-icon>mdi-plus</v-icon>
            </v-btn>
            <v-btn icon class="mb-2" color="primary" @click="editPracticeItem" :disabled="!selectedPracticeItem">
                <v-icon>mdi-pencil</v-icon>
            </v-btn>
            <v-btn icon color="primary" @click="deletePracticeItem" :disabled="!selectedPracticeItem">
                <v-icon>mdi-delete</v-icon>
            </v-btn>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';

const practiceItems = ref([
    { name: '練習項目1', loopRange: '0:00-0:30', speed: 0 },
    { name: '練習項目2', loopRange: '0:30-1:00', speed: -1 },
    { name: '練習項目3', loopRange: '1:00-1:30', speed: 2 },
]);

const selectedPracticeItem = ref(null);

const selectItem = (item) => {
    selectedPracticeItem.value = item === selectedPracticeItem.value ? null : item;
};

const isSelected = (item) => {
    return item === selectedPracticeItem.value;
};

const addPracticeItem = () => {
    practiceItems.value.push({
        name: `新しい練習項目${practiceItems.value.length + 1}`,
        loopRange: '0:00-0:00',
        speed: 0,
    });
};

const editPracticeItem = () => {
    if (selectedPracticeItem.value) {
        console.log('編集する項目:', selectedPracticeItem.value);
    }
};

const deletePracticeItem = () => {
    if (selectedPracticeItem.value) {
        const index = practiceItems.value.findIndex(item => item === selectedPracticeItem.value);
        if (index !== -1) {
            practiceItems.value.splice(index, 1);
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