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
                <WaveformView ref="waveformRef" :songLengthMs="songLengthMs" />
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
                <PracticeListView />
            </v-container>
            <v-container v-else-if="activeView === 'markers'" class="practice-container pa-0" :style="markerViewStyle">
                <MarkerView ref="markerViewRef" :songLengthMs="songLengthMs" />
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

        <v-bottom-navigation v-if="isPortrait" v-model="activeView" mandatory>
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
import MarkerView from './components/MarkerView.vue';
import PracticeListView from './components/PracticeListView.vue';
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
const songLengthMs = ref(0);

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
            currentSongName.value = objectFromBackend[1].split('/').pop() || objectFromBackend[1];
            songLengthMs.value = objectFromBackend[2] / objectFromBackend[3] * 1000; // bufferLength / sampleRate * 1000
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


const formatTime = (seconds: number) => {
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
};

const listContainerStyle = computed(() => {
    const bottomNavHeight = isPortrait.value ? 56 : 0; // ボトムナビゲーションの高さ（通常は56px）
    return {
        height: `calc(100vh - 64px - ${waveformHeight.value}px - ${bottomNavHeight}px)`
    };
});

const markerViewStyle = computed(() => {
    const bottomNavHeight = isPortrait.value ? 56 : 0; // ボトムナビゲーションの高さ（通常は56px）
    return {
        height: `calc(100vh - 64px - ${waveformHeight.value}px - ${bottomNavHeight}px)`,
        overflow: 'auto'
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
    margin-bottom: 20px;
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

.practice-container {
    display: flex;
    flex-direction: column;
    padding: 0;
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
