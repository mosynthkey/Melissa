<template>
    <v-app class="no-scroll">
        <v-app-bar :elevation="2" density="compact">
            <v-menu>
                <template v-slot:activator="{ props }">
                    <v-btn icon v-bind="props">
                        <v-icon>mdi-menu</v-icon>
                    </v-btn>
                </template>
                <v-list>
                    <v-list-item @click="showSettings">
                        <v-list-item-title>設定</v-list-item-title>
                    </v-list-item>
                    <v-list-item @click="showAbout">
                        <v-list-item-title>About</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-menu>
            <v-btn icon @click="excuteCommand('Back', 1)">
                <v-icon>mdi-rewind</v-icon>
            </v-btn>
            <v-btn icon @click="togglePlayPause">
                <v-icon>{{ isPlaying ? 'mdi-pause' : 'mdi-play' }}</v-icon>
            </v-btn>
            <v-btn icon @click="showFileList">
                <v-icon>mdi-playlist-music</v-icon>
            </v-btn>
            <v-app-bar-title class="song-title text-truncate" :class="{ 'small-font': currentSongName.length > 20 }"
                @click="showFileList">
                {{ currentSongName }}
            </v-app-bar-title>
            <v-btn-toggle v-model="activeView" mandatory v-if="!isPortrait">
                <v-btn icon value="home" class="home-button">
                    <v-icon>mdi-home</v-icon>
                </v-btn>
                <v-btn icon value="practice">
                    <v-icon>mdi-playlist-check</v-icon>
                </v-btn>
                <v-btn icon value="markers">
                    <v-icon>mdi-bookmark-multiple</v-icon>
                </v-btn>
                <v-btn icon value="mixer">
                    <v-icon>mdi-tune-vertical</v-icon>
                </v-btn>
                <v-btn icon value="debug">
                    <v-icon>mdi-bug</v-icon>
                </v-btn>
            </v-btn-toggle>
        </v-app-bar>
        <v-main>
            <div class="waveform-container">
                <WaveformView ref="waveformRef" />
            </div>

            <v-container v-if="activeView === 'home'">
                <div class="button-container">
                    <v-btn variant="tonal" @click="excuteCommand('Pitch_Minus', 1)">
                        Pitch -1
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('Pitch_Plus', 1)">
                        Pitch +1
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('Pitch_Reset', 1)">
                        Pitch Reset
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('SetSpeed_Minus5', 1)">
                        Speed -1
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('SetSpeed_Plus5', 1)">
                        Speed +1
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('ResetSpeed', 1)">
                        Speed Reset
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('SetLoopStart', 1)">
                        Set loop start
                    </v-btn>
                    <v-btn variant="tonal" @click="excuteCommand('SetLoopEnd', 1)">
                        Set loop end
                    </v-btn>
                </div>
            </v-container>

            <v-container v-else-if="activeView === 'practice'" class="practice-container pa-0"
                :style="listContainerStyle">
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
                                    <td>{{ item.pitch }}</td>
                                </tr>
                            </tbody>
                        </table>
                    </div>
                    <div class="ml-4 d-flex flex-column">
                        <v-btn icon class="mb-2" color="primary" @click="addPracticeItem">
                            <v-icon>mdi-plus</v-icon>
                        </v-btn>
                        <v-btn icon class="mb-2" color="primary" @click="editPracticeItem"
                            :disabled="!selectedPracticeItem">
                            <v-icon>mdi-pencil</v-icon>
                        </v-btn>
                        <v-btn icon color="primary" @click="deletePracticeItem" :disabled="!selectedPracticeItem">
                            <v-icon>mdi-delete</v-icon>
                        </v-btn>
                    </div>
                </div>
            </v-container>

            <v-container v-else-if="activeView === 'markers'" class="markers-container pa-0"
                :style="listContainerStyle">
                <div class="d-flex h-100">
                    <div class="markers-table-container flex-grow-1">
                        <table class="markers-table">
                            <thead>
                                <tr>
                                    <th>色</th>
                                    <th>時間</th>
                                    <th>名前</th>
                                </tr>
                            </thead>
                            <tbody>
                                <tr v-for="marker in markers" :key="marker.id" @click="selectMarker(marker)"
                                    :class="{ 'selected-row': isMarkerSelected(marker) }">
                                    <td><v-icon :color="marker.color">mdi-bookmark</v-icon></td>
                                    <td>{{ marker.time }}</td>
                                    <td>{{ marker.name }}</td>
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
            </v-container>

            <v-container v-else-if="activeView === 'debug'" class="debug-container">
                <v-card class="debug-area" variant="outlined">
                    <v-card-text>
                        <pre class="debug-text">{{ reversedDebugLogs.join('\n') }}</pre>
                    </v-card-text>
                </v-card>
            </v-container>

            <v-container v-else-if="activeView === 'mixer'">
                <!-- ミキサーの表示 -->
                <h2>ミキサー</h2>
                <!-- ここにミキサーの内容を追加 -->
            </v-container>
        </v-main>

        <!-- ファイルリストポップアップ -->
        <v-dialog v-model="fileListDialog" fullscreen>
            <v-card>
                <v-toolbar dark color="primary">
                    <v-btn icon dark @click="fileListDialog = false">
                        <v-icon>mdi-close</v-icon>
                    </v-btn>
                    <v-toolbar-title>ファイルリスト</v-toolbar-title>
                    <v-spacer></v-spacer>
                    <v-btn icon dark @click="showFileChooserAndImport">
                        <v-icon>mdi-file-import</v-icon>
                    </v-btn>
                    <v-btn icon dark>
                        <v-icon>mdi-refresh</v-icon>
                    </v-btn>
                    <v-btn icon dark>
                        <v-icon>mdi-dots-vertical</v-icon>
                    </v-btn>
                </v-toolbar>
                <v-list>
                    <v-list-item v-for="(file, index) in fileList" :key="index" @click="loadFileAndCloseDialog(file)">
                        <template v-slot:prepend>
                            <v-icon>mdi-file-music</v-icon>
                        </template>
                        <v-list-item-title>{{ getFileName(file) }}</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-card>
        </v-dialog>

        <!-- ローディング面 -->
        <v-dialog v-model="isLoading" persistent max-width="300px">
            <v-card>
                <v-card-text>
                    ファイルを読み込んでいます...
                    <v-progress-circular indeterminate color="primary"></v-progress-circular>
                </v-card-text>
            </v-card>
        </v-dialog>

        <!-- About ダイアログ -->
        <v-dialog v-model="aboutDialog" max-width="300px">
            <v-card>
                <v-card-title>About</v-card-title>
                <v-card-text>
                    <p>Melissa Mobile v1.0.0</p>
                    <!-- 他の情報を追加できます -->
                </v-card-text>
                <v-card-actions>
                    <v-spacer></v-spacer>
                    <v-btn color="primary" text @click="aboutDialog = false">閉じる</v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <v-bottom-navigation v-if="isPortrait" v-model="activeView">
            <v-btn value="home">
                <v-icon>mdi-home</v-icon>
                ホーム
            </v-btn>
            <v-btn value="practice">
                <v-icon>mdi-playlist-check</v-icon>
                練習
            </v-btn>
            <v-btn value="markers">
                <v-icon>mdi-bookmark-multiple</v-icon>
                マーカー
            </v-btn>
            <v-btn value="mixer">
                <v-icon>mdi-tune-vertical</v-icon>
                ミキサー
            </v-btn>
            <v-btn value="debug">
                <v-icon>mdi-bug</v-icon>
                デバッグ
            </v-btn>
        </v-bottom-navigation>
    </v-app>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed, nextTick } from 'vue';
import WaveformView from './components/WaveformView.vue';
// @ts-ignore
import * as Juce from "juce-framework-frontend";

const excuteCommand = Juce.getNativeFunction("excuteCommand");
const getFileList = Juce.getNativeFunction("getFileList");
const loadFile = Juce.getNativeFunction("loadFile");
const showFileChooserAndImport = Juce.getNativeFunction("showFileChooserAndImport");

const fileListDialog = ref(false);
const fileList = ref<string[]>([]);
const isLoading = ref(false);
const currentSongName = ref('曲が選択されてません');
const isPlaying = ref(false);
const aboutDialog = ref(false);
const activeView = ref('home');
const debugLogs = ref<string[]>([]);
const waveformRef = ref(null);
const waveformHeight = ref(0);

const addDebugLog = (log: string) => {
    debugLogs.value.unshift(`${new Date().toISOString()}: ${log}`);
    if (debugLogs.value.length > 100) {
        debugLogs.value.pop();
    }
};

const reversedDebugLogs = computed(() => debugLogs.value);

const togglePlayPause = () => {
    excuteCommand('StartStop', 1);
};

const fetchFileList = async () => {
    try {
        const result = await getFileList();
        fileList.value = JSON.parse(result);
    } catch (error) {
        console.error('ファイルリストの取得に失敗しました:', error);
    }
};

const showFileList = () => {
    fileListDialog.value = true;
};

const getFileName = (filePath: string) => {
    return filePath.split('/').pop() || filePath;
};


const showSettings = () => {
    // 設定画面を表示する処理をここに実装
    console.log('設定画面を表示');
};

const showAbout = () => {
    aboutDialog.value = true;
};

let notificationToken: any = null;

onMounted(async () => {
    await fetchFileList();

    notificationToken = window.__JUCE__.backend.addEventListener("MelissaNotification", (objectFromBackend: any) => {
        addDebugLog(JSON.stringify(objectFromBackend));
        const message = objectFromBackend[0];
        if (message === 'songChanged') {
            isLoading.value = false;
            // ここで現在の曲名を更新する処理を追加する必要があります
        } else if (message === 'playbackStatusChanged') {
            isPlaying.value = objectFromBackend[1] === 0;
        }
    });

    await nextTick();
    if (waveformRef.value) {
        waveformHeight.value = waveformRef.value.$el.offsetHeight;
    }
});

onUnmounted(() => {
    if (notificationToken) {
        window.__JUCE__.backend.removeEventListener(notificationToken);
    }
});

const practiceHeaders = [
    { title: 'Name', key: 'name' },
    { title: 'Loop range', key: 'loopRange' },
    { title: 'Pitch', key: 'pitch' },
];

const practiceItems = ref([
    { name: '練習項目1', loopRange: '0:00-0:30', pitch: 0 },
    { name: '練習項目2', loopRange: '0:30-1:00', pitch: -1 },
    { name: '練習項目3', loopRange: '1:00-1:30', pitch: 2 },
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
        pitch: 0,
    });
};

const editPracticeItem = () => {
    if (selectedPracticeItem.value) {
        console.log('編集する目:', selectedPracticeItem.value);
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

const loadFileAndCloseDialog = (file: string) => {
    isLoading.value = true;
    fileListDialog.value = false;
    try {
        loadFile(file);
        currentSongName.value = getFileName(file);
    } catch (error) {
        console.error('ファイルのロードに失敗しました:', error);
        addDebugLog(`ファイルのロード失敗: ${error}`);
    } finally {
        isLoading.value = false;
    }
};

// デバッグログを追加するテスト用の関数
const addTestLog = () => {
    addDebugLog(`テストログ ${Date.now()}`);
};

// コンポーネントがマウントされたときにテストログを追加
onMounted(() => {
    addTestLog();
});

const markers = ref([
    { id: 1, color: 'red', time: '0:30', name: 'サビ開始' },
    { id: 2, color: 'blue', time: '1:45', name: 'ブリッジ' },
    { id: 3, color: 'green', time: '2:30', name: '2番開始' },
]);

const selectedMarker = ref(null);

const selectMarker = (marker) => {
    selectedMarker.value = marker === selectedMarker.value ? null : marker;
};

const isMarkerSelected = (marker) => {
    return marker === selectedMarker.value;
};

const addMarker = () => {
    const newId = Math.max(0, ...markers.value.map(m => m.id)) + 1;
    markers.value.push({
        id: newId,
        color: 'gray',
        time: '0:00',
        name: `新しいマーカー${newId}`,
    });
};

const editMarker = () => {
    if (selectedMarker.value) {
        console.log('編集するマーカー:', selectedMarker.value);
        // ここにマーカー編集のロジックを追加
    }
};

const deleteMarker = () => {
    if (selectedMarker.value) {
        const index = markers.value.findIndex(m => m === selectedMarker.value);
        if (index !== -1) {
            markers.value.splice(index, 1);
            selectedMarker.value = null;
        }
    }
};

const listContainerStyle = computed(() => {
    const bottomNavHeight = isPortrait.value ? 56 : 0; // ボトムナビゲーションの高さ（通常は56px）
    return {
        height: `calc(100vh - 64px - ${waveformHeight.value}px - ${bottomNavHeight}px)`
    };
});

const isPortrait = ref(window.innerHeight > window.innerWidth);

const updateOrientation = () => {
    isPortrait.value = window.innerHeight > window.innerWidth;
};

onMounted(() => {
    updateOrientation();
    window.addEventListener('resize', updateOrientation);
    nextTick(() => {
        if (waveformRef.value) {
            waveformHeight.value = waveformRef.value.$el.offsetHeight;
        }
    });
});

onUnmounted(() => {
    window.removeEventListener('resize', updateOrientation);
});

</script>

<style>
html,
body {
    overflow: hidden;
    position: fixed;
    width: 100%;
    height: 100%;
}
</style>

<style scoped>
.waveform-container {
    margin-top: 20px;
    /* ヘッダーと波形の間に20pxの隙間を作る */
}

.button-container {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    margin-top: 16px;
}

.v-app-bar-title {
    flex-grow: 1;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    background-color: rgba(0, 0, 0, 0.1);
    padding: 4px 8px;
    border-radius: 4px;
    margin-right: 8px;
    text-align: center;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    user-select: none;
}

.v-app-bar>.v-btn:last-child {
    margin-right: -12px;
    /* アイコンを右端に寄せる */
}

.debug-container {
    display: flex;
    flex-direction: column;
    height: calc(100vh - 64px);
    /* ヘッダーの高さを引いた値 */
    padding: 0;
}

.debug-area {
    flex-grow: 1;
    overflow-y: auto;
    margin-bottom: 16px;
}

.debug-text {
    font-size: 0.8rem;
    white-space: pre-wrap;
    word-wrap: break-word;
}

.small-font {
    font-size: 0.8em;
}

.menu-spacing {
    margin-left: 8px;
    /* アイコン0.5個分くらいのスペース */
}

.practice-container,
.markers-container {
    display: flex;
    flex-direction: column;
    padding: 0;
}

.practice-table-container,
.markers-table-container {
    flex-grow: 1;
    overflow-y: auto;
    border: 1px solid rgba(0, 0, 0, 0.12);
    border-radius: 4px;
}

.practice-table,
.markers-table {
    width: 100%;
    border-collapse: collapse;
}

.practice-table thead,
.markers-table thead {
    position: sticky;
    top: 0;
    z-index: 1;
}

.practice-table th,
.practice-table td,
.markers-table th,
.markers-table td {
    padding: 8px;
    border-bottom: 1px solid rgba(0, 0, 0, 0.12);
    text-align: left;
}

.practice-table tr,
.markers-table tr {
    cursor: pointer;
}

.selected-row {
    background-color: rgba(0, 0, 0, 0.12);
}

.v-btn-toggle {
    margin-right: 0px;
    /* アイコンを右端に寄せる */
}

.home-button {
    border-left: 1px solid rgba(0, 0, 0, 0.12) !important;
}

.v-bottom-navigation {
    position: fixed;
    bottom: 0;
    left: 0;
    right: 0;
}
</style>
