<template>
    <v-app>
        <v-app-bar :elevation="2" density="compact">
            <v-btn icon @click="excuteCommand('Back', 1)">
                <v-icon>mdi-rewind</v-icon>
            </v-btn>
            <v-btn icon @click="togglePlayPause">
                <v-icon>{{ isPlaying ? 'mdi-pause' : 'mdi-play' }}</v-icon>
            </v-btn>
            <v-btn icon @click="showFileList">
                <v-icon>mdi-playlist-music</v-icon>
            </v-btn>
            <v-app-bar-title class="text-truncate">{{ currentSongName }}</v-app-bar-title>
            <v-spacer></v-spacer>
            <v-menu>
                <template v-slot:activator="{ props }">
                    <v-btn icon v-bind="props">
                        <v-icon>mdi-dots-vertical</v-icon>
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
        </v-app-bar>
        <v-main>
            <div class="waveform-container">
                <WaveformView />
            </div>

            <div class="button-container">
                <v-btn variant="tonal" @click="excuteCommand('StartStop', 1)">
                    Play / Stop
                </v-btn>
                <v-btn variant="tonal" @click="excuteCommand('Back', 1)">
                    Back
                </v-btn>
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
                <v-btn variant="tonal" @click="showFileList">
                    FileList
                </v-btn>
                <v-btn variant="tonal" @click="showFileChooserAndImport">
                    Import
                </v-btn>
            </div>

            <!-- デバッグ用表示領域 -->
            <v-expansion-panels>
                <v-expansion-panel>
                    <v-expansion-panel-title>
                        デバッグ情報
                    </v-expansion-panel-title>
                    <v-expansion-panel-text>
                        <v-card class="debug-area" variant="outlined">
                            <v-card-text>
                                <pre>{{ debugLogs.join('\n') }}</pre>
                            </v-card-text>
                        </v-card>
                    </v-expansion-panel-text>
                </v-expansion-panel>
            </v-expansion-panels>
        </v-main>

        <!-- ファイルリストポップアップ -->
        <v-dialog v-model="fileListDialog" max-width="500px">
            <v-card>
                <v-card-title>ファイルリスト</v-card-title>
                <v-card-text>
                    <v-list>
                        <v-list-item v-for="(file, index) in fileList" :key="index" @click="openFile(file)">
                            <v-list-item-title>{{ getFileName(file) }}</v-list-item-title>
                        </v-list-item>
                    </v-list>
                </v-card-text>
            </v-card>
        </v-dialog>

        <!-- ローディング画面 -->
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
    </v-app>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
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
const currentSongName = ref('曲が選択されていません');
const isPlaying = ref(false);
const aboutDialog = ref(false);

const debugLogs = ref<string[]>([]);

const addDebugLog = (log: string) => {
    debugLogs.value.push(`${new Date().toISOString()}: ${log}`);
    if (debugLogs.value.length > 100) {
        debugLogs.value.shift();
    }
};

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

const openFile = async (file: string) => {
    isLoading.value = true;
    try {
        await loadFile(file);
        currentSongName.value = getFileName(file);
        fileListDialog.value = false;
    } catch (error) {
        console.error('File loading failed:', error);
    } finally {
        isLoading.value = false;
    }
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
            // 例: currentSongName.value = getCurrentSongName(); // この関数は実装する必要があります
        } else if (message === 'playbackStatusChanged') {
            isPlaying.value = objectFromBackend[1] === 0;
        }
    });
});

onUnmounted(() => {
    if (notificationToken) {
        window.__JUCE__.backend.removeEventListener(notificationToken);
    }
});

const showSettings = () => {
    // 設定画面を表示する処理をここに実装
    console.log('設定画面を表示');
};

const showAbout = () => {
    aboutDialog.value = true;
};
</script>

<style scoped>
.waveform-container {
    margin-top: 20px;
    /* ヘッダーと波形の間に20pxの隙間を作る */
}

.button-container {
    margin-top: 20px;
    /* ボタンと波形の間に隙間を作る */
}

.v-app-bar-title {
    max-width: 60%;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.v-app-bar>.v-btn:last-child {
    margin-right: -12px;
    /* アイコンを右端に寄せる */
}

.debug-area {
    max-height: 200px;
    overflow-y: auto;
}

.debug-area pre {
    white-space: pre-wrap;
    word-wrap: break-word;
}
</style>